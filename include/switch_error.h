#ifndef SWITCH_ERROR_H
#define SWITCH_ERROR_H

/* Shows a message on the console's text screen and waits for a button, then returns; the caller
   ends the program. Used for a problem the player can fix (missing game files), where the
   application would otherwise just close. */
void SwitchError_show(const char * title, const char * text);

/* The same for a problem found while the game runs: the game's window is closed first. */
void SwitchError_fatal(const char * text);

#endif // SWITCH_ERROR_H
