#ifndef SWITCH_STEER_H
#define SWITCH_STEER_H

/* Fine steering. The game steers with two on/off flags per frame (left, right), so a stick or a
   tilt is turned into how often the flag is on: half way is on every second frame, full is on
   every frame. Digital steering (the D-pad) is left as it is.

   Two sources are added: the left stick (with a dead zone) and, when it is switched on, the tilt
   of the console like a steering wheel (handheld mode). */

/* The left stick's horizontal position, -1 (left) to 1 (right). */
void SwitchSteer_setStick(float x);

/* The accelerometer in G, in the console's frame (X to the right, Y toward the bottom edge,
   gravity's direction). Works out the tilt and keeps it. */
void SwitchSteer_feedAccel(float ax, float ay, float az);
float SwitchSteer_tilt(void);                   /* degrees, clockwise positive, smoothed */

void SwitchSteer_setGyroEnabled(int on);
int  SwitchSteer_gyroEnabled(void);
int  SwitchSteer_toggleGyro(void);              /* returns the new state; the tilt now is straight */
void SwitchSteer_recenter(void);                /* the tilt now is straight */

/* The steering wanted now, -1 to 1 (left to right), and the flags for this frame. */
float SwitchSteer_value(void);
void SwitchSteer_tick(int * left, int * right);
void SwitchSteer_reset(void);                   /* the pulse state (a new race) */

/* The tilt in degrees for an accelerometer reading; returns 0 when the console lies too flat to
   tell (the tilt cannot be worked out). */
int SwitchSteer_rollFromAccel(float ax, float ay, float az, float * degrees);

#endif // SWITCH_STEER_H
