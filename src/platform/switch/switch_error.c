#include "switch_error.h"

#include <SDL.h>
#include <switch.h>
#include <stdio.h>

/* The text screen of libnx draws into the console's own framebuffer, so it is used before the
   game's window exists (or after it is gone). It is written through stderr: stdout goes to the
   log file. */

void SwitchError_show(const char * title, const char * text){

    PadState pad;

    consoleInit(NULL);
    fprintf(stderr, "\x1b[2J\x1b[1;1H");
    fprintf(stderr, "dRally for Nintendo Switch\n");
    fprintf(stderr, "==========================\n\n");
    fprintf(stderr, "%s\n\n%s\n\nPress + or A to exit.\n", title, text);
    consoleUpdate(NULL);

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    while(appletMainLoop()){

        padUpdate(&pad);
        if(padGetButtonsDown(&pad)&(HidNpadButton_Plus|HidNpadButton_A|HidNpadButton_B)) break;
        consoleUpdate(NULL);
    }

    consoleExit(NULL);
}

void SwitchError_fatal(const char * text){

    printf("dRally error: %s\n", text);
    fflush(stdout);
    SDL_Quit();
    SwitchError_show("The game stopped because of a problem:", text);
}
