/* Host test of the on-screen keyboard text entry. Build and run: tests/run.sh */
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "switch.h"
#include "../include/switch_text.h"
#include "../include/switch_keys.h"
#include "../include/switch_touch.h"

const char * stub_keyboard_text = NULL;
int stub_keyboard_shown = 0;
static Uint32 now_ms = 0;
static int last_key = 0;            /* the game's pending key, as dRally_Keyboard keeps it */
static SDL_Scancode typed[256];     /* what the game received, in order */
static int typed_len = 0, held = 0;

Uint32 SDL_GetTicks(void){ return now_ms; }
static int touch_ignored = 0;
void SwitchTouch_ignoreFor(unsigned now, unsigned ms){ (void)now; (void)ms; touch_ignored++; }
Result swkbdCreate(SwkbdConfig * c, int n){ (void)c; (void)n; return 0; }
void swkbdConfigMakePresetDefault(SwkbdConfig * c){ (void)c; }
void swkbdConfigSetHeaderText(SwkbdConfig * c, const char * s){ (void)c; (void)s; }
void swkbdConfigSetGuideText(SwkbdConfig * c, const char * s){ (void)c; (void)s; }
void swkbdConfigSetStringLenMax(SwkbdConfig * c, int n){ c->max = n; }
void swkbdConfigSetStringLenMin(SwkbdConfig * c, int n){ (void)c; (void)n; }
Result swkbdShow(SwkbdConfig * c, char * out, unsigned long size){

    (void)c;
    stub_keyboard_shown++;
    if(!stub_keyboard_text) return 1;
    strncpy(out, stub_keyboard_text, size-1);
    return 0;
}
void swkbdClose(SwkbdConfig * c){ (void)c; }

void dRally_Keyboard_make(SDL_Scancode sc){ last_key = sc; held = sc; typed[typed_len++] = sc; }
void dRally_Keyboard_break(SDL_Scancode sc){ (void)sc; held = 0; last_key = 0; }
int dRally_Keyboard_lastKey(void){ return last_key; }

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

/* Runs the event loop for a while; the "game" reads (clears) the pending key once per round. */
static void run(int rounds){

    int i;

    for(i = 0; i < rounds; i++){

        SwitchText_pump();
        SwitchKeys_pump();
        last_key = 0;       /* the game polls its key */
        now_ms += 100;
    }
}

int main(void){

    int i;

    /* nothing happens while no entry screen runs */
    stub_keyboard_text = "AB";
    run(20);
    CHECK(stub_keyboard_shown == 0);

    /* the keyboard opens by itself once the screen has had time to draw */
    SwitchText_begin(5);
    run(2);
    CHECK(stub_keyboard_shown == 0);
    stub_keyboard_text = "Ab 9-x";     /* 6 characters, the screen takes 5 */
    run(80);
    CHECK(stub_keyboard_shown == 1);
    /* 5 backspaces, then A B space 9 - (the sixth character is cut) */
    CHECK(typed_len == 10);
    for(i = 0; i < 5; i++) CHECK(typed[i] == SDL_SCANCODE_BACKSPACE);
    CHECK(typed[5] == SDL_SCANCODE_A);
    CHECK(typed[6] == (SDL_Scancode)(SDL_SCANCODE_A+1));
    CHECK(typed[7] == SDL_SCANCODE_SPACE);
    CHECK(typed[8] == SDL_SCANCODE_9);
    CHECK(typed[9] == SDL_SCANCODE_MINUS);
    CHECK(held == 0);

    CHECK(touch_ignored == 2);      /* the touches the system keyboard took are ignored */

    /* cancelled keyboard: nothing typed */
    typed_len = 0;
    stub_keyboard_text = NULL;
    SwitchText_request();
    run(20);
    CHECK(stub_keyboard_shown == 2);
    CHECK(typed_len == 0);

    /* characters the screen has no key for are skipped */
    stub_keyboard_text = "a!b";
    SwitchText_request();
    run(80);
    CHECK(typed_len == 5+2);        /* 5 backspaces, A, B */

    /* when the screen ends nothing is held and requests are ignored */
    SwitchText_end();
    SwitchText_request();
    run(20);
    CHECK(stub_keyboard_shown == 3);

    if(failures) printf("%d check(s) failed\n", failures);
    else printf("switch_text: all checks passed\n");
    return failures != 0;
}
