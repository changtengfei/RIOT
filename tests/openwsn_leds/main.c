/**
\brief This is a program which shows how to use the bsp modules for the board
       and leds.

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform.

Load this program on your boards. The LEDs should start blinking furiously.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board_ow.h"
#include "leds.h"
#include "xtimer.h"

void some_delay(void);

/**
\brief The program starts executing here.
*/
int main(void) {uint8_t i;
   
   board_init_ow();
   
   while(1) {
      puts("Blink from beginning :-)");
      // error LED functions
      puts("leds_error_blink");
      leds_error_blink();       xtimer_sleep(1);
      leds_error_off();         xtimer_sleep(1);
      
      // radio LED functions
      puts("leds_radio_toggle");
      leds_radio_toggle();      xtimer_sleep(1);
      leds_radio_off();         xtimer_sleep(1);
      
      // sync LED functions
      puts("leds_sync_toggle");
      leds_sync_toggle();       xtimer_sleep(1);
      leds_sync_off();          xtimer_sleep(1);
      
      // debug LED functions
      puts("leds_debug_toggle");
      leds_debug_toggle();      xtimer_sleep(1);
      leds_debug_off();         xtimer_sleep(1);
      
      // all LED functions
      puts("leds_all_toggle");
      leds_all_off();           xtimer_sleep(1);
      leds_all_on();            xtimer_sleep(1);
      leds_all_off();           xtimer_sleep(1);
      leds_all_toggle();        xtimer_sleep(1);
      
      // LED increment function
      leds_all_off();           xtimer_sleep(1);
      puts("leds_circular_shift");
      for (i=0;i<9;i++) {
         leds_circular_shift(); xtimer_sleep(1);
      }
   }

   return 0;
}

void some_delay(void) {
   volatile uint16_t delay;
   for (delay=0xffff;delay>0;delay--);
}