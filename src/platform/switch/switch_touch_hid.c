#include "switch_touch.h"

#include <SDL.h>
#include <switch.h>

/* The console's touch screen is read straight from the HID service (SDL's own touch events are
   not used, so the touches cannot be counted twice). The panel reports 1280x720 whatever the
   screen shows; it only exists in handheld mode. */

#define PANEL_W 1280.0f
#define PANEL_H 720.0f
#define MAX_TOUCHES 4

void dRally_Display_mapTouch(float nx, float ny, int * gx, int * gy);

void SwitchTouch_poll(void){

    static int started = 0;
    HidTouchScreenState state;
    SwitchTouchFinger fingers[MAX_TOUCHES];
    int n, count;

    if(!started){

        hidInitializeTouchScreen();
        started = 1;
    }

    if(hidGetTouchScreenStates(&state, 1) < 1) return;

    count = (state.count < MAX_TOUCHES) ? state.count : MAX_TOUCHES;
    for(n = 0; n < count; n++){

        fingers[n].id = (int)state.touches[n].finger_id;
        dRally_Display_mapTouch(state.touches[n].x/PANEL_W, state.touches[n].y/PANEL_H, &fingers[n].x, &fingers[n].y);
    }

    SwitchTouch_feed(SDL_GetTicks(), fingers, count);
    SwitchTouch_pump(SDL_GetTicks());
}
