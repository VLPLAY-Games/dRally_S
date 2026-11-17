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

#define SWITCH_LOCAL_MAX_PLAYERS 4
enum {
    SWITCH_RACE_GAS = 0x01, SWITCH_RACE_BRAKE = 0x02,
    SWITCH_RACE_LEFT = 0x04, SWITCH_RACE_RIGHT = 0x08,
    SWITCH_RACE_TURBO = 0x10, SWITCH_RACE_GUN = 0x20,
    SWITCH_RACE_MINE = 0x40
};

int SwitchLocal_openLobby(void);
int SwitchLocal_joined(void);
int SwitchLocal_begin(void);
int SwitchLocal_lobbyButton(SDL_JoystickID id, int button, int down);
void SwitchLocal_end(void);
int SwitchLocal_count(void);
int SwitchLocal_acceptMenuInput(SDL_JoystickID id);
unsigned char SwitchLocal_controls(int player, float direction);
int SwitchLocal_advanceEffects(void);
void SwitchLocal_present(const unsigned char *pixels, int width, int height);

#endif // SWITCH_INPUT_H
