#include "switch_config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SwitchConfig_t SwitchConfig;

void SwitchConfig_defaults(void){

    SwitchConfig.vsync = 1;
    SwitchConfig.scaling = 0;
    SwitchConfig.widescreen = 0;
    SwitchConfig.rumble = 1;
    SwitchConfig.rumble_percent = 100;
    SwitchConfig.analog_steering = 1;
    SwitchConfig.stick_mode = 1;
    SwitchConfig.stick_deadzone = 15;
    SwitchConfig.stick_invert_x = 0;
}

static int clamp(int v, int lo, int hi){ return (v < lo) ? lo : ((v > hi) ? hi : v); }

static void set_value(const char * name, int value){

    if(!strcmp(name, "vsync")) SwitchConfig.vsync = !!value;
    else if(!strcmp(name, "widescreen")) SwitchConfig.widescreen = !!value;
    else if(!strcmp(name, "scaling")) SwitchConfig.scaling = clamp(value, 0, 1);
    else if(!strcmp(name, "rumble")) SwitchConfig.rumble = !!value;
    else if(!strcmp(name, "rumble_percent")) SwitchConfig.rumble_percent = clamp(value, 0, 100);
    else if(!strcmp(name, "analog_steering")) SwitchConfig.analog_steering = !!value;
    else if(!strcmp(name, "stick_mode")) SwitchConfig.stick_mode = clamp(value, 0, 1);
    else if(!strcmp(name, "stick_deadzone")) SwitchConfig.stick_deadzone = clamp(value, 0, 50);
    else if(!strcmp(name, "stick_invert_x")) SwitchConfig.stick_invert_x = !!value;
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
        "# Picture scaling. 0: smooth (the picture is blurred a little when enlarged). 1: smart: the\n"
        "# picture is first enlarged by a whole number (sharp pixels), then fitted to the screen\n"
        "# (720p handheld, 1080p docked) without blurring the pixel edges.\n"
        "scaling = 0\n"
        "\n"
        "# 1: stretch the picture over the whole 16:9 screen (the game was made for 4:3, so everything\n"
        "# gets wider). 0: the original 4:3 picture with black bars at the sides.\n"
        "widescreen = 0\n"
        "\n"
        "# Vibration in races (hits, mines). rumble_percent is how strong, 0 to 100.\n"
        "rumble = 1\n"
        "rumble_percent = 100\n"
        "\n"
        "# 1: the left stick steers in fine steps. 0: steering by the stick is on or off, like the\n"
        "# D-pad. stick_deadzone is the part of the stick's travel that does nothing (0 to 50 percent).\n"
        "analog_steering = 1\n"
        "\n"
        "# What the left stick does in a race. 1: it points where the car should go, as seen on the\n"
        "# screen (stick up: the car turns to head up the screen, stick right: to the right...);\n"
        "# the car turns by the shortest way. 0: the stick turns the car left and right, like a wheel.\n"
        "# Gas and brake stay on ZR and ZL in both. Needs analog_steering = 1.\n"
        "stick_mode = 1\n"
        "stick_deadzone = 15\n"
        "\n"
        "# 1: swap left and right of the left stick (use it if the stick steers the wrong way).\n"
        "stick_invert_x = 0\n";

}

static int get_value(const char * name, int * value){

    if(!strcmp(name, "vsync")) *value = SwitchConfig.vsync;
    else if(!strcmp(name, "scaling")) *value = SwitchConfig.scaling;
    else if(!strcmp(name, "widescreen")) *value = SwitchConfig.widescreen;
    else if(!strcmp(name, "rumble")) *value = SwitchConfig.rumble;
    else if(!strcmp(name, "rumble_percent")) *value = SwitchConfig.rumble_percent;
    else if(!strcmp(name, "analog_steering")) *value = SwitchConfig.analog_steering;
    else if(!strcmp(name, "stick_mode")) *value = SwitchConfig.stick_mode;
    else if(!strcmp(name, "stick_deadzone")) *value = SwitchConfig.stick_deadzone;
    else if(!strcmp(name, "stick_invert_x")) *value = SwitchConfig.stick_invert_x;
    else return 0;
    return 1;
}

void SwitchConfig_save(const char * path){

    FILE * f;
    const char * p = SwitchConfig_template();

    if(!(f = fopen(path, "wb"))) return;

    /* the template, with each "name = value" line carrying the current value */
    while(*p){

        char line[160], name[64];
        size_t n = 0;
        int value;

        while(p[n] && (p[n] != '\n') && (n < sizeof(line)-1)){ line[n] = p[n]; n++; }
        line[n] = 0;
        p += n;
        if(*p == '\n') p++;

        if((line[0] != '#') && (sscanf(line, "%63s = %d", name, &value) == 2) && get_value(name, &value)) fprintf(f, "%s = %d\n", name, value);
        else fprintf(f, "%s\n", line);
    }
    fclose(f);
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
