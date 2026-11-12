#include "switch_steer.h"
#include "switch_config.h"

#include <math.h>

#define RAD2DEG 57.2957795f
#define SMOOTHING 0.25f         /* how much of each new tilt reading is taken (the hand shakes) */
#define MIN_STEER 0.05f         /* less than this is no steering */

static struct {
    float   stick;
    float   tilt;               /* smoothed, degrees */
    int     tilt_valid;
    int     center_pending;     /* the next reading is the straight position */
    float   center;             /* the tilt that is straight */
    int     gyro;
    float   acc;                /* the pulse accumulator */
    int     last_dir;
} S = {0};

static float clampf(float v, float lo, float hi){ return (v < lo) ? lo : ((v > hi) ? hi : v); }

void SwitchSteer_setStick(float x){

    S.stick = clampf(x, -1.0f, 1.0f);
}

int SwitchSteer_rollFromAccel(float ax, float ay, float az, float * degrees){

    (void)az;

    /* upright the reading is (0, 1, 0): gravity toward the bottom edge. Turning the console like a
       wheel to the right (clockwise, seen from the player) moves gravity toward the right edge. */
    if(sqrtf(ax*ax+ay*ay) < 0.35f) return 0;

    *degrees = atan2f(ax, ay)*RAD2DEG;
    return 1;
}

void SwitchSteer_feedAccel(float ax, float ay, float az){

    float roll;

    if(!SwitchSteer_rollFromAccel(ax, ay, az, &roll)) return;

    if(!S.tilt_valid){

        S.tilt = roll;
        S.tilt_valid = 1;
        if(S.center_pending){

            S.center = roll;
            S.center_pending = 0;
        }
    }
    else S.tilt += (roll-S.tilt)*SMOOTHING;
}

float SwitchSteer_tilt(void){ return S.tilt; }

void SwitchSteer_setGyroEnabled(int on){

    S.gyro = !!on;

    /* the sensor is only read while it is on, so the last tilt is old: the first new reading is
       the straight position */
    S.tilt_valid = 0;
    S.center_pending = S.gyro;
}

int SwitchSteer_gyroEnabled(void){ return S.gyro; }

int SwitchSteer_toggleGyro(void){

    SwitchSteer_setGyroEnabled(!S.gyro);
    return S.gyro;
}

void SwitchSteer_recenter(void){

    S.center = S.tilt;
}

void SwitchSteer_reset(void){

    S.acc = 0.0f;
    S.last_dir = 0;
}

/* Stick: the dead zone is cut off and the rest stretched to the full range, with a slightly
   gentler start so small movements are small. */
static float stick_value(void){

    const float dz = SwitchConfig.stick_deadzone/100.0f;
    float a = fabsf(S.stick), v;

    if(!SwitchConfig.analog_steering || (a <= dz)) return 0.0f;

    v = (a-dz)/(1.0f-dz);
    v = v*(0.35f+0.65f*v);          /* between linear and squared */
    return (S.stick < 0.0f) ? -v : v;
}

static float gyro_value(void){

    const float dz = (float)SwitchConfig.gyro_deadzone;
    const float range = (float)SwitchConfig.gyro_range;
    float t, a, v;

    if(!S.gyro || !S.tilt_valid) return 0.0f;

    t = S.tilt-S.center;
    a = fabsf(t);
    if(a <= dz) return 0.0f;

    v = clampf((a-dz)/((range > dz) ? (range-dz) : 1.0f), 0.0f, 1.0f);
    return (t < 0.0f) ? -v : v;
}

float SwitchSteer_value(void){

    return clampf(stick_value()+gyro_value(), -1.0f, 1.0f);
}

void SwitchSteer_tick(int * left, int * right){

    const float v = SwitchSteer_value();
    const float a = fabsf(v);
    const int dir = (v < 0.0f) ? -1 : 1;

    *left = *right = 0;

    if(a < MIN_STEER){

        S.acc = 0.0f;
        S.last_dir = 0;
        return;
    }

    if(dir != S.last_dir){

        /* a new direction starts with a pulse, so the car answers at once */
        S.acc = 1.0f;
        S.last_dir = dir;
    }

    S.acc += a;
    if(S.acc >= 1.0f){

        S.acc -= 1.0f;
        if(dir < 0) *left = 1;
        else *right = 1;
    }
}
