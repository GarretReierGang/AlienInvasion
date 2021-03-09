#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/ioport.h> //I/O memory allocation and mapping
#include <asm/io.h>	//ioremap iounmap
#include <linux/interrupt.h> //irq
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <linux/platform_device.h> //platform_device_register etc..
#include <linux/device.h>	   //device_create/device_destroy
//#include <unistd.h>

//For IO configuration
#include <linux/ioctl.h>


//DEBUGING
#include <linux/printk.h>
//EndOF debugging specific includes

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Garret Gang && Devon SookHoo");
MODULE_DESCRIPTION("ECEn 427 Audio Driver");
MODULE_VERSION("0.1");


#define MODULE_NAME "audio"
#define NUM_DEVICES 1
#define SUCCESS 0

#define NUM_BYTES_TO_WRITE 512
#define TOTAL_SPACE (1024)

#define AUDIO_RIGHT_OFFSET (0x08 / sizeof(uint32_t))
#define AUDIO_LEFT_OFFSET (0x0C / sizeof(uint32_t))
#define AUDIO_WRITE_OFFSET (AUDIO_RIGHT_OFFSET / sizeof(uint32_t))

#define IRQ_DISABLED 0x0
#define ISR_ENABLE_OFFSET (0x10/ sizeof(uint32_t))
#define ISR_ENABLE (1 << 0)
#define ISR_DISABLE ~(1 << 0)
#define TARGET_VALUE 800

#define IOCTL_MAGIC_NUMBER 'm'
#define IOCTL_START_LOOP _IO(IOCTL_MAGIC_NUMBER, 0)
#define IOCTL_STOP_LOOP _IO(IOCTL_MAGIC_NUMBER, 1)

#define START_LOOPING 1
#define STOP_LOOPING 0

//IOCTL specific commands


// Function declarations
static int audio_init(void);
static void audio_exit(void);
static int audio_remove(struct platform_device * pdev);
static int audio_probe(struct platform_device *pdev) ;
static irqreturn_t audio_isr(int irq, void *dev_id);
static ssize_t audioRead(struct file *, char __user *, size_t, loff_t *);
static ssize_t audioWrite(struct file *, const char __user *, size_t, loff_t *);
static long audioIoctl(struct file *, unsigned int cmd, unsigned long arg);

module_init(audio_init);
module_exit(audio_exit);


static int majorNumber;
static struct class* audioClass = NULL;
//static struct device* audioDevice = NULL;

struct audio_device {
    int minor_num;                      // Device minor number
    struct cdev cdev;                   // Character device structure
    struct platform_device * pdev;      // Platform device pointer
    struct device* dev;                 // device (/dev)

    phys_addr_t phys_addr;              // Physical address
    u32 mem_size;                       // Allocated mem space size
    u32* virt_addr;                     // Virtual address

    // Add any items to this that you need
    dev_t devt;
    u32 irq;
    bool deviceProbed;
};


size_t audio_currentPos;
size_t audio_bufferSize;
int loop = STOP_LOOPING;
int32_t* audio_Buffer;


static struct audio_device audioDevice = {
  .deviceProbed = false,
};

static struct of_device_id audioDevicePlatform_match[] /*__devinitdata*/ = {
  { .compatible = "byu,ecen427-audio_codec", },
  { /*End of Table*/}
};
//MODULE_DEVICE_TABLE(of, audiDevicePlatform_match);


static struct platform_driver audioDevicePlatformDriver = {
  .driver = {
    .name = MODULE_NAME,
    .owner = THIS_MODULE,
    .of_match_table = audioDevicePlatform_match,
  },
   .probe = audio_probe,
   .remove = audio_remove,
};

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = audioRead,
  .write = audioWrite,
  .unlocked_ioctl = audioIoctl,
};


// This is called when Linux loads your driver
static int audio_init(void) {
  int ret;

  pr_info("%s: Initializing Audio Driver!!!\n", MODULE_NAME);
  // Get a major number for the driver -- alloc_chrdev_region; // pg. 45, LDD3.
  ret = alloc_chrdev_region(&audioDevice.devt, 0, NUM_DEVICES, MODULE_NAME);
  if (ret < 0)
  {
    printk(KERN_ALERT "%s: failed to register a major number\n", MODULE_NAME);
    goto MAJOR_NUMBER_ERROR;
  }
  majorNumber = MAJOR(audioDevice.devt);
  printk(KERN_INFO "%s: registered correctly with major number %d\n", MODULE_NAME, majorNumber);

  // Create a device class. -- class_create()
  audioClass = class_create(THIS_MODULE, MODULE_NAME);
  if (IS_ERR(audioClass))
  {
    printk(KERN_ALERT "%s: failed to create class\n", MODULE_NAME);
    ret =  PTR_ERR(audioClass);
    goto CLASS_CREATE_ERROR;
  }
  printk(KERN_INFO "%s: class created properly with address 0x%x\n", MODULE_NAME, (uint32_t) audioClass);

  // Register the driver as a platform driver -- platform_driver_register
  ret = platform_driver_register(&audioDevicePlatformDriver);
  if (ret < 0)
  {
    printk(KERN_ALERT "%s: failed to register driver\n", MODULE_NAME);
    goto PLATFORM_DRIVER_ERROR;
  }
  // If any of the above functions fail, return an appropriate linux error code, and make sure
  // you reverse any function calls that were successful.
  printk(KERN_INFO "%s: audio_init finished successfully\n", MODULE_NAME);


  return 0;
PLATFORM_DRIVER_ERROR:
  class_destroy(audioClass);
CLASS_CREATE_ERROR:
  unregister_chrdev_region(audioDevice.devt, NUM_DEVICES);
MAJOR_NUMBER_ERROR:
  return ret;
}

// This is called when Linux unloads your driver
static void audio_exit(void) {
  printk(KERN_INFO "%s: audio_exit has been called\n", MODULE_NAME);
  // platform_driver_unregister
  platform_driver_unregister(&audioDevicePlatformDriver);
  // class_unregister and class_destroy
  printk(KERN_INFO "%s: unregister class\n", MODULE_NAME);
  class_unregister(audioClass);
  printk(KERN_INFO "%s: destroy class\n", MODULE_NAME);
  class_destroy(audioClass);
  // unregister_chrdev_region
  unregister_chrdev_region(audioDevice.devt, NUM_DEVICES);
  printk(KERN_INFO "%s: audio_exit has finished\n", MODULE_NAME);
  return;
}

// Called by kernel when a platform device is detected that matches the 'compatible' name of this driver.
// Purpose:
//    allocated resouces, inintalize character driver
//    create device file for charDrive ops.
//    Reserves memory for the driver
//    Creates a virtualPointer for accessing hardware
// Takes:
//    pdev, physicla device that matches compatibiliity decleration
// Returns success of Probe;
// Uses:
//  audioDevice, struct where all allocation memory is stored
//
static int audio_probe(struct platform_device *pdev) {
  int ret;
  struct resource * allocResource;
  struct resource * memResource;
  printk(KERN_INFO "%s: audio_probe has been called\n", MODULE_NAME);
  if (audioDevice.deviceProbed)
  {
    printk(KERN_INFO "%s: has already been probed\n", MODULE_NAME);
    return SUCCESS;
  }

  // Initialize the character device structure (cdev_init)
  cdev_init(&audioDevice.cdev, &fops);
  // Register the character device with Linux (cdev_add)
  ret = cdev_add(&audioDevice.cdev, audioDevice.devt, NUM_DEVICES);
  if (ret < 0)
  {
    printk(KERN_ALERT "%s: failed to add device structure\n", MODULE_NAME);
    goto CDEV_ADD_FAIL;
  }
  printk(KERN_INFO "%s: cdev registered\n", MODULE_NAME);
  // Create a device file in /dev so that the character device can be accessed from user space (device_create).
  audioDevice.dev = device_create(audioClass, NULL, audioDevice.devt, NULL, MODULE_NAME); //WORRIED THIS IS WRONG
  if (IS_ERR(audioDevice.dev))
  {
    printk(KERN_ALERT "%s: failed to create device\n", MODULE_NAME);
    ret = PTR_ERR(audioDevice.dev);
    goto DEVICE_CREATE_FAIL;
  }
  printk(KERN_INFO "%s: device created\n", MODULE_NAME);
  // Get the physical device address from the device tree -- platform_get_resource
  audioDevice.pdev = pdev;
  allocResource = platform_get_resource(audioDevice.pdev, IORESOURCE_MEM, 0);
  if (allocResource == NULL)
  {
    printk(KERN_ALERT "%s: failed to get resources\n", MODULE_NAME);
    ret = PTR_ERR(allocResource);
    goto PLATFORM_RESOURCE_ERROR;
  }
  printk(KERN_INFO "%s: platform get resource successs\n", MODULE_NAME);
  //store physical address
  audioDevice.phys_addr = allocResource->start;
  audioDevice.mem_size = allocResource->end - allocResource->start;
  printk(KERN_INFO "%s DEBUG: start: 0x%x end: 0x%x\n", MODULE_NAME, allocResource->start, allocResource->end);

  // Reserve the memory region -- request_mem_region
  memResource = request_mem_region(audioDevice.phys_addr, audioDevice.mem_size, MODULE_NAME);
  if ( memResource == NULL || IS_ERR(memResource))
  {
    printk(KERN_ALERT "%s: failed reserve memory region\n", MODULE_NAME);
    ret = PTR_ERR(memResource);
    goto MEM_REQUEST_ERROR;
  }
  printk(KERN_INFO "%s: request mem region successs\n", MODULE_NAME);
  // Get a (virtual memory) pointer to the device -- ioremap
  audioDevice.virt_addr = ioremap (allocResource->start, allocResource->end - allocResource->start);
  printk(KERN_INFO  "%s DEBUG: Virtual address 0x%x\n", MODULE_NAME, (uint32_t) audioDevice.virt_addr);
  if (IS_ERR(audioDevice.virt_addr))
  {
    printk(KERN_ALERT "%s: failed ioremap\n", MODULE_NAME);
    ret = PTR_ERR(audioDevice.virt_addr);
    goto IOREMAP_FAILURE;
  }
  // Get the IRQ number from the device tree -- platform_get_resource
  audioDevice.irq = platform_get_irq(pdev, 0); // performs the same task as platform_get_resource

  printk(KERN_INFO "%s: irq %d", MODULE_NAME, audioDevice.irq);
  // Register your interrupt service routine -- request_irq
  ret = request_irq((int)audioDevice.irq, audio_isr, 0x00/*FLAGS?*/, MODULE_NAME, audioDevice.pdev);
  if (ret < 0)
  {
    printk(KERN_ALERT "%s: request_irq failed\n", MODULE_NAME);
    goto REQUEST_IRQ_FAILED;
  }
  // If any of the above functions fail, return an appropriate linux error code, and make sure
  // you reverse any function calls that were successful.

  printk(KERN_INFO "%s: audio_probe has finished\n", MODULE_NAME);
  //now to enable IRQ interrupts?
  uint32_t statusFlags = ioread32((void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET));
  statusFlags |= ISR_ENABLE;
  printk(KERN_INFO "%s: address 0x%x\n", MODULE_NAME, (uint32_t) (audioDevice.virt_addr + ISR_ENABLE_OFFSET));
  printk(KERN_INFO "%s: Status flags b 0x%x\n", MODULE_NAME, statusFlags);
  printk(KERN_INFO "%s: Status flags disabled 0x%x\n", MODULE_NAME, statusFlags & ISR_DISABLE);
  iowrite32(statusFlags, (void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET) );

  audioDevice.deviceProbed = true;
  audio_Buffer = NULL;
  audio_currentPos = 0;
  audio_bufferSize = 0;

  return 0; //(success)

  REQUEST_IRQ_FAILED:
    printk(KERN_ALERT "%s: iounmap\n", MODULE_NAME);
    iounmap(audioDevice.virt_addr);
  IOREMAP_FAILURE:
    printk(KERN_ALERT "%s: release mem region\n", MODULE_NAME);
    release_mem_region(audioDevice.phys_addr, audioDevice.mem_size);
  MEM_REQUEST_ERROR:
    printk(KERN_ALERT "%s: unregister device\n", MODULE_NAME);
    platform_device_unregister(pdev);
  PLATFORM_RESOURCE_ERROR:
    printk(KERN_ALERT "%s: device destroy\n", MODULE_NAME);
    device_destroy(audioClass, audioDevice.devt);
  DEVICE_CREATE_FAIL:
    printk(KERN_ALERT "%s: cdev delete\n", MODULE_NAME);
    cdev_del(&audioDevice.cdev);
  CDEV_ADD_FAIL:
    //Do I need to delete the cdev here?
  return ret;
}

// Purpose:
//    Takes down audio driver when the audio hardware is removed
//    Should never be called unless linux is unloading this driver
// Takes:
//    pdev, physicla device that matches compatibiliity decleration
// Returns that the operation succeeded
//  Uses:
//    audioDevice, frees/unmaps all claimed memmory
static int audio_remove(struct platform_device * pdev) {
  printk(KERN_INFO "%s: audio_remove called\n", MODULE_NAME);
  // Release IRQ
  free_irq(audioDevice.irq, audioDevice.pdev);

  // iounmap
  printk("%s DEBUG: virtual address:0x%x", MODULE_NAME, (uint32_t) audioDevice.virt_addr);
  iounmap(audioDevice.virt_addr);
  // release_mem_region
  printk(KERN_INFO "%s DEBUG: physical address: 0x%x Memory Size: %d", MODULE_NAME,audioDevice.phys_addr, audioDevice.mem_size);
  release_mem_region(audioDevice.phys_addr, audioDevice.mem_size);
  // device_destroy
  device_destroy(audioClass, audioDevice.devt);
  // cdev_del
  cdev_del(&audioDevice.cdev);
  printk(KERN_INFO "%s: audio_remove successful", MODULE_NAME);
  return SUCCESS;
}

// Purpose:
//    Handles interupts triggered by the audio hardware
// Takes:
//    irq, The audio hardware only produces one type of interrupt, this can be ignored
//    dev_id, as there will never be more than one audio device per system, this value is ignored
static irqreturn_t audio_isr(int irq, void *dev_id) {
  printk(KERN_INFO "%s: audio_isr called %d %d\n", MODULE_NAME, audio_currentPos, audio_bufferSize);
  //stub
  if (audio_bufferSize == audio_currentPos )
  { // Clip is finished playing, disable interupts
    printk(KERN_INFO "%s: bufferEmpty, disabling ISR\n", MODULE_NAME);
    uint32_t statusFlags = ioread32((void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET));
    statusFlags &= ISR_DISABLE;
    iowrite32(statusFlags, (void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET) );
  }
  else
  { // more samples need to be written to hardware fifo
    int32_t availableSpace;
    int32_t usedFifoSpace = (int32_t)(ioread32((void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET)) & 0x7FE) >> 1;
    availableSpace = TOTAL_SPACE - usedFifoSpace;
    printk(KERN_INFO "%s: used Space %u availableSpace %d \n", MODULE_NAME, usedFifoSpace, availableSpace);
    while (availableSpace > 1)
    { //While there is space, fill it

        iowrite32(audio_Buffer[audio_currentPos], ((void*) (audioDevice.virt_addr + AUDIO_RIGHT_OFFSET)));
        iowrite32(audio_Buffer[audio_currentPos], ((void*) (audioDevice.virt_addr + AUDIO_LEFT_OFFSET)));
        ++audio_currentPos;
        usedFifoSpace = (int32_t)(ioread32((void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET)) & 0x7FE) >> 1;
        availableSpace = TOTAL_SPACE - usedFifoSpace;
        if (audio_currentPos >= audio_bufferSize)
        { //Stop writing if the clip is over
          break;
        }
    }
    if (loop && audio_currentPos >= audio_bufferSize)
    { //The sound should be looped, and audio_currentPos has reached the end of the buffer
      audio_currentPos = 0;
    }
    usedFifoSpace = (ioread32((void *) (audioDevice.virt_addr + AUDIO_RIGHT_OFFSET)) & 0x1FF800) >> 11;
    printk(KERN_INFO "%s: writing to 0x%x Used fifo Space = %ud\n", MODULE_NAME, (uint32_t)(audioDevice.virt_addr + AUDIO_RIGHT_OFFSET), usedFifoSpace);
  }
  //  Disable interupts as soon as possible.
  //disable_irq(audioDevice.irq);
  return IRQ_HANDLED; //interrupt has been handled
}

// Purpose:
//    char driver for reading from audio device file
//  Takes:
//    a lot of variables for compatibitility, they are not used.
//  returns 1 if the audio is playing, 0 if it is no longer playing
static ssize_t audioRead(struct file * f, char __user * userBuffer, size_t readSize, loff_t * offset)
{
  // stub
  // printk(KERN_INFO "AUDIO: audioRead has been called\n");
  return !(audio_bufferSize == audio_currentPos);
}

// Purpose:
//    char driver for wriitng to audio device file
//  Takes:
//    f, file descriptor not used as there will only be 1 audio file
//    userBuffer, array from user space of the sound clip to play audioWrite does not claim ownership of the buffer
//    writeSize, length of userBuffer
//    offset, not used, as there is only 1 location in audio we want to write to
//  Returns SUCCESS, as audio device can always be written to.
static ssize_t audioWrite(struct file * f, const char __user * userBuffer, size_t writeSize, loff_t * offset)
{
  unsigned long copyResult;
  //  Disable interupts as soon as possible.
  uint32_t statusFlags = ioread32((void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET));
  //Enter Mutex
  statusFlags &= ISR_DISABLE;
  iowrite32(statusFlags, (void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET) );

  if (audio_Buffer != NULL)
  { //audio_Buffer needs to be freed
    kfree(audio_Buffer);
    audio_Buffer = NULL;
  }

  while (! (audio_Buffer = kmalloc(writeSize * sizeof(int32_t), GFP_KERNEL))); //Wait for the kernel to alloc memmory
  //stub
  printk(KERN_INFO "AUDIO: audioWrite has been called\n");
  audio_bufferSize = writeSize;
  audio_currentPos = 0;
  //COPY the entire audio, not just a small part of it
  while ( (copyResult = copy_from_user(audio_Buffer, (void *) userBuffer, writeSize * sizeof(int32_t))) != 0);
  printk(KERN_INFO "AUDIO: writeSize=%d B1 %x B2 %x B3 %x B4 %x\n", writeSize, audio_Buffer[0], audio_Buffer[1], audio_Buffer[2], audio_Buffer[3]);

  //  reenable interupts at the end of it
  statusFlags |= ISR_ENABLE;
  iowrite32(statusFlags, (void *) (audioDevice.virt_addr + ISR_ENABLE_OFFSET) );
  //Exit Mutex
  return SUCCESS;
}


static long audioIoctl(struct file * fp, unsigned int cmd, unsigned long arg)
{
  switch(cmd)
  {
    case IOCTL_START_LOOP:
    loop = START_LOOPING;
    return SUCCESS;
    break;
    case IOCTL_STOP_LOOP:
    loop = STOP_LOOPING;
    return SUCCESS;
    break;
    default:
    printk(KERN_INFO "AUDIO: UNKNOWN COMMAND\n");
    return -ENOTTY;
  }
}
