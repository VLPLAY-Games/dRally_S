/* Host test of the fine steering and the rumble shapes. Build and run: tests/run.sh */
#include <stdio.h>
#include <math.h>
#include "../include/switch_config.h"
#include "../include/switch_steer.h"
#include "../include/switch_rumble.h"

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

static void count(int ticks, int * l, int * r){

    int i, a, b;

    *l = *r = 0;
    for(i = 0; i < ticks; i++){ SwitchSteer_tick(&a, &b); *l += a; *r += b; }
}

int main(void){

    int l, r;
    float roll;
    unsigned low, high, ms;

    SwitchConfig_defaults();
    SwitchConfig.stick_mode = 0;                    /* the stick turns left and right (see the heading tests below) */
    SwitchSteer_reset();

    /* nothing pressed: no steering */
    count(100, &l, &r); CHECK(l == 0 && r == 0);

    /* inside the dead zone (15 %): nothing; full stick: every frame */
    SwitchSteer_setStick(0.10f); count(100, &l, &r); CHECK(l == 0 && r == 0);
    SwitchSteer_setStick(1.0f);  count(100, &l, &r); CHECK(r == 100 && l == 0);
    SwitchSteer_setStick(-1.0f); count(100, &l, &r); CHECK(l == 100 && r == 0);

    /* in between: a share of the frames, more for more stick, and never both flags at once */
    SwitchSteer_setStick(0.5f);  count(1000, &l, &r);
    CHECK(r > 150 && r < 600 && l == 0);
    {
        int half = r;
        SwitchSteer_setStick(0.8f); count(1000, &l, &r);
        CHECK(r > half && r < 1000);
    }

    /* the pulses are spread out: no run of more than 3 steering frames at half stick */
    {
        int i, a, b, run = 0, longest = 0;

        SwitchSteer_setStick(0.6f);
        for(i = 0; i < 500; i++){

            SwitchSteer_tick(&a, &b);
            run = (a || b) ? run+1 : 0;
            if(run > longest) longest = run;
        }
        CHECK(longest <= 3);
    }

    /* analog steering off: the stick does nothing here (it is the D-pad's job) */
    SwitchConfig.analog_steering = 0;
    SwitchSteer_setStick(1.0f); count(50, &l, &r); CHECK(l == 0 && r == 0);
    SwitchConfig.analog_steering = 1;
    SwitchSteer_setStick(0.0f);

    /* the tilt of the console: upright is 0 degrees, right is clockwise */
    CHECK(SwitchSteer_rollFromAccel(0.0f, 1.0f, 0.0f, &roll) && fabsf(roll) < 0.01f);
    CHECK(SwitchSteer_rollFromAccel(0.5f, 0.866f, 0.0f, &roll) && fabsf(roll-30.0f) < 0.1f);
    CHECK(SwitchSteer_rollFromAccel(-0.5f, 0.866f, 0.0f, &roll) && fabsf(roll+30.0f) < 0.1f);
    CHECK(!SwitchSteer_rollFromAccel(0.0f, 0.1f, 1.0f, &roll));     /* lying flat: cannot tell */

    /* gyro steering is off until it is switched on */
    SwitchSteer_feedAccel(0.0f, 1.0f, 0.0f);
    CHECK(SwitchSteer_value() == 0.0f);
    CHECK(SwitchSteer_toggleGyro() == 1);       /* the first reading after this is the straight position */
    SwitchSteer_feedAccel(sinf(7.0f/57.2958f), cosf(7.0f/57.2958f), 0.0f);
    CHECK(SwitchSteer_value() == 0.0f);         /* 7 degrees right when it was switched on: straight */

    /* 10 degrees to the right of straight: part of the way (dead zone 3, range 25) */
    {
        int i;
        for(i = 0; i < 40; i++) SwitchSteer_feedAccel(sinf(17.0f/57.2958f), cosf(17.0f/57.2958f), 0.0f);
        CHECK(SwitchSteer_value() > 0.25f && SwitchSteer_value() < 0.45f);       /* 10 degrees from straight */
        for(i = 0; i < 40; i++) SwitchSteer_feedAccel(sinf(47.0f/57.2958f), cosf(47.0f/57.2958f), 0.0f);
        CHECK(SwitchSteer_value() == 1.0f);                         /* past the range: full */
        for(i = 0; i < 40; i++) SwitchSteer_feedAccel(sinf(-33.0f/57.2958f), cosf(-33.0f/57.2958f), 0.0f);
        CHECK(SwitchSteer_value() == -1.0f);
        for(i = 0; i < 40; i++) SwitchSteer_feedAccel(sinf(9.0f/57.2958f), cosf(9.0f/57.2958f), 0.0f);
        CHECK(SwitchSteer_value() == 0.0f);                         /* inside the dead zone */

        /* straight can be set again, and the stick adds to the tilt */
        for(i = 0; i < 40; i++) SwitchSteer_feedAccel(sinf(27.0f/57.2958f), cosf(27.0f/57.2958f), 0.0f);
        SwitchSteer_recenter();
        CHECK(SwitchSteer_value() == 0.0f);
        SwitchSteer_setStick(0.5f);
        CHECK(SwitchSteer_value() > 0.1f);
        SwitchSteer_setStick(0.0f);
    }
    CHECK(SwitchSteer_toggleGyro() == 0);                           /* off again */
    CHECK(SwitchSteer_value() == 0.0f);

    /* vibration: harder hit, stronger and longer; nothing for a scrape; the percentage scales it; off is off */
    CHECK(SwitchRumble_shape(SWITCHRUMBLE_IMPACT, 0x10000, 100, &low, &high, &ms) && low == 65535 && ms == 200);
    {
        unsigned low2, high2, ms2;

        CHECK(SwitchRumble_shape(SWITCHRUMBLE_IMPACT, 0x4000, 100, &low2, &high2, &ms2) && low2 < low && ms2 < ms);
        CHECK(!SwitchRumble_shape(SWITCHRUMBLE_IMPACT, 100, 100, &low2, &high2, &ms2));
        CHECK(SwitchRumble_shape(SWITCHRUMBLE_IMPACT, 0x10000, 50, &low2, &high2, &ms2) && low2 > 30000 && low2 < 35000);
        CHECK(!SwitchRumble_shape(SWITCHRUMBLE_IMPACT, 0x10000, 0, &low2, &high2, &ms2));
        CHECK(SwitchRumble_shape(SWITCHRUMBLE_MINE_DROP, 0, 100, &low2, &high2, &ms2) && low2 == 0 && ms2 == 40);
        CHECK(SwitchRumble_shape(SWITCHRUMBLE_MINE_BLAST, 0x9000, 100, &low2, &high2, &ms2) && ms2 > 150);
        CHECK(!SwitchRumble_shape(SWITCHRUMBLE_NONE, 0x10000, 100, &low2, &high2, &ms2));
    }

    /* direction steering: the stick points where the car should go on the screen */
    {
        int dl, dr, handled;

        SwitchConfig_defaults();
        SwitchConfig.stick_mode = 1;
        SwitchSteer_reset();
        SwitchSteer_setGyroEnabled(0);

        /* the game's Direction for the screen directions: 0 is up, it grows toward the left */
        CHECK(fabsf(SwitchSteer_directionFor(0.0f, -1.0f)) < 0.01f);                       /* up */
        CHECK(fabsf(SwitchSteer_directionFor(-1.0f, 0.0f)-90.0f) < 0.01f);                 /* left */
        CHECK(fabsf(fabsf(SwitchSteer_directionFor(0.0f, 1.0f))-180.0f) < 0.01f);          /* down */
        CHECK(fabsf(SwitchSteer_directionFor(1.0f, 0.0f)+90.0f) < 0.01f);                  /* right */

        /* the stick in its dead zone asks for nothing */
        SwitchSteer_setStick(0.1f); SwitchSteer_setStickY(-0.1f);
        handled = SwitchSteer_tickHeading(0.0f, &dl, &dr);
        CHECK(!handled && !dl && !dr);

        /* car heading up, stick right: turn right on every frame (the error is large) */
        SwitchSteer_setStick(1.0f); SwitchSteer_setStickY(0.0f);
        dl = dr = 0;
        for(l = 0; l < 20; l++){ int a, b; SwitchSteer_tickHeading(0.0f, &a, &b); dl += a; dr += b; }
        CHECK(dr == 20 && dl == 0);

        /* car heading up, stick left: turn left */
        SwitchSteer_setStick(-1.0f);
        dl = dr = 0;
        for(l = 0; l < 20; l++){ int a, b; SwitchSteer_tickHeading(0.0f, &a, &b); dl += a; dr += b; }
        CHECK(dl == 20 && dr == 0);

        /* car heading left (90), stick up: turn right, the short way */
        SwitchSteer_setStick(0.0f); SwitchSteer_setStickY(-1.0f);
        dl = dr = 0;
        for(l = 0; l < 20; l++){ int a, b; SwitchSteer_tickHeading(90.0f, &a, &b); dl += a; dr += b; }
        CHECK(dr == 20 && dl == 0);

        /* the shortest way across the 0/360 line: car at 350, stick up: turn right; car at 10: turn left */
        dl = dr = 0;
        for(l = 0; l < 20; l++){ int a, b; SwitchSteer_tickHeading(330.0f, &a, &b); dl += a; dr += b; }
        CHECK(dl == 20 && dr == 0);                     /* 330 is 30 degrees to the left of up: a larger Direction is a left turn */
        dl = dr = 0;
        for(l = 0; l < 20; l++){ int a, b; SwitchSteer_tickHeading(30.0f, &a, &b); dl += a; dr += b; }
        CHECK(dr == 20 && dl == 0);

        /* heading as asked (within the dead band): no steering, but it is handled */
        handled = SwitchSteer_tickHeading(1.0f, &dl, &dr);
        CHECK(handled && !dl && !dr);

        /* a small error steers only some of the frames */
        dl = dr = 0;
        for(l = 0; l < 40; l++){ int a, b; SwitchSteer_tickHeading(10.0f, &a, &b); dl += a; dr += b; }
        CHECK(dl == 0 && dr > 5 && dr < 35);

        /* in this mode a stick pushed sideways does not turn by itself (SwitchSteer_value) */
        SwitchSteer_setStick(1.0f); SwitchSteer_setStickY(0.0f);
        CHECK(SwitchSteer_value() == 0.0f);

        /* with analog steering off nothing is handled here */
        SwitchConfig.analog_steering = 0;
        CHECK(!SwitchSteer_tickHeading(0.0f, &dl, &dr));
    }

    if(failures) printf("%d check(s) failed\n", failures);
    else printf("switch_steer: all checks passed\n");
    return failures != 0;
}
