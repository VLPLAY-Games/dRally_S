#ifndef SWITCH_RUMBLE_H
#define SWITCH_RUMBLE_H

/* Vibration of the controller in races. The events come from the race code for the player's own
   car: a hit (how hard, 0 to 0x10000, the game's own measure), a mine dropped, a mine going off
   (how near, 0 to 0x10000). SwitchConfig sets whether and how strong. */

enum { SWITCHRUMBLE_NONE, SWITCHRUMBLE_IMPACT, SWITCHRUMBLE_MINE_DROP, SWITCHRUMBLE_MINE_BLAST, SWITCHRUMBLE_BLIP };

/* How a kind of event feels: motor strengths 0..65535 (low = the big motor, high = the small one)
   and the length in ms. Returns 0 when there is nothing to do. */
int SwitchRumble_shape(int kind, int strength, int percent, unsigned * low, unsigned * high, unsigned * ms);

void SwitchRumble_event(int kind, int strength);

#endif // SWITCH_RUMBLE_H
