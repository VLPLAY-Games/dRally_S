#include "switch_text.h"
#include "switch_keys.h"
#include "switch_touch.h"

#include <SDL.h>
#include <switch.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define OPEN_DELAY_MS   500     /* let the entry screen draw before the keyboard covers it */

static struct {
    int             active;
    int             max_chars;
    int             want_open;
    Uint32          begin_ms;
} T = {0};

void SwitchText_begin(int max_chars){

    memset(&T, 0, sizeof(T));
    T.active = 1;
    T.max_chars = (max_chars > 0) ? max_chars : 12;
    T.want_open = 1;
    T.begin_ms = SDL_GetTicks();
}

void SwitchText_end(void){

    SwitchKeys_clear();
    memset(&T, 0, sizeof(T));
}

void SwitchText_request(void){

    if(T.active) T.want_open = 1;
}

/* The scan code that types a character on the entry screen, 0 when the screen has no such key.
   The screen accepts capitals, digits and - = , . / and the space. */
static SDL_Scancode scancode_for(int c){

    c = toupper(c);
    if((c >= 'A') && (c <= 'Z')) return (SDL_Scancode)(SDL_SCANCODE_A+(c-'A'));
    if((c >= '1') && (c <= '9')) return (SDL_Scancode)(SDL_SCANCODE_1+(c-'1'));
    switch(c){
    case '0': return SDL_SCANCODE_0;
    case ' ': return SDL_SCANCODE_SPACE;
    case '-': return SDL_SCANCODE_MINUS;
    case '=': return SDL_SCANCODE_EQUALS;
    case ',': return SDL_SCANCODE_COMMA;
    case '.': return SDL_SCANCODE_PERIOD;
    case '/': return SDL_SCANCODE_SLASH;
    }
    return SDL_SCANCODE_UNKNOWN;
}

static void show_keyboard(void){

    SwkbdConfig kbd;
    char text[64] = {0};
    int n;

    T.want_open = 0;

    /* the touches that went to the keyboard must not reach the game when it closes */
    SwitchTouch_ignoreFor(SDL_GetTicks(), 0);
    if(R_FAILED(swkbdCreate(&kbd, 0))) return;
    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetHeaderText(&kbd, "Enter a name");
    swkbdConfigSetGuideText(&kbd, "Letters, digits, space and - = , . /");
    swkbdConfigSetStringLenMax(&kbd, T.max_chars);
    swkbdConfigSetStringLenMin(&kbd, 0);
    if(R_FAILED(swkbdShow(&kbd, text, sizeof(text))) || !text[0]){

        swkbdClose(&kbd);
        SwitchTouch_ignoreFor(SDL_GetTicks(), 400);
        return;
    }
    swkbdClose(&kbd);
    SwitchTouch_ignoreFor(SDL_GetTicks(), 400);

    /* clear what the screen holds, then type the new text */
    for(n = 0; n < T.max_chars; n++) SwitchKeys_press(SDL_SCANCODE_BACKSPACE);
    for(n = 0; (text[n] != 0) && (n < T.max_chars); n++){

        const SDL_Scancode sc = scancode_for((unsigned char)text[n]);

        if(sc != SDL_SCANCODE_UNKNOWN) SwitchKeys_press(sc);
    }
}

void SwitchText_pump(void){

    if(!T.active) return;

    /* wait until the typed keys have all gone to the game */
    if(SwitchKeys_busy()) return;

    if(T.want_open && (SDL_GetTicks()-T.begin_ms >= OPEN_DELAY_MS)) show_keyboard();
}
