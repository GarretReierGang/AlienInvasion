/*
 * gpio_switch.h
 *
 * ggpio_switch
 * Garret Gang, Tanner Gaskin
 * BYU 2019
 */

#include <stdint.h>


/*********************************** macros ***********************************/
#define GPIO_SWITCH_FAILURE       -1      //error return value
#define GPIO_SWITCH_SUCCESS     0       //success return value
#define MMAP_OFFSET             0
#define GPIO_SWITCH_MASK        0x04


/**************************** function prototypes *****************************/
// PURPOSE:		initializes the uio driver,
// ARGUMENTS:	devDevice - used to connect to the switch directory
// RETURNS:  	-1 if failed in error, 0 otherwise
int32_t gpio_switch_init(char devDevice[]);

// PURPOSE:		read from a register of the UIO device
// ARGUMENTS:	None
// RETURNS:		-1 initialization failed, 1 otherwise
uint32_t gpio_switch_read();

// PURPOSE:		close the UIO device
// 						this function must be called after all read/write operations are done
// 						to properly unmap the memory and close the file descriptor
// ARGUMENTS: None
// RETURNS:		The value of the 2 switches as the 2 lsb's
void gpio_switch_exit();

// PURPOSE:		Acknowledge that a switch interrupt has occured
// ARGUMENTS:	None
// RETURNS:		None
void gpio_switch_acknowledge_interrupt();
