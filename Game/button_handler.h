#ifndef BUTTON_HANDLER_H
#include <stdint.h>
#include <stdbool.h>
#define BUTTON_HANDLER_H


// flag, button_handler sets to true
// other state machines can set to false
bool button0_released;
bool button1_released;
bool button2_released;
bool button3_released;


// flag button handler sets to true/false
bool button0_pressed;
bool button1_pressed;
bool button2_pressed;
bool button3_pressed;

//Timer other state machines can read this, only the button_handler
//writes to the timers
uint32_t button0Timer;
uint32_t button1Timer;
uint32_t button2Timer;
uint32_t button3Timer;

#define BUTTON0 (1 << 0)
#define BUTTON1 (1 << 1)
#define BUTTON2 (1 << 2)
#define BUTTON3 (1 << 3)
#define BUTTON_INTC_MASK 0x2


// Purpose: debounce button click
// Sets: which button is currently pressed once it is debounced
// Sets: which button was last pressed
void button_handler_tick();

// Purpose: handles button interrupt service routine
// Sets: button_stateChange flag
// Sets: temporary button value
void button_isr();

// purpose: clears all of the clearable flags
void button_handler_clear_flags();

#endif
