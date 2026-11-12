#ifndef SWITCH_INPUT_H
#define SWITCH_INPUT_H

#include <SDL.h>

/* Nintendo Switch controller -> keyboard mapping (see switch_input.c). */

void dRally_Keyboard_make(SDL_Scancode);
void dRally_Keyboard_break(SDL_Scancode);

/* In a race Escape ends the race at once and P pauses it, so the buttons change: + pauses,
   - leaves the race and B does nothing; the right stick click switches the view (Tab). */
void SwitchInput_setRace(int race);

void SwitchInput_button(int button, int down);
void SwitchInput_hat(int mask);
void SwitchInput_axis(int axis, int value);

#endif // SWITCH_INPUT_H
