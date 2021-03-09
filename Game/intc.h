#ifndef INTC_H
#define INTC_H
#include <stdint.h>

#define INTC_SUCCESS 0
#define INTC_FIT_INTERUPT_MASK 1
#define INTC_ID "/dev/uio4" //path to the ic id

void intc_waitTicks(uint32_t ticks);

// PUROSE:    Initializes the driver (opens UIO file and calls mmap)
//            This must be called before calling any other intc_* functions
// ARGUMENTS: devDevice - The file path to the uio dev file
// RETURNS:   A negative error code on error, INTC_SUCCESS otherwise
int32_t intc_init();

// PURPOSE:   Called to exit the driver (unmap and close UIO file)
// ARGUMENTS: None
// RETURNS:   None
void intc_exit();

// PURPOSE:   This function will block until an interrupt occurrs
// ARGUMENTS: None
// RETURNS:   Bitmask of activated interrupts
uint32_t intc_wait_for_interrupt();

// PURPOSE:   Acknowledge interrupt(s) in the interrupt controller
// ARGUMENTS: irq_mask - Bitmask of interrupt lines to acknowledge.
// RETURNS:   None
void intc_ack_interrupt(uint32_t irq_mask);

// PURPOSE:   Instruct the UIO to enable interrupts for this device in Linux
//            (see the UIO documentation for how to do this)
// ARGUMENTS: None
// RETURNS:   None
void intc_enable_uio_interrupts();

// PURPOSE:     Enable interrupt line(s)
//              This function only enables interrupt lines, ie, a 0 bit in irq_mask
//              will not disable the interrupt lineINTC_ENABLE_INTERUPTS
// ARGUMENTS:   irq_mask - Bitmask of lines to enable
// RETURNS:     None
void intc_irq_enable(uint32_t irq_mask);

// PURPOSE:   Disables interrupt lines
// ARGUMENTS: irq_mask - Bitmask of lines to enable
// RETURNS:   None
void intc_irq_disable(uint32_t irq_mask);
#endif
