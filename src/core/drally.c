#include "drally.h"
#include "drally_display.h"

#if defined(DR_MULTIPLAYER)
extern __DWORD__ ___19bd60h;
void ___623d4h(void);
#endif

extern void_cb ___2432c8h;

void dRally_Keyboard_init(void);
void ___60466h(int, int);
void ___3e720h(void);
void __VGA3_SETMODE(void);
void dRally_System_init(void);
void dRally_Sound_quit(void);
void dRally_System_clean(void);

static void ___10060h(void){

	printf("\nDeath Rally *** Full Version 1.1\n");
}

static void ___100dch(void){

	__VGA3_SETMODE();
#if defined(DR_SWITCH)
	/* closing the application from the HOME menu: a normal exit */
	printf("dRally: closed by the system\n");
	exit(0);
#else
	printf("DEATH RALLY Exit: CTRL+ALT+DEL pressed!\n");
	exit(0x70);
#endif // DR_SWITCH
}

#if defined(DR_SWITCH)

#include <switch.h>
#include <unistd.h>
#include "datacheck.h"
#include "switch_config.h"
#include "switch_error.h"

/* The game finds its files (ENGINE.BPA, CINEM/, DR.CFG ...) in the current directory. Run from
   the folder the NRO was started from, as the Homebrew Menu reports it in argv[0], and keep a
   log next to the data. */
static void dRally_Switch_init(int argc, char * argv[]){

	char dir[256] = "sdmc:/switch/dRally";

	if((argc > 0) && argv[0] && (strncmp(argv[0], "sdmc:/", 6) == 0)){

		char * slash;

		strncpy(dir, argv[0], sizeof(dir)-1);
		dir[sizeof(dir)-1] = 0;
		slash = strrchr(dir, '/');
		if(slash) *slash = 0;
	}

	chdir(dir);
	freopen("log.txt", "w", stdout);
	setvbuf(stdout, NULL, _IOLBF, 0);
	printf("dRally for Nintendo Switch, running in %s\n", dir);

	SwitchConfig_load("SWITCH.CFG");
	printf("Settings: vsync %d, scaling %d, widescreen %d, rumble %d (%d%%), analog steering %d (dead zone %d%%, invert %d), gyro %d (range %d, dead zone %d degrees)\n",
		SwitchConfig.vsync, SwitchConfig.scaling, SwitchConfig.widescreen, SwitchConfig.rumble, SwitchConfig.rumble_percent, SwitchConfig.analog_steering,
		SwitchConfig.stick_deadzone, SwitchConfig.stick_invert_x, SwitchConfig.gyro, SwitchConfig.gyro_range, SwitchConfig.gyro_deadzone);

	/* All the game files at once, before anything else starts: a list of what is wrong is more
	   use than a program that just closes. */
	{
		char report[2048], text[3072];

		if(DataCheck_scan(".", report, sizeof(report)) > 0){

			printf("Game files:\n%s", report);
			snprintf(text, sizeof(text),
				"The Death Rally game files in\n  %s\nare not complete or not the original ones:\n\n%s\n"
				"Copy the files of your Death Rally installation (the Windows version, game version 1.1)\n"
				"into that folder; the list of files is in SWITCH.md.", dir, report);
			SwitchError_show("The game files are not right.", text);
			exit(0);
		}
	}
}

#endif // DR_SWITCH

int main(int argc, char * argv[]){

#if defined(DR_SWITCH)
	dRally_Switch_init(argc, argv);
#endif
	dRally_System_init();
#if defined(DR_LETTERBOX)
	dRally_Display_init(W_LETTERBOX);
#else
	dRally_Display_init(W_SHRINK);
#endif // DR_LETTERBOX
	___10060h();
	___60466h(70, 1);
	___2432c8h = &___100dch;
	dRally_Keyboard_init();
	___3e720h();

#if defined(DR_MULTIPLAYER)
	if(___19bd60h != 0) ___623d4h();
#endif

	dRally_Sound_quit();
	dRally_Display_clean();
	dRally_System_clean();

	return 0;
}
