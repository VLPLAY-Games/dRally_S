#ifndef SWITCH_CONFIG_H
#define SWITCH_CONFIG_H

/* Settings of the Switch port, read from SWITCH.CFG next to the game (written with its defaults
   and explanations the first time the game runs). One "name = value" per line; # starts a comment. */

typedef struct {
    char ipx_server[64];    /* network race: host name or address of the IPX tunnelling server (ipxnet); empty: no network */
    int ipx_port;           /* its UDP port (8888 by default) */
    int vsync;              /* 1: the picture waits for the screen (no tearing); 0: off */
    int widescreen;         /* 1: the picture is stretched over the whole 16:9 screen; 0: 4:3 with black bars */
    int scaling;            /* 0: smooth (bilinear); 1: smart (whole-number enlargement, then smoothed to the screen) */
    int rumble;             /* 1: vibration in races */
    int rumble_percent;     /* 0..100: how strong */
    int analog_steering;    /* 1: the left stick steers in fine steps (pulse width); 0: on or off */
    int stick_mode;         /* 0: the stick turns the car left and right; 1: the stick points where the car should go (on the screen) */
    int stick_deadzone;     /* 0..50 percent of the stick's travel that does nothing */
    int stick_invert_x;     /* 1: the left stick's left and right are swapped */
} SwitchConfig_t;

extern SwitchConfig_t SwitchConfig;

void SwitchConfig_defaults(void);
void SwitchConfig_parse(const char * text);                 /* applies the lines it understands */
void SwitchConfig_save(const char * path);                  /* the template with the current values */
void SwitchConfig_load(const char * path);                  /* defaults, then the file; writes the file if it is missing */
const char * SwitchConfig_template(void);

#endif // SWITCH_CONFIG_H
