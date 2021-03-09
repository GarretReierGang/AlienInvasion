

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
#include "gpio_button.h"

#define GPIO_BUTTON_MMAP_SIZE   0x1000  //size of memory to allocate
#define GPIO_IER_OFFSET 0x0128
#define GPIO_GIER_OFFSET 0x011C
#define GPIO_IPISR_OFFSET 0x120
#define GPIO_ENABLE_BUTTON_INTERUPTS (1<<31)
#define GPIO_ENABLE_CHANNEL_0 0x01
#define GPIO_ACKNOWLEDGE_CHANNEL_0 0x01

/********************************** globals ***********************************/
static int f; 		// this is a file descriptor that describes an open UIO device
static char *ptr;	// this is the virtual address of the UIO device registers


/********************************* functions **********************************/
// PURPOSE:	  	initializes the uio driver
// ARGUMENTS:		None
// RETURNS:   	-1 if failed in error, 0 otherwise
int32_t gpio_button_init() {

	//open the device
	f = open(GPIO_BUTTON_ID, O_RDWR);
	if(f == GPIO_BUTTON_FAILURE) {
		//file descriptors have to be > 0 to be valid
		return GPIO_BUTTON_FAILURE;
	}

	//memory map the physical address of the hardware into virtual address space
	ptr = mmap(NULL, GPIO_BUTTON_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, f, MMAP_OFFSET);
	if(ptr == MAP_FAILED) { //if the file was not able to open
		return GPIO_BUTTON_FAILURE; //return the value indicating that that the file did not open
	}

	/* put hardware setup here */
	//Enable global interupts from GPIO button
	*((volatile uint32_t *) (ptr + GPIO_GIER_OFFSET) ) = GPIO_ENABLE_BUTTON_INTERUPTS;
  *((volatile uint32_t *) (ptr + GPIO_IER_OFFSET) ) = GPIO_ENABLE_CHANNEL_0;

	return GPIO_BUTTON_SUCCESS; //indicate file open success
}

// PURPOSE:		Read from a register of the UIO device
// ARGUMENTS:	None
// RETURNS:		The current state of all 4 buttons in the lsb's
uint32_t gpio_button_read() {
	return *((volatile uint32_t *)(ptr)); //dereference ptr, which contains the button values
}

// PURPOSE:		close the UIO device
//						this function must be called after all read/write operations are done
//						to properly unmap the memory and close the file descriptor
// ARGUMENTS:	None
// RETURNS:		None
void gpio_button_exit() {
	munmap(ptr, GPIO_BUTTON_MMAP_SIZE); //unmap memory
	close(f); //close the UIO device
}

// PURPOSE:		Acknowledges that an interupt has occured
// ARGUMENTS:	None
// RETURNS:		None
void gpio_button_acknowledge_interupt()
{
	//ptr + GPIO_IPISR_OFFSET refers to the Interrupt Status Register address
	//Setting bit 0 this register acknowledges the interrupt
	*((volatile uint32_t *) (ptr + GPIO_IPISR_OFFSET)) = GPIO_ACKNOWLEDGE_CHANNEL_0;
}
