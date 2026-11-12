#include "switch_input.h"
#include "switch_text.h"
#include "switch_config.h"
#include "switch_steer.h"
#include "switch_rumble.h"

/* The controller drives the game through the keyboard it was written for. Death Rally steers
   with the arrows and uses a few keys in a race (defaults of the original): A accelerate, Z brake,
   Left Ctrl machine gun, Left Alt drop mine, Left Shift turbo, Space horn. Several controller
   inputs can map to one key, so the key states are recomputed from the raw input state and a
   make/break is sent only when a key changes. */

#include <stdlib.h>

#define SW_MAX_BUTTONS 32
#define SW_AXIS_THRESHOLD 12000

/* Button order of the SDL2 Switch joystick driver (the libnx HID button bits). */
enum {
    SW_A, SW_B, SW_X, SW_Y, SW_LSTICK, SW_RSTICK, SW_L, SW_R, SW_ZL, SW_ZR, SW_PLUS, SW_MINUS,
    SW_DLEFT, SW_DUP, SW_DRIGHT, SW_DDOWN, SW_STICK_LEFT, SW_STICK_UP, SW_STICK_RIGHT, SW_STICK_DOWN
};

static struct {
    int             race;
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

/* In a race with fine steering on, the stick does not press Left and Right (SwitchSteer turns its
   position into steering), and it only accelerates or brakes when it is pushed clearly up or down:
   steering diagonally must not brake. The D-pad is always on or off. */
static int fine_steering(void){

    return SW.race && SwitchConfig.analog_steering;
}

static void update_keys(void){

    int left, right, up, down;
    int stick_left, stick_right, stick_up, stick_down;

    stick_left  = SW.button[SW_STICK_LEFT]  || SW.axis_x < -SW_AXIS_THRESHOLD;
    stick_right = SW.button[SW_STICK_RIGHT] || SW.axis_x >  SW_AXIS_THRESHOLD;
    stick_up    = SW.button[SW_STICK_UP]    || SW.axis_y < -SW_AXIS_THRESHOLD;
    stick_down  = SW.button[SW_STICK_DOWN]  || SW.axis_y >  SW_AXIS_THRESHOLD;

    if(fine_steering()){

        const int ay = abs(SW.axis_y), ax = abs(SW.axis_x);
        const int vertical = (ay > 2*SW_AXIS_THRESHOLD) && (ay > 2*ax);     /* clearly up or down */

        stick_left = stick_right = 0;
        stick_up = vertical && (SW.axis_y < 0);
        stick_down = vertical && (SW.axis_y > 0);
    }

    left  = SW.button[SW_DLEFT]  || stick_left  || (SW.hat & SDL_HAT_LEFT);
    right = SW.button[SW_DRIGHT] || stick_right || (SW.hat & SDL_HAT_RIGHT);
    up    = SW.button[SW_DUP]    || stick_up    || (SW.hat & SDL_HAT_UP);
    down  = SW.button[SW_DDOWN]  || stick_down  || (SW.hat & SDL_HAT_DOWN);

    set_key(SDL_SCANCODE_LEFT,   left);
    set_key(SDL_SCANCODE_RIGHT,  right);
    set_key(SDL_SCANCODE_UP,     up);
    set_key(SDL_SCANCODE_DOWN,   down);

    set_key(SDL_SCANCODE_RETURN, SW.button[SW_A]);                       /* confirm */
    if(SW.race){

        /* Escape ends the race, P pauses it: Plus pauses, Minus leaves, B is left alone */
        set_key(SDL_SCANCODE_P,      SW.button[SW_PLUS]);
        set_key(SDL_SCANCODE_ESCAPE, SW.button[SW_MINUS]);
        set_key(SDL_SCANCODE_TAB,    SW.button[SW_RSTICK]);              /* view */
        set_key(SDL_SCANCODE_F1,     0);
    }
    else {

        set_key(SDL_SCANCODE_P,      0);
        set_key(SDL_SCANCODE_TAB,    0);
        set_key(SDL_SCANCODE_ESCAPE, SW.button[SW_B] || SW.button[SW_PLUS]); /* back */
        set_key(SDL_SCANCODE_F1,     SW.button[SW_MINUS]);                   /* help */
    }
    set_key(SDL_SCANCODE_LSHIFT, SW.button[SW_X]);                       /* turbo */
    set_key(SDL_SCANCODE_SPACE,  SW.button[SW_Y]);                       /* horn */
    set_key(SDL_SCANCODE_LALT,   SW.button[SW_L]);                       /* drop mine */
    set_key(SDL_SCANCODE_LCTRL,  SW.button[SW_R]);                       /* machine gun */
    set_key(SDL_SCANCODE_Z,      SW.button[SW_ZL]);                      /* brake */
    set_key(SDL_SCANCODE_A,      SW.button[SW_ZR]);                      /* accelerate */
}

void SwitchInput_setRace(int race){

    SW.race = !!race;
    update_keys();
}

void SwitchInput_button(int button, int down){

    if((button >= 0) && (button < SW_MAX_BUTTONS)){

        SW.button[button] = !!down;
        update_keys();

        /* a click of the left stick: in a race it switches gyro steering on or off (and sets the
           straight position), on a name entry screen it opens the on-screen keyboard again */
        if((button == SW_LSTICK) && down){

            if(SW.race){

                SwitchRumble_event(SWITCHRUMBLE_BLIP, 0);
                if(SwitchSteer_toggleGyro()) SwitchRumble_event(SWITCHRUMBLE_BLIP, 0);
            }
            else SwitchText_request();
        }
    }
}

void SwitchInput_hat(int mask){

    SW.hat = mask;
    update_keys();
}

void SwitchInput_axis(int axis, int value){

    if(axis == 0){

        SW.axis_x = value;
        SwitchSteer_setStick(value/32767.0f);
    }
    else if(axis == 1) SW.axis_y = value;
    update_keys();
}
