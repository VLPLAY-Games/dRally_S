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
	printf("DEATH RALLY Exit: CTRL+ALT+DEL pressed!\n");
	exit(0x70);
}

#if defined(DR_SWITCH)

#include <switch.h>
#include <unistd.h>

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
