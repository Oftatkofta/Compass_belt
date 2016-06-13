#include <string.h>
#include <stdint.h>
#include <util/delay.h>
#include <matrix8x8/matrix8x8.h>
#include "button.h"

#define DEBOUNCE_DOWN 25 /* ms */
#define DEBOUNCE_UP   25 /* ms */

/*** button() -- check if button is pressed; wait for release if so

Tests if the input button is pressed. If so:
   o- illuminate all the segments in the display (as both a confirmation
      that the input was recognized, and as a lamp test)
   o- wait for a debounce period
   o- wait until button is released
   o- wait for a second debounce period

Returns 0 if no button press was detected, or returns non-0 if the button
was pressed and released.
***/
int button(void) {
   uint8_t img[8];

   if(!BTN_PRESSED) return 0;
   memset(img, 0xff, 8); matrix8x8_draw(img); /* lamp test */
   _delay_ms(DEBOUNCE_DOWN); /* wait for contacts to settle closed */
   while(BTN_PRESSED); /* wait for button to be released */
   _delay_ms(DEBOUNCE_UP); /* wait for contacts to settle open */

   return 1;
}

/*** button_sleep() -- wait for fixed delay or button press

Waits for a fixed delay to expire or for the button to be pressed, whichever
comes first.

Arguments:
   ms -- delay time in milliseconds

Returns 0 if the delay time expired with no button press. Returns non-0
if a button press was detected before the end of the delay.
***/
#define SLEEP_PERIOD 25
int button_sleep(const int ms) {
   int i;

   for(i = 0; i < ms; i+= SLEEP_PERIOD) {
      if(button()) return 1;
      _delay_ms(SLEEP_PERIOD);
   }
   return button();
}
