

/*
 * Generic UIO device driver
 * This driver is intended as an example of how to interact with a UIO device.
 * It contains only the boiler-plate necessary to open, close, read from, and
 *  write to a UIO device.  There are usually additional initialization steps
 *  that must be performed before you can actually use a UIO device.
 *
 * ECEn 427
 * Benjamin James, Tanner Gaskin
 * BYU 2018
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "gpio_switches.h"

#define GPIO_SWITCH_MMAP_SIZE 0x1000  //size of memory to allocate
#define GPIO_IER_OFFSET 0x0128 // offset for the interrupt enable register
#define GPIO_GIER_OFFSET 0x011C //offset for the GIE register
#define GPIO_IPISR_OFFSET 0x120 //offset for the IPISR
#define GPIO_ENABLE_SWITCH_INTERUPTS (1<<31) //the most significant bit is used to enable switch interrupts
#define GPIO_ENABLE_CHANNEL_0 0x01 //channel 0 uses the lsb
#define GPIO_ACKNOWLEDGE_CHANNEL_0 0x01


/********************************** globals ***********************************/
static int fileDescriptor; 		// this is a file descriptor that describes an open UIO device
static char *base;	// this is the virtual address of the UIO device registers


/********************************* fileDescriptorunctions **********************************/
// PURPOSE:		initializes the uio driver,
// ARGUMENTS:	devDevice - used to connect to the switch directory
// RETURNS:  	-1 if failed in error, 0 otherwise
int32_t gpio_switch_init(char devDevice[]) {

	//open the device
	fileDescriptor = open(devDevice, O_RDWR);
	if(fileDescriptor == GPIO_SWITCH_FAILURE) {
		//file descriptors have to be > 0 to be valid
		return GPIO_SWITCH_FAILURE;
	}

	//memory map the physical address of the hardware into virtual address space
	base = mmap(NULL, GPIO_SWITCH_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, MMAP_OFFSET);
	if(base == MAP_FAILED) { //if the file was not able to open
		return GPIO_SWITCH_FAILURE; //return the value indicating that that the file did not open
	}

	/* put hardware setup here */
	//Enable global interrupts from GPIO switch
	*((volatile uint32_t *) (base + GPIO_GIER_OFFSET) ) = GPIO_ENABLE_SWITCH_INTERUPTS;
  *((volatile uint32_t *) (base + GPIO_IER_OFFSET) ) = GPIO_ENABLE_CHANNEL_0;

	return GPIO_SWITCH_SUCCESS; //indicate file open success
}

// PURPOSE:		read from a register of the UIO device
// ARGUMENTS:	None
// RETURNS:		-1 initialization failed, 1 otherwise
uint32_t gpio_switch_read() {
	return *((volatile uint32_t *)(base));
}

// PURPOSE:		close the UIO device
// 						this function must be called after all read/write operations are done
// 						to properly unmap the memory and close the file descriptor
// ARGUMENTS: None
// RETURNS:		The value of the 2 switches as the 2 lsb's
void gpio_switch_exit() {
	munmap(base, GPIO_SWITCH_MMAP_SIZE); //unmap memory
	close(fileDescriptor); //close the UIO device
}

// PURPOSE:		Acknowledge that a switch interrupt has occured
// ARGUMENTS:	None
// RETURNS:		None
void gpio_switch_acknowledge_interrupt()
{	//ptr + GPIO_IPISR_OFFSET refers to the Interrupt Status Register address
	//Setting bit 0 this register acknowledges the interrupt
	// flips the channel 0 bit of the GPIO ISR register to akcnowledge the interrupt
	*((volatile uint32_t *) (base + GPIO_IPISR_OFFSET)) = GPIO_ACKNOWLEDGE_CHANNEL_0;
}
