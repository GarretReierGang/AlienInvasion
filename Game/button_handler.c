#include "button_handler.h"
#include "gpio_button.h"
#include "intc.h"
#include <stdbool.h>

#define GPIO_BUTTON_ID "/dev/uio1" //path to the button id
#define DEBOUNCE_TICKS 1
#define BUTTON_PRESSED ( (currentlyPressedButton) & buttonIdentifier)
#define BUTTON_NOT_PRESSED !( (currentlyPressedButton) & buttonIdentifier)
#define DEBUG_MESSAGE //printf("Button %d pressed\n\r", (buttonIdentifier-1));
#define DEBUG_MESSAGE2 //printf("Button %d released\n\r", (buttonIdentifier-1));

//STATE MACHINE
typedef enum button_state {
  button_init = 0, //initial button handler state, initializes buttons
  button_waitForPress, // button state is stable
  button_debouncePress, // button state is unstable
  button_waitForRelease,
  button_debounceRelease
} button_state;

//GLOBAL VARIABLES
static uint32_t currentlyPressedButton;
static bool button_stateChange;
static enum button_state button0State = button_init;
static enum button_state button1State = button_init;
static enum button_state button2State = button_init;
static enum button_state button3State = button_init;

// Purpose: handles button interrupt service routine
// Sets: button_stateChange flag
// Sets: temporary button value
void button_isr() {
  button_stateChange = true;
  currentlyPressedButton = gpio_button_read();
  gpio_button_acknowledge_interupt();
  intc_ack_interrupt(BUTTON_INTC_MASK);
}

// Purpose: Performs the Moores actions of the state machine
// Argument currentState: The current state of the state machine
// Argument buttonTimer: The time for debouncing the buttons
// Return type: void
void stateAction(enum button_state* currentState, uint32_t* buttonTimer){
  switch (*currentState)
  { //Only moore action is the button_debounce
    case button_init:
    //do nothing if we are initializing
      break;

    default:
          (*buttonTimer)++;
      break;
  }
}

// Purpose: handles state change for a given buttonState machine, performing requisite operations on the timer, and flags
// Requires: nothing outside of this file
// Return type: void
void stateChange(enum button_state* currentState, uint32_t* buttonTimer, uint32_t buttonIdentifier, bool* buttonPressFlag, bool* buttonReleaseFlag) {
  switch(*currentState)
  { //State machines
      case button_init: //intial state, initialized button_gpio

        if (BUTTON_PRESSED)
        { //Button started out pressed
          (*currentState) = button_waitForRelease;
        }
        else
        { //button started out released
          (*currentState) = button_waitForPress;
        }
        (*buttonTimer) = 0;
      break;
      case button_waitForPress: // button state is stable as not being pressed, wait for it to be pressed
        if (button_stateChange && BUTTON_PRESSED)
        { //Debounce the button press, and start the timer counting
          (*currentState) = button_debouncePress;
          (*buttonTimer) = 0;
        }
      break;
      case button_debouncePress: //
        if (button_stateChange && !BUTTON_PRESSED)
        { //button was released before it oculd be debounced, reset the count
          (*currentState) = button_waitForPress;
          (*buttonTimer) = 0;
        }
        else if (*buttonTimer >= DEBOUNCE_TICKS)
        { //button has been debounced, set flag and wait for it to be released
          (*currentState) = button_waitForRelease;
          (*buttonPressFlag) = true;
          DEBUG_MESSAGE
        }
      break;
      case button_waitForRelease:
        if (button_stateChange && !BUTTON_PRESSED)
        { //Button is released, start debouncing it.
          (*currentState) = button_debounceRelease;
          (*buttonTimer) = 0;
        }
      break;
      case button_debounceRelease:
        if (button_stateChange && BUTTON_PRESSED)
        { //The button is bouncing, don't trigger a button release yet
          (*currentState) = button_waitForRelease;
          (*buttonTimer) = 0;
        }
        else if ( (*buttonTimer) >= DEBOUNCE_TICKS)
        { //Button Has been released, signal a button release
          (*currentState) = button_waitForPress;
          (*buttonPressFlag) = false;
          (*buttonReleaseFlag) = true;
          DEBUG_MESSAGE2
        }
      break;
  }
}

// Purpose: debounce button click
// Sets: which button is currently pressed once it is debounced
// Sets: which button was last pressed
void button_handler_tick() {
    // state actions
    stateAction(&button0State, &button0Timer);
    stateAction(&button1State, &button1Timer);
    stateAction(&button2State, &button2Timer);
    stateAction(&button3State, &button3Timer);

    //Switch states
    stateChange(&button0State, &button0Timer, BUTTON0, &button0_pressed, &button0_released);
    stateChange(&button1State, &button1Timer, BUTTON1, &button1_pressed, &button1_released);
    stateChange(&button2State, &button2Timer, BUTTON2, &button2_pressed, &button2_released);
    stateChange(&button3State, &button3Timer, BUTTON3, &button3_pressed, &button3_released);
    button_stateChange = false;
}

// Purpose: clears all flags
// Arguments: None
// Return type: void
void button_handler_clear_flags(){
  button0_released = false;
  button1_released = false;
  button2_released = false;
  button3_released = false;
}
