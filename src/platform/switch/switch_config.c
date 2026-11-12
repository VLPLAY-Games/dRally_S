#include "switch_config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SwitchConfig_t SwitchConfig;

void SwitchConfig_defaults(void){

    SwitchConfig.vsync = 1;
    SwitchConfig.rumble = 1;
    SwitchConfig.rumble_percent = 100;
    SwitchConfig.analog_steering = 1;
    SwitchConfig.stick_deadzone = 15;
    SwitchConfig.gyro = 0;
    SwitchConfig.gyro_range = 25;
    SwitchConfig.gyro_deadzone = 3;
}

static int clamp(int v, int lo, int hi){ return (v < lo) ? lo : ((v > hi) ? hi : v); }

static void set_value(const char * name, int value){

    if(!strcmp(name, "vsync")) SwitchConfig.vsync = !!value;
    else if(!strcmp(name, "rumble")) SwitchConfig.rumble = !!value;
    else if(!strcmp(name, "rumble_percent")) SwitchConfig.rumble_percent = clamp(value, 0, 100);
    else if(!strcmp(name, "analog_steering")) SwitchConfig.analog_steering = !!value;
    else if(!strcmp(name, "stick_deadzone")) SwitchConfig.stick_deadzone = clamp(value, 0, 50);
    else if(!strcmp(name, "gyro")) SwitchConfig.gyro = !!value;
    else if(!strcmp(name, "gyro_range")) SwitchConfig.gyro_range = clamp(value, 5, 90);
    else if(!strcmp(name, "gyro_deadzone")) SwitchConfig.gyro_deadzone = clamp(value, 0, 20);
}

void SwitchConfig_parse(const char * text){

    char line[160];
    const char * p = text;

    while(*p){

        size_t n = 0;
        char * eq;
        char * name;
        char * value;

        while(p[n] && (p[n] != '\n') && (n < sizeof(line)-1)){ line[n] = p[n]; n++; }
        line[n] = 0;
        p += n;
        while(*p && (*p != '\n')) p++;      /* the rest of a line that was too long */
        if(*p == '\n') p++;

        if((eq = strchr(line, '#'))) *eq = 0;
        if(!(eq = strchr(line, '='))) continue;
        *eq = 0;
        name = line;
        value = eq+1;
        while(isspace((unsigned char)*name)) name++;
        while((eq > name) && isspace((unsigned char)eq[-1])) *--eq = 0;
        while(isspace((unsigned char)*value)) value++;

        if(*name && (isdigit((unsigned char)*value) || (*value == '-'))) set_value(name, atoi(value));
    }
}

const char * SwitchConfig_template(void){

    return
        "# Settings of dRally for the Nintendo Switch. Change a value and start the game again.\n"
        "\n"
        "# 1: the picture waits for the screen (no tearing). 0: off. Turn it off if the game runs\n"
        "# too slowly or stutters.\n"
        "vsync = 1\n"
        "\n"
        "# Vibration in races (hits, mines). rumble_percent is how strong, 0 to 100.\n"
        "rumble = 1\n"
        "rumble_percent = 100\n"
        "\n"
        "# 1: the left stick steers in fine steps. 0: steering by the stick is on or off, like the\n"
        "# D-pad. stick_deadzone is the part of the stick's travel that does nothing (0 to 50 percent).\n"
        "analog_steering = 1\n"
        "stick_deadzone = 15\n"
        "\n"
        "# 1: steer by tilting the console like a wheel (handheld mode only). gyro_range is the tilt\n"
        "# in degrees for full steering, gyro_deadzone the tilt that does nothing. In a race, a click\n"
        "# of the left stick switches it on or off and sets the straight position.\n"
        "gyro = 0\n"
        "gyro_range = 25\n"
        "gyro_deadzone = 3\n";
}

void SwitchConfig_load(const char * path){

    FILE * f;

    SwitchConfig_defaults();

    if((f = fopen(path, "rb"))){

        char text[4096];
        const size_t n = fread(text, 1, sizeof(text)-1, f);

        text[n] = 0;
        fclose(f);
        SwitchConfig_parse(text);
    }
    else if((f = fopen(path, "wb"))){

        fputs(SwitchConfig_template(), f);
        fclose(f);
    }
}
