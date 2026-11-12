#include "switch_keys.h"

void dRally_Keyboard_make(SDL_Scancode);
void dRally_Keyboard_break(SDL_Scancode);
int dRally_Keyboard_lastKey(void);

#define QUEUE_SIZE 64

static SDL_Scancode queue[QUEUE_SIZE];
static int queue_len = 0, queue_pos = 0;
static SDL_Scancode pressed = SDL_SCANCODE_UNKNOWN;     /* made and not yet released */

void SwitchKeys_press(SDL_Scancode sc){

    if(queue_len < QUEUE_SIZE) queue[queue_len++] = sc;
}

int SwitchKeys_busy(void){

    return (pressed != SDL_SCANCODE_UNKNOWN) || (queue_pos < queue_len);
}

void SwitchKeys_clear(void){

    if(pressed != SDL_SCANCODE_UNKNOWN) dRally_Keyboard_break(pressed);
    pressed = SDL_SCANCODE_UNKNOWN;
    queue_len = queue_pos = 0;
}

void SwitchKeys_pump(void){

    /* a key is held until the game has read it, then released */
    if(pressed != SDL_SCANCODE_UNKNOWN){

        if(dRally_Keyboard_lastKey() == 0){

            dRally_Keyboard_break(pressed);
            pressed = SDL_SCANCODE_UNKNOWN;
        }
        return;
    }

    if(queue_pos < queue_len){

        if(dRally_Keyboard_lastKey() == 0){

            pressed = queue[queue_pos++];
            dRally_Keyboard_make(pressed);
        }
        return;
    }

    queue_len = queue_pos = 0;
}
