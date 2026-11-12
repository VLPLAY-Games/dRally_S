#include "switch_touch.h"
#include "switch_keys.h"

#include <stdlib.h>
#include <string.h>

#define TAP_MAX_MS      400     /* a touch longer than this is not a tap */
#define TAP_SLOP        12      /* ... and neither is one that moved farther than this (game pixels) */
#define TAP_GRACE_MS    60      /* a tap a screen did not take after this becomes Enter */
#define SWIPE_STEP      40      /* game pixels of movement per arrow key */
#define MAX_FINGERS     4

static struct {
    int         mode;
    unsigned    ignore_until;
    int         count;                      /* fingers now */
    SwitchTouchFinger finger[MAX_FINGERS];
    int         active;                     /* a contact is going on */
    int         peak;                       /* most fingers during it */
    unsigned    down_ms;
    int         start_x, start_y;           /* where the first finger went down */
    int         last_x, last_y;             /* the first finger now */
    int         max_dist;
    int         claimed;                    /* a screen took this contact */
    int         swiped;                     /* it became a swipe */
    int         swipe_x, swipe_y;           /* movement not yet turned into keys */
    int         tap_pending;
    int         tap_x, tap_y;
    unsigned    tap_ms;
    int         two_tap_pending;
} T = {0};

void SwitchTouch_mapToGame(float nx, float ny, int out_w, int out_h, int lw, int lh, int * gx, int * gy){

    float scale, view_w, view_h, ox, oy;

    if((out_w <= 0) || (out_h <= 0) || (lw <= 0) || (lh <= 0)){

        *gx = *gy = 0;
        return;
    }

    scale = (float)out_w/(float)lw;
    if((float)out_h/(float)lh < scale) scale = (float)out_h/(float)lh;
    view_w = lw*scale;
    view_h = lh*scale;
    ox = ((float)out_w-view_w)/2.0f;
    oy = ((float)out_h-view_h)/2.0f;

    *gx = (int)(((nx*out_w)-ox)/scale);
    *gy = (int)(((ny*out_h)-oy)/scale);
}

void SwitchTouch_setMode(int mode){

    T.mode = mode;
}

void SwitchTouch_ignoreFor(unsigned now_ms, unsigned ms){

    T.ignore_until = now_ms+ms;
    T.active = 0;
    T.tap_pending = T.two_tap_pending = 0;
}

void SwitchTouch_claim(void){

    T.claimed = 1;
}

int SwitchTouch_takeTap(int * x, int * y){

    if(!T.tap_pending) return 0;
    T.tap_pending = 0;
    *x = T.tap_x;
    *y = T.tap_y;
    return 1;
}

int SwitchTouch_isDown(int * x, int * y){

    if(!T.active || (T.count != 1)) return 0;
    *x = T.last_x;
    *y = T.last_y;
    return 1;
}

static void end_contact(unsigned now_ms){

    const unsigned duration = now_ms-T.down_ms;

    if(!T.claimed && !T.swiped && (duration <= TAP_MAX_MS)){

        if((T.peak == 1) && (T.max_dist <= TAP_SLOP)){

            T.tap_pending = 1;
            T.tap_x = T.start_x;
            T.tap_y = T.start_y;
            T.tap_ms = now_ms;
        }
        else if((T.peak == 2) && (T.max_dist <= 2*TAP_SLOP)){

            T.two_tap_pending = 1;
        }
    }
    T.active = 0;
}

void SwitchTouch_feed(unsigned now_ms, const SwitchTouchFinger * fingers, int count){

    if(now_ms < T.ignore_until){

        T.count = 0;
        return;
    }

    if(count > MAX_FINGERS) count = MAX_FINGERS;
    T.count = count;
    if(count > 0) memcpy(T.finger, fingers, sizeof(SwitchTouchFinger)*count);

    if(count > 0){

        const int x = fingers[0].x, y = fingers[0].y;

        if(!T.active){

            T.active = 1;
            T.peak = count;
            T.down_ms = now_ms;
            T.start_x = x;
            T.start_y = y;
            T.max_dist = 0;
            T.claimed = T.swiped = 0;
            T.swipe_x = T.swipe_y = 0;
        }
        else {

            const int dist = abs(x-T.start_x)+abs(y-T.start_y);

            if(count > T.peak) T.peak = count;
            if(dist > T.max_dist) T.max_dist = dist;

            /* a single finger moving: the movement turns into arrow keys in SwitchTouch_pump */
            if((count == 1) && (T.peak == 1)){

                T.swipe_x += x-T.last_x;
                T.swipe_y += y-T.last_y;
            }
        }
        T.last_x = x;
        T.last_y = y;
    }
    else if(T.active){

        end_contact(now_ms);
    }
}

void SwitchTouch_pump(unsigned now_ms){

    if(now_ms < T.ignore_until) return;

    if(T.two_tap_pending){

        /* back on the screens; in a race Escape would end it, so the tap pauses instead */
        T.two_tap_pending = 0;
        SwitchKeys_press((T.mode == SWITCHTOUCH_RACE) ? SDL_SCANCODE_P : SDL_SCANCODE_ESCAPE);
    }

    if(T.mode == SWITCHTOUCH_RACE){

        /* touching the screen never steers or confirms in a race */
        T.tap_pending = 0;
        T.swipe_x = T.swipe_y = 0;
        return;
    }

    /* a tap no screen took is Enter */
    if(T.tap_pending && (now_ms-T.tap_ms >= TAP_GRACE_MS)){

        T.tap_pending = 0;
        SwitchKeys_press(SDL_SCANCODE_RETURN);
    }

    /* a swipe is arrow keys, one per step of movement along the longer direction */
    if(T.active && !T.claimed && (T.count == 1) && (T.peak == 1)){

        while(1){

            if((abs(T.swipe_x) >= SWIPE_STEP) && (abs(T.swipe_x) >= abs(T.swipe_y))){

                SwitchKeys_press((T.swipe_x > 0) ? SDL_SCANCODE_RIGHT : SDL_SCANCODE_LEFT);
                T.swipe_x += (T.swipe_x > 0) ? -SWIPE_STEP : SWIPE_STEP;
                T.swipe_y = 0;
                T.swiped = 1;
            }
            else if(abs(T.swipe_y) >= SWIPE_STEP){

                SwitchKeys_press((T.swipe_y > 0) ? SDL_SCANCODE_DOWN : SDL_SCANCODE_UP);
                T.swipe_y += (T.swipe_y > 0) ? -SWIPE_STEP : SWIPE_STEP;
                T.swipe_x = 0;
                T.swiped = 1;
            }
            else break;
        }
    }
}
