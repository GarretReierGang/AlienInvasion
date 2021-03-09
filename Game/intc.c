#include "intc.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>

//MACROS
#define INTC_ENABLE_INTERUPTS 0x07
#define INTC_FAILURE -1 //failed to initailize intc
#define ACTIVATED_INTERUPT_BITMASK_SIZE 4
#define INTC_ISR_OFFSET 0x00 //offset for the interrupt status register
#define INTC_IER_OFFSET 0x08 //offset for the interrupt enable register
#define INTC_IAR_OFFSET 0x0C //offset for the interrupt ackowledge register
#define INTC_SIE_OFFSET 0x10 //offset for the SIE
#define INTC_CIE_OFFSET 0x14 //offset for the CIE
#define INTC_MER_OFFSET 0x1C //offset for the Master Enable Register
#define INTC_ENABLE_MER 0x03 //masking for the MER
#define MMAP_OFFSET 0
#define INTC_MMAP_SIZE 0x1000 //allocated space for file
#define GPIO_SWITCH_INTC_MASK (0x4)
#define GPIO_BUTTON_INTC_MASK 0x2
#define GPIO_FIT_INTC_MASK 0x1

//GLOBAL VARIABLES
static int f; //gets the file for the interrupt controller
static char *ptr; //ptr is a pointer to the base address
static uint32_t activatedInterupts; //used to indentify which interrupts have been triggered
static uint32_t whatIWasBefore; //stores the previous value of the interrupts
static const uint32_t enableInterupts = INTC_ENABLE_INTERUPTS;

void intc_waitTicks(uint32_t ticks){
  static uint32_t waitCounter;
  // PUROSE:    Wait a some time for debugging purposes
  // ARGUMENTS: ticks - how many interupt ticks to wait
  // RETURNS:   void
  while (waitCounter < ticks){
    waitCounter++;
    printf("waiting for interupt\n");
    intc_wait_for_interrupt();
    intc_ack_interrupt(INTC_FIT_INTERUPT_MASK);
    intc_enable_uio_interrupts();
    printf("interupt occured\n");
  }
}

// PUROSE:    Initializes the driver (opens UIO file and calls mmap)
//            This must be called before calling any other intc_* functions
// ARGUMENTS: devDevice - The file path to the uio dev file
// RETURNS:   A negative error code on error, INTC_SUCCESS otherwise
int32_t intc_init()
{
  f = open(INTC_ID, O_RDWR); //attempt to open directory at devDevice
  if (f == INTC_FAILURE) //if the file did  not open
  {
    return INTC_FAILURE; //indicate the file failed to read
  }

  //GET BASE ADDRESS
  ptr = mmap(NULL,INTC_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, f, MMAP_OFFSET);
  if (ptr == MAP_FAILED) // if ptr failed to read
  {
    return INTC_FAILURE; //indicate the ptr failed to read
  }
  //do stuff to setup intc controller
  whatIWasBefore = *((volatile uint32_t *) (ptr + INTC_IER_OFFSET)); //remembering what I was so that I can clear myself after the fact
  *((volatile uint32_t *) (ptr + INTC_MER_OFFSET)) = INTC_ENABLE_MER; //set the MER to the correct value
  write(f, (void *) &enableInterupts, ACTIVATED_INTERUPT_BITMASK_SIZE); //write f file to enable interrupts

  //recently added
  intc_irq_enable(GPIO_BUTTON_INTC_MASK | GPIO_FIT_INTC_MASK | GPIO_SWITCH_INTC_MASK);
  intc_enable_uio_interrupts();

  return INTC_SUCCESS; //indicate the intc was successfully initialized
}

// PURPOSE:   Called to exit the driver (unmap and close UIO file)
// ARGUMENTS: None
// RETURNS:   None
void intc_exit()
{
  //exit the driver
  *((volatile uint32_t *)(ptr + INTC_IER_OFFSET)) = whatIWasBefore & *((volatile uint32_t *)(ptr + INTC_IER_OFFSET));
  munmap(ptr, INTC_MMAP_SIZE); //unmap the UIO file
  close(f); // close the UIO file
}

// PURPOSE:   This function will block until an interrupt occurrs
// ARGUMENTS: None
// RETURNS:   Bitmask of activated interrupts
uint32_t intc_wait_for_interrupt()
{
  //Stay in this function until any interrupt occurs
  read(f, (void *) &activatedInterupts, ACTIVATED_INTERUPT_BITMASK_SIZE); //get interrupt status
  return *((volatile uint32_t *) (ptr + INTC_ISR_OFFSET)); //return which interrupt fired
}

// PURPOSE:   Acknowledge interrupt(s) in the interrupt controller
// ARGUMENTS: irq_mask - Bitmask of interrupt lines to acknowledge.
// RETURNS:   None
void intc_ack_interrupt(uint32_t irq_mask)
{
  //ptr + INTC_IAR_OFFSET refers to Interrupt Acknowledge register
  //Assigning this register to the value irq_mask acknowledges that
  //  an interrupt has occured
   *((volatile uint32_t *)(ptr + INTC_IAR_OFFSET)) = irq_mask;
}

// PURPOSE:   Instruct the UIO to enable interrupts for this device in Linux
//            (see the UIO documentation for how to do this)
// ARGUMENTS: None
// RETURNS:   None
void intc_enable_uio_interrupts()
{
  write(f, (void *) &enableInterupts, ACTIVATED_INTERUPT_BITMASK_SIZE);
}

// PURPOSE:     Enable interrupt line(s)
//              This function only enables interrupt lines, ie, a 0 bit in irq_mask
//              will not disable the interrupt lineINTC_ENABLE_INTERUPTS
// ARGUMENTS:   irq_mask - Bitmask of lines to enable
// RETURNS:     None
void intc_irq_enable(uint32_t irq_mask)
{
  //Assigning register: sie to the value irq_mask will enable interrupt lines
	*((volatile uint32_t *)(ptr + INTC_SIE_OFFSET)) = irq_mask;
}

// PURPOSE:   Disables interrupt lines
// ARGUMENTS: irq_mask - Bitmask of lines to enable
// RETURNS:   None
void intc_irq_disable(uint32_t irq_mask)
{
  //Writing irq_mask to CIE will disable interupt lines
  *((volatile uint32_t *)(ptr + INTC_CIE_OFFSET)) = irq_mask;
}
