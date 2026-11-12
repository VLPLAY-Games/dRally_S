#ifndef SWITCH_TOUCH_H
#define SWITCH_TOUCH_H

/* Touch screen input (handheld mode).

   Positions are in the game's screen coordinates (640x480 in the menus). Two layers use the
   touch screen:

   - Screens that know their layout (the menus, the volume sliders, the yes/no question) ask for
     taps and drags and handle them themselves.
   - Everything else gets gestures that become keys the game already understands: a tap is
     Enter, a swipe is the arrow keys, a tap with two fingers is Escape. A contact a screen has
     taken (SwitchTouch_takeTap, SwitchTouch_claim) is left out of the gestures. In a race only
     the two-finger tap works, and it pauses (P), so touching the screen never steers. */

typedef struct {
    int id;
    int x, y;
} SwitchTouchFinger;

enum { SWITCHTOUCH_SCREENS, SWITCHTOUCH_RACE };

/* Where a touch on the window lands on the game's screen. nx, ny are 0..1 over the window; out_w,
   out_h the renderer's output size and lw, lh the game's logical size, which is centred in it. */
void SwitchTouch_mapToGame(float nx, float ny, int out_w, int out_h, int lw, int lh, int * gx, int * gy);

/* Feed the current fingers and the time (ms); then pump the gestures. */
void SwitchTouch_feed(unsigned now_ms, const SwitchTouchFinger * fingers, int count);
void SwitchTouch_pump(unsigned now_ms);

/* A finished tap, once: returns 1 and its position. */
int SwitchTouch_takeTap(int * x, int * y);
/* One finger is down: returns 1 and where. */
int SwitchTouch_isDown(int * x, int * y);
/* The current contact is handled by a screen: no gesture keys come from it. */
void SwitchTouch_claim(void);

void SwitchTouch_setMode(int mode);
/* Ignore the touch screen for a while (after the system keyboard closed). */
void SwitchTouch_ignoreFor(unsigned now_ms, unsigned ms);

/* Reads the console's touch screen and runs the above (Switch only). */
void SwitchTouch_poll(void);

#endif // SWITCH_TOUCH_H
