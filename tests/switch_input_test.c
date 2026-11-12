/* Host test of the Switch controller -> keyboard mapping. Build and run: tests/run.sh */
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "switch_input.h"
#include "switch_rumble.h"
#include "switch_steer.h"
#include "switch_config.h"

static int makes[SDL_NUM_SCANCODES], breaks[SDL_NUM_SCANCODES];
static int failures = 0;

static int keyboard_requests = 0, blips = 0;
void SwitchRumble_event(int kind, int strength){ (void)strength; if(kind == SWITCHRUMBLE_BLIP) blips++; }
void SwitchText_request(void){ keyboard_requests++; }
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

    /* a click of the left stick asks for the on-screen keyboard (only on press) */
    SwitchInput_button(LSTICK, 1); SwitchInput_button(LSTICK, 0);
    CHECK(keyboard_requests == 1);

    /* in a race Plus pauses (P), Minus leaves (Escape), B does nothing, R3 switches the view */
    SwitchInput_setRace(1);
    SwitchInput_button(B, 1); CHECK(makes[SDL_SCANCODE_ESCAPE] == 1);   /* still the count from above */
    SwitchInput_button(B, 0);
    SwitchInput_button(PLUS, 1); CHECK(makes[SDL_SCANCODE_P] == 1);
    SwitchInput_button(PLUS, 0); CHECK(breaks[SDL_SCANCODE_P] == 1);
    {
        const int escapes = makes[SDL_SCANCODE_ESCAPE];
        SwitchInput_button(B, 1); SwitchInput_button(B, 0);
        CHECK(makes[SDL_SCANCODE_ESCAPE] == escapes);                   /* B does not leave the race */
        SwitchInput_button(MINUS, 1); CHECK(makes[SDL_SCANCODE_ESCAPE] == escapes+1);
        SwitchInput_button(MINUS, 0);
    }
    SwitchInput_button(RSTICK, 1); CHECK(makes[SDL_SCANCODE_TAB] == 1);
    SwitchInput_button(RSTICK, 0);
    /* in a race with fine steering the stick does not press Left and Right: SwitchSteer takes its
       position; the D-pad still does, and the stick only accelerates or brakes when pushed clearly */
    SwitchConfig_defaults();
    SwitchConfig.stick_mode = 0;
    SwitchConfig.stick_invert_x = 0;                    /* (the default is on: the first console test steered the wrong way) */
    {
        const int lefts = makes[SDL_SCANCODE_LEFT], ups = makes[SDL_SCANCODE_UP], downs = makes[SDL_SCANCODE_DOWN];

        SwitchInput_axis(0, -30000);                    /* stick left */
        CHECK(makes[SDL_SCANCODE_LEFT] == lefts);
        CHECK(SwitchSteer_value() < -0.5f);
        SwitchInput_axis(0, 0);
        SwitchInput_button(DLEFT, 1); SwitchInput_button(DLEFT, 0);
        CHECK(makes[SDL_SCANCODE_LEFT] == lefts+1);     /* the D-pad is digital */

        SwitchInput_axis(1, -20000); SwitchInput_axis(0, 20000);       /* up and right, diagonal */
        CHECK(makes[SDL_SCANCODE_UP] == ups);           /* does not accelerate by accident */
        SwitchInput_axis(0, 0);
        SwitchInput_axis(1, -30000);                    /* clearly up */
        CHECK(makes[SDL_SCANCODE_UP] == ups+1);
        SwitchInput_axis(1, 30000);                     /* clearly down */
        CHECK(makes[SDL_SCANCODE_DOWN] == downs+1);
        SwitchInput_axis(1, 0);

        SwitchConfig.analog_steering = 0;               /* fine steering off: the stick is digital again */
        SwitchInput_axis(0, -30000);
        CHECK(makes[SDL_SCANCODE_LEFT] == lefts+2);
        SwitchInput_axis(0, 0);
        SwitchConfig.analog_steering = 1;

        SwitchConfig.stick_invert_x = 1;                /* swapped: pushed left steers right */
        SwitchInput_axis(0, -30000);
        CHECK(SwitchSteer_value() > 0.5f);
        SwitchInput_axis(0, 30000);
        CHECK(SwitchSteer_value() < -0.5f);
        SwitchInput_axis(0, -32768);                    /* the extreme value does not overflow */
        CHECK(SwitchSteer_value() > 0.5f);
        SwitchInput_axis(0, 0);
        SwitchConfig.stick_invert_x = 0;
    }

    /* direction steering: the stick presses no arrows at all in a race (the D-pad still does) */
    {
        const int lefts = makes[SDL_SCANCODE_LEFT], ups = makes[SDL_SCANCODE_UP], downs = makes[SDL_SCANCODE_DOWN];

        SwitchConfig.stick_mode = 1;
        SwitchInput_axis(0, -30000); SwitchInput_axis(0, 0);
        SwitchInput_axis(1, -30000); SwitchInput_axis(1, 30000); SwitchInput_axis(1, 0);
        CHECK(makes[SDL_SCANCODE_LEFT] == lefts && makes[SDL_SCANCODE_UP] == ups && makes[SDL_SCANCODE_DOWN] == downs);
        SwitchInput_button(DLEFT, 1); SwitchInput_button(DLEFT, 0);
        CHECK(makes[SDL_SCANCODE_LEFT] == lefts+1);
        SwitchConfig.stick_mode = 0;
    }

    /* the left stick click in a race switches gyro steering (with a short vibration each time) */
    {
        const int b = blips;

        SwitchInput_button(LSTICK, 1); SwitchInput_button(LSTICK, 0);
        CHECK(SwitchSteer_gyroEnabled() == 1);
        CHECK(blips == b+2);
        SwitchInput_button(LSTICK, 1); SwitchInput_button(LSTICK, 0);
        CHECK(SwitchSteer_gyroEnabled() == 0);
        CHECK(blips == b+3);
    }
    CHECK(keyboard_requests == 1);                      /* not counted as a keyboard request in a race */

    SwitchInput_setRace(0);
    SwitchInput_button(PLUS, 1); SwitchInput_button(PLUS, 0);
    CHECK(makes[SDL_SCANCODE_P] == 1);                                  /* no pause key on the screens */

    /* numbers outside the table are ignored */
    SwitchInput_button(99, 1); SwitchInput_button(-1, 1);

    if(failures) printf("%d check(s) failed\n", failures);
    else printf("switch_input: all checks passed\n");
    return failures != 0;
}
