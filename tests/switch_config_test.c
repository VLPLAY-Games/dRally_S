/* Host test of the Switch settings file. Build and run: tests/run.sh */
#include <stdio.h>
#include <string.h>
#include "../include/switch_config.h"

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

int main(void){

    SwitchConfig_defaults();
    CHECK(SwitchConfig.vsync == 1 && SwitchConfig.stick_mode == 1 && SwitchConfig.stick_deadzone == 15);

    /* the template written to a new file is itself valid and keeps the defaults */
    SwitchConfig_parse(SwitchConfig_template());
    CHECK(SwitchConfig.vsync == 1 && SwitchConfig.rumble == 1 && SwitchConfig.rumble_percent == 100);
    CHECK(SwitchConfig.analog_steering == 1 && SwitchConfig.widescreen == 0 && SwitchConfig.scaling == 0);

    /* values, comments, spaces; unknown lines and bad values are ignored; ranges are limited */
    SwitchConfig_parse("  vsync=0   # off\n stick_mode = 0\nstick_deadzone = 80\nscaling = 7\nrumble_percent = -5\n"
                       "nonsense line\nrumble = abc\nanalog_steering = 0\n\n\n");
    CHECK(SwitchConfig.vsync == 0);
    CHECK(SwitchConfig.stick_mode == 0);
    CHECK(SwitchConfig.stick_deadzone == 50);
    CHECK(SwitchConfig.scaling == 1);               /* 7 limited to 1 */
    CHECK(SwitchConfig.rumble_percent == 0);        /* -5 limited to 0 */
    CHECK(SwitchConfig.rumble == 1);                /* "abc" ignored */
    CHECK(SwitchConfig.analog_steering == 0);

    /* a file that is missing is created from the template */
    {
        const char * path = "/tmp/switch_config_test.cfg";
        FILE * f;
        char text[4096] = {0};

        remove(path);
        SwitchConfig_load(path);
        CHECK(SwitchConfig.vsync == 1);             /* defaults again */
        f = fopen(path, "rb");
        CHECK(f != NULL);
        if(f){ fread(text, 1, sizeof(text)-1, f); fclose(f); }
        CHECK(strstr(text, "vsync = 1") != NULL && strstr(text, "stick_invert_x = 0") != NULL);
        remove(path);
    }

    /* saving writes the template with the current values; loading it again gives the same settings */
    {
        const char * path = "/tmp/switch_config_save_test.cfg";
        FILE * f;
        char text[4096] = {0};

        SwitchConfig_defaults();
        SwitchConfig.scaling = 1;
        SwitchConfig.widescreen = 1;
        SwitchConfig.stick_deadzone = 40;
        SwitchConfig_save(path);

        f = fopen(path, "rb");
        CHECK(f != NULL);
        if(f){ fread(text, 1, sizeof(text)-1, f); fclose(f); }
        CHECK(strstr(text, "scaling = 1") != NULL && strstr(text, "widescreen = 1") != NULL);
        CHECK(strstr(text, "# Picture scaling") != NULL);               /* the explanations stay */

        SwitchConfig_defaults();
        SwitchConfig_load(path);
        CHECK(SwitchConfig.scaling == 1 && SwitchConfig.widescreen == 1 && SwitchConfig.stick_deadzone == 40);
        CHECK(SwitchConfig.vsync == 1 && SwitchConfig.stick_invert_x == 0);
        remove(path);
    }

    if(failures) printf("%d check(s) failed\n", failures);
    else printf("switch_config: all checks passed\n");
    return failures != 0;
}
