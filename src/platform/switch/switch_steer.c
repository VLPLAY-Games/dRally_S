#include "switch_steer.h"
#include "switch_config.h"

#include <math.h>

#define RAD2DEG 57.2957795f
#define MIN_STEER 0.05f         /* less than this is no steering */

static struct {
    float   stick;
    float   stick_y;
    SwitchSteerState pulse;
} S = {0};

static float clampf(float v, float lo, float hi){ return (v < lo) ? lo : ((v > hi) ? hi : v); }

void SwitchSteer_setStick(float x){

    S.stick = clampf(x, -1.0f, 1.0f);
}

void SwitchSteer_setStickY(float y){

    S.stick_y = clampf(y, -1.0f, 1.0f);
}

void SwitchSteer_reset(void){

    S.pulse = (SwitchSteerState){0};
}

/* Stick: the dead zone is cut off and the rest stretched to the full range, with a slightly
   gentler start so small movements are small. */
static float stick_value(float x){

    const float dz = SwitchConfig.stick_deadzone/100.0f;
    float a = fabsf(x), v;

    if(!SwitchConfig.analog_steering || SwitchConfig.stick_mode || (a <= dz)) return 0.0f;

    v = (a-dz)/(1.0f-dz);
    v = v*(0.35f+0.65f*v);          /* between linear and squared */
    return (x < 0.0f) ? -v : v;
}

float SwitchSteer_value(void){

    return clampf(stick_value(S.stick), -1.0f, 1.0f);
}

/* v from -1 (left) to 1 (right): the flags for this frame, on as often as v says */
static void pulse(SwitchSteerState *state, float v, int * left, int * right){

    const float a = fabsf(v);
    const int dir = (v < 0.0f) ? -1 : 1;

    *left = *right = 0;

    if(a < MIN_STEER){

        *state = (SwitchSteerState){0};
        return;
    }

    if(dir != state->last_dir){

        /* a new direction starts with a pulse, so the car answers at once */
        state->acc = 1.0f;
        state->last_dir = dir;
    }

    state->acc += a;
    if(state->acc >= 1.0f){

        state->acc -= 1.0f;
        if(dir < 0) *left = 1;
        else *right = 1;
    }
}

void SwitchSteer_tick(int * left, int * right){

    pulse(&S.pulse, SwitchSteer_value(), left, right);
}

float SwitchSteer_directionFor(float x, float y){

    /* the car moves by (-sin D, -cos D * 5/6) on the screen */
    return atan2f(-x, -y/0.833333f)*RAD2DEG;
}

#define HEADING_DEADBAND 3.0f       /* degrees of error that are left alone */
#define HEADING_FULL 25.0f          /* the error at which the steering is on every frame */

static int tick_heading(SwitchSteerState *state, float x, float y, float car_direction,
                        int *left, int *right){

    float dz = SwitchConfig.stick_deadzone/100.0f;
    const float mag = sqrtf(x*x+y*y);
    float err, a;

    if(dz < 0.25f) dz = 0.25f;      /* the direction of a stick near the centre is not reliable */

    if(!SwitchConfig.analog_steering || !SwitchConfig.stick_mode || (mag <= dz)){

        *left = *right = 0;
        if(SwitchConfig.stick_mode) *state = (SwitchSteerState){0};
        return 0;
    }

    err = SwitchSteer_directionFor(x, y)-car_direction;
    while(err > 180.0f) err -= 360.0f;
    while(err <= -180.0f) err += 360.0f;

    a = fabsf(err);
    if(a < HEADING_DEADBAND){

        *left = *right = 0;
        *state = (SwitchSteerState){0};
        return 1;
    }

    a = (a-HEADING_DEADBAND)/(HEADING_FULL-HEADING_DEADBAND);
    if(a > 1.0f) a = 1.0f;
    if(a < 0.3f) a = 0.3f;

    /* a larger Direction is a turn to the left */
    pulse(state, (err > 0.0f) ? -a : a, left, right);
    return 1;
}

int SwitchSteer_tickHeading(float car_direction, int *left, int *right){
    return tick_heading(&S.pulse, S.stick, S.stick_y, car_direction, left, right);
}

void SwitchSteer_tickStick(SwitchSteerState *state, float x, float y, float direction,
                          int *left, int *right){
    x = clampf(x, -1.0f, 1.0f);
    y = clampf(y, -1.0f, 1.0f);
    if(!tick_heading(state, x, y, direction, left, right))
        pulse(state, stick_value(x), left, right);
}
