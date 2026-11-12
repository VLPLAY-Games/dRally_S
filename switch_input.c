#include "switch_input.h"

/* The controller drives the game through the keyboard it was written for. Death Rally steers
   with the arrows and uses a few keys in a race (defaults of the original): A accelerate, Z brake,
   Left Ctrl machine gun, Left Alt drop mine, Left Shift turbo, Space horn. Several controller
   inputs can map to one key, so the key states are recomputed from the raw input state and a
   make/break is sent only when a key changes. */

#define SW_MAX_BUTTONS 32
#define SW_AXIS_THRESHOLD 12000

/* Button order of the SDL2 Switch joystick driver (the libnx HID button bits). */
enum {
    SW_A, SW_B, SW_X, SW_Y, SW_LSTICK, SW_RSTICK, SW_L, SW_R, SW_ZL, SW_ZR, SW_PLUS, SW_MINUS,
    SW_DLEFT, SW_DUP, SW_DRIGHT, SW_DDOWN, SW_STICK_LEFT, SW_STICK_UP, SW_STICK_RIGHT, SW_STICK_DOWN
};

static struct {
    unsigned char   button[SW_MAX_BUTTONS];
    int             axis_x, axis_y;
    int             hat;
    unsigned char   key[SDL_NUM_SCANCODES];
} SW = {0};

static void set_key(SDL_Scancode sc, int down){

    down = !!down;
    if(SW.key[sc] != down){

        SW.key[sc] = down;
        if(down) dRally_Keyboard_make(sc);
        else dRally_Keyboard_break(sc);
    }
}

static void update_keys(void){

    int left, right, up, down;

    left  = SW.button[SW_DLEFT]  || SW.button[SW_STICK_LEFT]  || (SW.hat & SDL_HAT_LEFT)  || SW.axis_x < -SW_AXIS_THRESHOLD;
    right = SW.button[SW_DRIGHT] || SW.button[SW_STICK_RIGHT] || (SW.hat & SDL_HAT_RIGHT) || SW.axis_x >  SW_AXIS_THRESHOLD;
    up    = SW.button[SW_DUP]    || SW.button[SW_STICK_UP]    || (SW.hat & SDL_HAT_UP)    || SW.axis_y < -SW_AXIS_THRESHOLD;
    down  = SW.button[SW_DDOWN]  || SW.button[SW_STICK_DOWN]  || (SW.hat & SDL_HAT_DOWN)  || SW.axis_y >  SW_AXIS_THRESHOLD;

    set_key(SDL_SCANCODE_LEFT,   left);
    set_key(SDL_SCANCODE_RIGHT,  right);
    set_key(SDL_SCANCODE_UP,     up);
    set_key(SDL_SCANCODE_DOWN,   down);

    set_key(SDL_SCANCODE_RETURN, SW.button[SW_A]);                       /* confirm */
    set_key(SDL_SCANCODE_ESCAPE, SW.button[SW_B] || SW.button[SW_PLUS]); /* back / pause */
    set_key(SDL_SCANCODE_LSHIFT, SW.button[SW_X]);                       /* turbo */
    set_key(SDL_SCANCODE_SPACE,  SW.button[SW_Y]);                       /* horn */
    set_key(SDL_SCANCODE_LALT,   SW.button[SW_L]);                       /* drop mine */
    set_key(SDL_SCANCODE_LCTRL,  SW.button[SW_R]);                       /* machine gun */
    set_key(SDL_SCANCODE_Z,      SW.button[SW_ZL]);                      /* brake */
    set_key(SDL_SCANCODE_A,      SW.button[SW_ZR]);                      /* accelerate */
    set_key(SDL_SCANCODE_F1,     SW.button[SW_MINUS]);                   /* help */
}

void SwitchInput_button(int button, int down){

    if((button >= 0) && (button < SW_MAX_BUTTONS)){

        SW.button[button] = !!down;
        update_keys();
    }
}

void SwitchInput_hat(int mask){

    SW.hat = mask;
    update_keys();
}

void SwitchInput_axis(int axis, int value){

    if(axis == 0) SW.axis_x = value;
    else if(axis == 1) SW.axis_y = value;
    update_keys();
}
