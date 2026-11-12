#include "switch_text.h"

#include <SDL.h>
#include <switch.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

void dRally_Keyboard_make(SDL_Scancode);
void dRally_Keyboard_break(SDL_Scancode);
int dRally_Keyboard_lastKey(void);

#define OPEN_DELAY_MS   500     /* let the entry screen draw before the keyboard covers it */
#define QUEUE_SIZE      64

static struct {
    int             active;
    int             max_chars;
    int             want_open;
    Uint32          begin_ms;
    SDL_Scancode    queue[QUEUE_SIZE];
    int             queue_len, queue_pos;
    SDL_Scancode    pressed;    /* the key made and not yet released, 0 for none */
} T = {0};

void SwitchText_begin(int max_chars){

    memset(&T, 0, sizeof(T));
    T.active = 1;
    T.max_chars = (max_chars > 0) ? max_chars : 12;
    T.want_open = 1;
    T.begin_ms = SDL_GetTicks();
}

void SwitchText_end(void){

    if(T.pressed) dRally_Keyboard_break(T.pressed);
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

static void queue_key(SDL_Scancode sc){

    if(T.queue_len < QUEUE_SIZE) T.queue[T.queue_len++] = sc;
}

static void show_keyboard(void){

    SwkbdConfig kbd;
    char text[64] = {0};
    int n;

    T.want_open = 0;

    if(R_FAILED(swkbdCreate(&kbd, 0))) return;
    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetHeaderText(&kbd, "Enter a name");
    swkbdConfigSetGuideText(&kbd, "Letters, digits, space and - = , . /");
    swkbdConfigSetStringLenMax(&kbd, T.max_chars);
    swkbdConfigSetStringLenMin(&kbd, 0);
    if(R_FAILED(swkbdShow(&kbd, text, sizeof(text))) || !text[0]){

        swkbdClose(&kbd);
        return;
    }
    swkbdClose(&kbd);

    /* clear what the screen holds, then type the new text */
    for(n = 0; n < T.max_chars; n++) queue_key(SDL_SCANCODE_BACKSPACE);
    for(n = 0; (text[n] != 0) && (n < T.max_chars); n++){

        const SDL_Scancode sc = scancode_for((unsigned char)text[n]);

        if(sc != SDL_SCANCODE_UNKNOWN) queue_key(sc);
    }
}

void SwitchText_pump(void){

    if(!T.active) return;

    /* a key is held until the game has read it, then released */
    if(T.pressed){

        if(dRally_Keyboard_lastKey() == 0){

            dRally_Keyboard_break(T.pressed);
            T.pressed = SDL_SCANCODE_UNKNOWN;
        }
        return;
    }

    if(T.queue_pos < T.queue_len){

        if(dRally_Keyboard_lastKey() == 0){

            T.pressed = T.queue[T.queue_pos++];
            dRally_Keyboard_make(T.pressed);
        }
        return;
    }

    T.queue_len = T.queue_pos = 0;

    if(T.want_open && (SDL_GetTicks()-T.begin_ms >= OPEN_DELAY_MS)) show_keyboard();
}
