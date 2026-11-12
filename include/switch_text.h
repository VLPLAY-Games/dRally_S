#ifndef SWITCH_TEXT_H
#define SWITCH_TEXT_H

/* Text entry with the console's on-screen keyboard.

   The game reads names (driver, save game, hall of fame) by scan code from its own entry screen.
   While that screen runs, the system software keyboard opens on its own; the text it returns is
   typed into the screen as key presses (backspaces first, then the letters), so the game's own
   code does the rest. Pressing the left stick opens the keyboard again. */

/* Called when the entry screen starts and ends; max_chars is the length the screen accepts. */
void SwitchText_begin(int max_chars);
void SwitchText_end(void);

/* Ask for a line of text with the keyboard, right now (the game waits). `initial` is shown in the box.
   Returns 1 and the text in `out` when it was confirmed and is not empty, 0 when cancelled. */
int SwitchText_prompt(const char * header, const char * guide, const char * initial, char * out, unsigned out_size);

/* Open the keyboard again (the entry screen must be running). */
void SwitchText_request(void);

/* Called from the event loop: opens the keyboard and feeds the typed keys to the game. */
void SwitchText_pump(void);

#endif // SWITCH_TEXT_H
