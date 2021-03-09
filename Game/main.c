#include <stdio.h>
#include "intc.h"
#include "gpio_button.h"
#include "gpio_switches.h"
#include "imageRender.h"
#include "player.h"
#include "alienControl.h"
#include "saucer.h"
#include "button_handler.h"
#include "globals.h"
#include "game_over_splash.h"
#include "bullet.h"
#include "bunker.h"
#include "alienBullets.h"
#include "audioPlayer.h"

#define BUTTON_INTC_MASK 0x2
#define GPIO_SWITCH_DEV "/dev/uio2"

void initAll(){
  printf("main.c\n");
  global_gameOver = false;
  //INIT FUNCITONS
  alienControl_init();
  imageRender_init();
  gpio_button_init();
  gpio_switch_init(GPIO_SWITCH_DEV);
  saucer_init();
  player_init();
  bullet_init();
  intc_init();
  bunker_init();
  audioPlayer_init();
}

void fit_isr()
{
  intc_ack_interrupt(INTC_FIT_INTERUPT_MASK);
}

int main() {
  initAll();

  uint32_t tick = 0;
  while (1)
  {
    uint32_t interupts = intc_wait_for_interrupt();
    if (interupts & INTC_FIT_INTERUPT_MASK)
    {
      fit_isr();

      player_tick();
      //printf("in alienBulletsaucerIsOut_tick\n");
      alienBullet_tick();
      //printf("in alienControl_tick\n");
      alienControl_tick();
      //printf("in saucer_tick\n");
      saucer_tick();
      button_handler_tick();
      game_over_splash_tick();
      bullet_tick();
      //--------------------NEW CODE FOR LAB 4 Milestone 3--------------
      audioPlayer_tick();
      //--------------------END OF NEW CODE-----------------------------


      if (tick % 100 == 0)
      {
        //printf("Tick %d\n\r", tick);
      }
      tick++;
    }
    else if (interupts & BUTTON_INTC_MASK)
    {
      button_isr();
    }
    else if (interupts & GPIO_SWITCH_MASK)
    {
      gpio_switch_acknowledge_interrupt();
    }


    intc_enable_uio_interrupts();
  }
  audioPlayer_exit();
  printf("END OF PROGRAM\n");
  return 0;
}

/*
TO DO
1. make aliens more efficient
2. Tank auto moving
3. Saucer auto move
4. Implement State machines
*/

//640x480
//gcc gpio_button.c imageRender.c intc.c sprites.c main.c player.c alienControl.c saucer.c
//gcc alienControl.c bullet.c bunker.c button_handler.c game_over_splash.c gpio_button.c imageRender.c intc.c main.c player.c saucer.c sprites.c
