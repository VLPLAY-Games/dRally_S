#ifndef SWITCH_KEYS_H
#define SWITCH_KEYS_H

#include <SDL.h>

/* Key presses the console itself produces (the on-screen keyboard, touch gestures).

   The game reads one key at a time and expects a key to be held until it has read it. A pressed
   key is made, held until the game has read it, then released; the next one follows. */

void SwitchKeys_press(SDL_Scancode sc);     /* queue a key press */
int  SwitchKeys_busy(void);                 /* a key is held or waiting */
void SwitchKeys_clear(void);                /* drop what is waiting, release the held key */
void SwitchKeys_pump(void);                 /* called from the event loop */

#endif // SWITCH_KEYS_H
