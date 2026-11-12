/* Host test of the touch gestures. Build and run: tests/run.sh */
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "../include/switch_touch.h"
#include "../include/switch_keys.h"

/* the keys the gestures produce, recorded instead of sent to the game */
static SDL_Scancode keys[64];
static int key_count = 0;
void SwitchKeys_press(SDL_Scancode sc){ keys[key_count++] = sc; }
int SwitchKeys_busy(void){ return 0; }
void SwitchKeys_clear(void){}
void SwitchKeys_pump(void){}

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

static unsigned now = 1000;
static void touch(int n, int x0, int y0, int x1, int y1){

    SwitchTouchFinger f[2] = { {1, x0, y0}, {2, x1, y1} };

    SwitchTouch_feed(now, f, n);
    SwitchTouch_pump(now);
}
static void release(void){ SwitchTouch_feed(now, NULL, 0); SwitchTouch_pump(now); }
static void wait(unsigned ms){ now += ms; SwitchTouch_pump(now); }
static void reset(void){

    release();
    wait(500);
    key_count = 0;
    SwitchTouch_setMode(SWITCHTOUCH_SCREENS);
}

int main(void){

    int x, y, i;

    /* window to game coordinates: 1280x720 window, 640x480 picture in the middle (scale 1.5) */
    SwitchTouch_mapStretched(0.5f, 0.5f, 640, 480, &x, &y);           /* widescreen: no bars, the whole window */
    CHECK(x == 320 && y == 240);
    SwitchTouch_mapStretched(0.0f, 0.999f, 320, 240, &x, &y);
    CHECK(x == 0 && y == 239);
    SwitchTouch_mapToGame(0.5f, 0.5f, 1280, 720, 640, 480, &x, &y);
    CHECK(x == 320 && y == 240);
    SwitchTouch_mapToGame(160.0f/1280, 0.0f, 1280, 720, 640, 480, &x, &y);
    CHECK(x == 0 && y == 0);
    SwitchTouch_mapToGame(1.0f-160.0f/1280, 1.0f, 1280, 720, 640, 480, &x, &y);
    CHECK(x == 640 && y == 480);
    SwitchTouch_mapToGame(0.0f, 0.5f, 1280, 720, 640, 480, &x, &y);
    CHECK(x < 0);                           /* in the black bar */

    /* a tap a screen takes gives no key */
    touch(1, 100, 100, 0, 0); wait(80);
    release();
    CHECK(SwitchTouch_takeTap(&x, &y) && x == 100 && y == 100);
    CHECK(!SwitchTouch_takeTap(&x, &y));
    wait(200);
    CHECK(key_count == 0);
    reset();

    /* a tap nobody takes becomes Enter after a short wait */
    touch(1, 50, 60, 0, 0); wait(80);
    release();
    wait(30); CHECK(key_count == 0);
    wait(60); CHECK(key_count == 1 && keys[0] == SDL_SCANCODE_RETURN);
    wait(300); CHECK(key_count == 1);       /* only once */
    reset();

    /* a long press is not a tap */
    touch(1, 50, 60, 0, 0); wait(600);
    release(); wait(300);
    CHECK(key_count == 0);
    reset();

    /* a swipe down 100 px is two Down keys and no Enter at the end */
    for(i = 0; i <= 10; i++){ touch(1, 100, 100+i*10, 0, 0); wait(16); }
    release(); wait(300);
    CHECK(key_count == 2 && keys[0] == SDL_SCANCODE_DOWN && keys[1] == SDL_SCANCODE_DOWN);
    reset();

    /* sideways and up */
    for(i = 0; i <= 8; i++){ touch(1, 300-i*10, 100, 0, 0); wait(16); }
    release(); wait(300);
    CHECK(key_count == 2 && keys[0] == SDL_SCANCODE_LEFT);
    reset();
    for(i = 0; i <= 5; i++){ touch(1, 300, 300-i*10, 0, 0); wait(16); }
    release(); wait(300);
    CHECK(key_count == 1 && keys[0] == SDL_SCANCODE_UP);
    reset();

    /* a screen that claims the contact (a slider) gets no keys, and finger position while down */
    touch(1, 300, 260, 0, 0); wait(16);
    CHECK(SwitchTouch_isDown(&x, &y) && x == 300 && y == 260);
    SwitchTouch_claim();
    for(i = 0; i <= 10; i++){ touch(1, 300+i*10, 260, 0, 0); wait(16); }
    release(); wait(300);
    CHECK(key_count == 0);
    CHECK(!SwitchTouch_takeTap(&x, &y));
    reset();

    /* two fingers tapping is Escape */
    touch(2, 200, 200, 260, 220); wait(80);
    release(); wait(100);
    CHECK(key_count == 1 && keys[0] == SDL_SCANCODE_ESCAPE);
    CHECK(!SwitchTouch_takeTap(&x, &y));
    reset();

    /* in a race only the two-finger tap works, and it pauses */
    SwitchTouch_setMode(SWITCHTOUCH_RACE);
    touch(1, 100, 100, 0, 0); wait(80); release(); wait(200);
    for(i = 0; i <= 10; i++){ touch(1, 100+i*10, 100, 0, 0); wait(16); }
    release(); wait(300);
    CHECK(key_count == 0);
    touch(2, 200, 200, 260, 220); wait(80); release(); wait(100);
    CHECK(key_count == 1 && keys[0] == SDL_SCANCODE_P);     /* pause: Escape would end the race */
    reset();

    /* touches are ignored for a while after the system keyboard */
    SwitchTouch_ignoreFor(now, 500);
    touch(1, 100, 100, 0, 0); wait(80); release(); wait(200);
    CHECK(key_count == 0);
    wait(500);
    touch(1, 100, 100, 0, 0); wait(80); release(); wait(200);
    CHECK(key_count == 1);

    if(failures) printf("%d check(s) failed\n", failures);
    else printf("switch_touch: all checks passed\n");
    return failures != 0;
}
