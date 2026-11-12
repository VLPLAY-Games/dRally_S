/* Host test of the Switch controller -> keyboard mapping. Build and run: tests/run.sh */
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "../switch_input.h"

static int makes[SDL_NUM_SCANCODES], breaks[SDL_NUM_SCANCODES];
static int failures = 0;

void dRally_Keyboard_make(SDL_Scancode sc){ makes[sc]++; }
void dRally_Keyboard_break(SDL_Scancode sc){ breaks[sc]++; }

#define CHECK(cond) do { if(!(cond)){ printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

/* libnx button numbers, as the SDL2 Switch joystick driver reports them */
enum { A, B, X, Y, LSTICK, RSTICK, L, R, ZL, ZR, PLUS, MINUS, DLEFT, DUP, DRIGHT, DDOWN, SLEFT, SUP, SRIGHT, SDOWN };

int main(void){

    /* A confirms */
    SwitchInput_button(A, 1); CHECK(makes[SDL_SCANCODE_RETURN] == 1);
    SwitchInput_button(A, 1); CHECK(makes[SDL_SCANCODE_RETURN] == 1);   /* no repeat make */
    SwitchInput_button(A, 0); CHECK(breaks[SDL_SCANCODE_RETURN] == 1);

    /* the D-pad and the stick both press Left: one make, one break after both are released */
    SwitchInput_button(DLEFT, 1); SwitchInput_button(SLEFT, 1);
    CHECK(makes[SDL_SCANCODE_LEFT] == 1);
    SwitchInput_button(DLEFT, 0); CHECK(breaks[SDL_SCANCODE_LEFT] == 0);
    SwitchInput_button(SLEFT, 0); CHECK(breaks[SDL_SCANCODE_LEFT] == 1);

    /* stick axis past the threshold steers, back at the centre releases */
    SwitchInput_axis(0, 20000); CHECK(makes[SDL_SCANCODE_RIGHT] == 1);
    SwitchInput_axis(0, 5000);  CHECK(breaks[SDL_SCANCODE_RIGHT] == 1);
    SwitchInput_axis(1, -30000); CHECK(makes[SDL_SCANCODE_UP] == 1);
    SwitchInput_axis(1, 0);      CHECK(breaks[SDL_SCANCODE_UP] == 1);

    /* a hat does the same */
    SwitchInput_hat(SDL_HAT_DOWN); CHECK(makes[SDL_SCANCODE_DOWN] == 1);
    SwitchInput_hat(0);            CHECK(breaks[SDL_SCANCODE_DOWN] == 1);

    /* race keys */
    SwitchInput_button(ZR, 1); CHECK(makes[SDL_SCANCODE_A] == 1);        /* accelerate */
    SwitchInput_button(ZL, 1); CHECK(makes[SDL_SCANCODE_Z] == 1);        /* brake */
    SwitchInput_button(R, 1);  CHECK(makes[SDL_SCANCODE_LCTRL] == 1);    /* machine gun */
    SwitchInput_button(L, 1);  CHECK(makes[SDL_SCANCODE_LALT] == 1);     /* mine */
    SwitchInput_button(X, 1);  CHECK(makes[SDL_SCANCODE_LSHIFT] == 1);   /* turbo */
    SwitchInput_button(Y, 1);  CHECK(makes[SDL_SCANCODE_SPACE] == 1);    /* horn */

    /* B and Plus both give Escape: it is released only when both are */
    SwitchInput_button(B, 1); SwitchInput_button(PLUS, 1);
    CHECK(makes[SDL_SCANCODE_ESCAPE] == 1);
    SwitchInput_button(B, 0); CHECK(breaks[SDL_SCANCODE_ESCAPE] == 0);
    SwitchInput_button(PLUS, 0); CHECK(breaks[SDL_SCANCODE_ESCAPE] == 1);

    /* numbers outside the table are ignored */
    SwitchInput_button(99, 1); SwitchInput_button(-1, 1);

    if(failures) printf("%d check(s) failed\n", failures);
    else printf("switch_input: all checks passed\n");
    return failures != 0;
}
