#ifndef SWITCH_INPUT_H
#define SWITCH_INPUT_H

#include <SDL.h>

/* Nintendo Switch controller -> keyboard mapping (see switch_input.c). */

void dRally_Keyboard_make(SDL_Scancode);
void dRally_Keyboard_break(SDL_Scancode);

void SwitchInput_button(int button, int down);
void SwitchInput_hat(int mask);
void SwitchInput_axis(int axis, int value);

#endif // SWITCH_INPUT_H
