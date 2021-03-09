#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h>
#include<linux/kobject.h>

#include <stdint.h>



#define MODULE_NAME "pit"
#define INTERUPT_FILE "_interupt"
#define TIMER_FILE "_timer_delay"
volatile int etx_value = 0;

#define PIT_CONTROL_OFFSET (0x00 / sizeof(uint32_t)
#define PIT_DELAY_TIME_OFFSET (0x04 /sizeof(uint32_t))


dev_t dev = 0;
static struct class *dev_class;
static struct cdev pit_cdev;
struct kobject *kobj_ref;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

/*************** Driver Fuctions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp,
                char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp,
                const char *buf, size_t len, loff_t * off);

/*************** Sysfs Fuctions **********************/
static ssize_t sysfs_show(struct kobject *kobj,
                struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj,
                struct kobj_attribute *attr,const char *buf, size_t count);

struct kobj_attribute pit_interupt = (MODULE_NAME INTERUPT_FILE, 0660, sysfs_show, sysfs_store);
struct kobj_attribute pit_timer_delay = (MODULE_NAME INTERUPT_FILE, 0660, sysfs_show, sysfs_store);

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .release        = etx_release,
};

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


static ssize_t sysfs_show(struct kobject *kobj,
                struct kobj_attribute *attr, char *buf)
{
        printk(KERN_INFO "Sysfs - Read!!!\n");
        return sprintf(buf, "%d", etx_value);
}

static ssize_t sysfs_show_interuptStatus(struct kobject* kobj,
                struct kobj_attribute *attr, char *buf)
{
  return sprintf(buf, "x%x", )
}


static ssize_t sysfs_store(struct kobject *kobj,
                struct kobj_attribute *attr,const char *buf, size_t count)
{
        printk(KERN_INFO "Sysfs - Write!!!\n");
        sscanf(buf,"%d",&etx_value);
        return count;
}

static int etx_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Opened...!!!\n");
        return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Closed...!!!\n");
        return 0;
}

static ssize_t etx_read(struct file *filp,
                char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Read function\n");
        return 0;
}
static ssize_t etx_write(struct file *filp,
                const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Write Function\n");
        return 0;
}


static int __init etx_driver_init(void)
{
  /*Allocating Major number*/
  if((alloc_chrdev_region(&dev, 0, 1, MODULE_NAME)) <0){
          printk(KERN_INFO "Cannot allocate major number\n");
          return -1;
  }
  printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

  /*Creating cdev structure*/
  cdev_init(&pit_cdev,&fops);

  /*Adding character device to the system*/
  if((cdev_add(&pit_cdev,dev,1)) < 0){
      printk(KERN_INFO "Cannot add the device to the system\n");
      goto r_class;
  }

  /*Creating struct class*/
  if((dev_class = class_create(THIS_MODULE,"pit_class")) == NULL){
      printk(KERN_INFO "Cannot create the struct class\n");
      goto r_class;
  }

  /*Creating device*/
  if((device_create(dev_class,NULL,dev,NULL,"pit_device")) == NULL){
      printk(KERN_INFO "Cannot create the Device 1\n");
      goto r_device;
  }

  /*Creating a directory in /sys/kernel/ */
  kobj_ref = kobject_create_and_add("pit_sysfs",kernel_kobj);

  /*Creating sysfs file for etx_value*/
  if(sysfs_create_file(kobj_ref,&pit_interupt.attr)){
          printk(KERN_INFO"Cannot create sysfs file......\n");
          goto r_sysfs;
  }
  printk(KERN_INFO "Device Driver Create kobj_ref...Done!!!\n");
  if(sysfs_create_file(kobj_ref,&pit_timer_delay.attr)){
      printk(KERN_INFO"Cannot create sysfs file......\n");
      goto r_sysfs;
  }
  printk(KERN_INFO "Device Driver Insert...Done!!!\n");
  return 0;

r_sysfs2:
  sysfs_remove_file(kernel_kobj, &pit_timer_delay.attr);
r_sysfs:
  kobject_put(kobj_ref);
  sysfs_remove_file(kernel_kobj, &pit_interupt.attr);

r_device:
  class_destroy(dev_class);
r_class:
  unregister_chrdev_region(dev,1);
  cdev_del(&pit_cdev);
  return -1;
}

void __exit etx_driver_exit(void)
{
        kobject_put(kobj_ref);
        sysfs_remove_file(kernel_kobj, &pit_attr.attr);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&pit_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com or admin@embetronicx.com>");
MODULE_DESCRIPTION("A simple device driver - SysFs");
MODULE_VERSION("1.8");
