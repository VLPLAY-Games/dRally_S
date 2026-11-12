/* Just enough of SDL.h to build the Switch input and text code on a PC for the tests. The scan
   code values are SDL2's own, the letters and digits are contiguous as in SDL. */
#ifndef STUB_SDL_H
#define STUB_SDL_H

typedef unsigned int Uint32;

typedef enum {
    SDL_SCANCODE_UNKNOWN = 0,
    SDL_SCANCODE_A = 4, SDL_SCANCODE_Z = 29,
    SDL_SCANCODE_1 = 30, SDL_SCANCODE_9 = 38, SDL_SCANCODE_0 = 39,
    SDL_SCANCODE_RETURN = 40, SDL_SCANCODE_ESCAPE = 41, SDL_SCANCODE_BACKSPACE = 42,
    SDL_SCANCODE_SPACE = 44, SDL_SCANCODE_MINUS = 45, SDL_SCANCODE_EQUALS = 46,
    SDL_SCANCODE_COMMA = 54, SDL_SCANCODE_PERIOD = 55, SDL_SCANCODE_SLASH = 56,
    SDL_SCANCODE_F1 = 58, SDL_SCANCODE_RIGHT = 79, SDL_SCANCODE_LEFT = 80,
    SDL_SCANCODE_DOWN = 81, SDL_SCANCODE_UP = 82,
    SDL_SCANCODE_LCTRL = 224, SDL_SCANCODE_LSHIFT = 225, SDL_SCANCODE_LALT = 226,
    SDL_NUM_SCANCODES = 512
} SDL_Scancode;

#define SDL_HAT_UP    0x01
#define SDL_HAT_RIGHT 0x02
#define SDL_HAT_DOWN  0x04
#define SDL_HAT_LEFT  0x08

Uint32 SDL_GetTicks(void);

#endif
