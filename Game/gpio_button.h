#ifndef GPIO_BUTTON_H
#define GPIO_BUTTON_H

/*
 * gpio_button_uio_example.h
 *
 * gpio_button_7
 * Garret Gang, Tanner Gaskin
 * BYU 2019
 */

#include <stdint.h>


/*********************************** macros ***********************************/
#define GPIO_BUTTON_FAILURE       -1      //error return value
#define GPIO_BUTTON_SUCCESS     0       //success return value
#define MMAP_OFFSET             0
#define GPIO_BUTTON_ID "/dev/uio1" //path to the button id
#define GPIO_BUTTON_INTC_MASK 0x2
#define GPIO_FIT_INTC_MASK 0x1
#define GPIO_BUTTON0 (1<<0)
#define GPIO_BUTTON1 (1<<1)
#define GPIO_BUTTON2 (1<<2)
#define GPIO_BUTTON3 (1<<3)
#define GPIO_SWITCH_INTC_MASK (0x4)


/**************************** function prototypes *****************************/
// PURPOSE:	  	initializes the uio driver
// ARGUMENTS:		None
// RETURNS:   	-1 if failed in error, 0 otherwise
int32_t gpio_button_init();

// PURPOSE:		Read from a register of the UIO device
// ARGUMENTS:	None
// RETURNS:		The current state of all 4 buttons in the lsb's
uint32_t gpio_button_read();

// PURPOSE:		close the UIO device
//						this function must be called after all read/write operations are done
//						to properly unmap the memory and close the file descriptor
// ARGUMENTS:	None
// RETURNS:		None
void gpio_button_exit();

// PURPOSE:		Acknowledges that an interupt has occured
// ARGUMENTS:	None
// RETURNS:		None
void gpio_button_acknowledge_interupt();
#endif
