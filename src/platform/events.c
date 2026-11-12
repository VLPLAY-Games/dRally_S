#include <stdlib.h>
#include "drally.h"

extern void_cb ___2432c8h;

void dRally_Keyboard_make(SDL_Scancode);
void dRally_Keyboard_break(SDL_Scancode);

#if defined(DR_SWITCH)

#include "switch_input.h"
#include "switch_keys.h"
#include "switch_text.h"
#include "switch_touch.h"

static int joysticks_opened = 0;

static void switch_open_joysticks(void){

    int n;

    for(n = 0; n < SDL_NumJoysticks(); n++){

        if(SDL_JoystickOpen(n)) printf("[dRally.INPUT] Opened controller %d: %s\n", n, SDL_JoystickNameForIndex(n));
    }
}

static void switch_handle_event(const SDL_Event * e){

    static int logged = 0, axis_logged = 0;

    switch(e->type){
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        /* the first presses are logged, to check the button numbering on a real console */
        if(logged < 200){

            logged++;
            printf("[dRally.INPUT] joystick button %d %s\n", e->jbutton.button, (e->type == SDL_JOYBUTTONDOWN) ? "down" : "up");
        }
        SwitchInput_button(e->jbutton.button, e->type == SDL_JOYBUTTONDOWN);
        break;
    case SDL_JOYHATMOTION:
        SwitchInput_hat(e->jhat.value);
        break;
    case SDL_JOYAXISMOTION:
        /* the first strong deflections of the left stick are logged: which way is negative */
        if((e->jaxis.axis < 2) && (abs(e->jaxis.value) > 20000) && (axis_logged < 40)){

            axis_logged++;
            printf("[dRally.INPUT] joystick axis %d value %d\n", e->jaxis.axis, e->jaxis.value);
        }
        SwitchInput_axis(e->jaxis.axis, e->jaxis.value);
        break;
    case SDL_JOYDEVICEADDED:
        SDL_JoystickOpen(e->jdevice.which);
        break;
    }
}

#endif // DR_SWITCH

void IO_Loop(void){

    SDL_Event e;

#if defined(DR_SWITCH)
    if(!joysticks_opened){

        joysticks_opened = 1;
        switch_open_joysticks();
    }
    SwitchTouch_poll();
    SwitchText_pump();
    SwitchKeys_pump();
#endif

    while(SDL_PollEvent(&e)){

#if defined(DR_SWITCH)
        switch_handle_event(&e);
#endif

        if(e.type == SDL_KEYDOWN){

            dRally_Keyboard_make(e.key.keysym.scancode);
        }
        else if(e.type == SDL_KEYUP){
           
            dRally_Keyboard_break(e.key.keysym.scancode);
        }
        else if(e.type == SDL_QUIT){
            printf("[dRally] TODO: exit not handled properly\n");
            ___2432c8h();
        }
    }
}
