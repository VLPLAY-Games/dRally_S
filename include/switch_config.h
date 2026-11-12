#ifndef SWITCH_CONFIG_H
#define SWITCH_CONFIG_H

/* Settings of the Switch port, read from SWITCH.CFG next to the game (written with its defaults
   and explanations the first time the game runs). One "name = value" per line; # starts a comment. */

typedef struct {
    int vsync;              /* 1: the picture waits for the screen (no tearing); 0: off */
    int rumble;             /* 1: vibration in races */
    int rumble_percent;     /* 0..100: how strong */
    int analog_steering;    /* 1: the left stick steers in fine steps (pulse width); 0: on or off */
    int stick_deadzone;     /* 0..50 percent of the stick's travel that does nothing */
    int gyro;               /* 1: steer by tilting the console (handheld mode) */
    int gyro_range;         /* degrees of tilt for full steering */
    int gyro_deadzone;      /* degrees of tilt that do nothing */
} SwitchConfig_t;

extern SwitchConfig_t SwitchConfig;

void SwitchConfig_defaults(void);
void SwitchConfig_parse(const char * text);                 /* applies the lines it understands */
void SwitchConfig_load(const char * path);                  /* defaults, then the file; writes the file if it is missing */
const char * SwitchConfig_template(void);

#endif // SWITCH_CONFIG_H
