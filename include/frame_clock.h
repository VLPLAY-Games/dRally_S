#ifndef FRAME_CLOCK_H
#define FRAME_CLOCK_H

/* The game's clock: a frame every 1/70 s (14285.7 us). The time is kept in microseconds and the
   remainder is not thrown away, so the game runs at 70 frames a second on average even when the
   screen presents at 60 and each present waits for the screen. */

typedef struct {
    int                 started;
    unsigned long long  base_us;
} FrameClock;

/* Frames that have passed since the last call (usually 0, 1 or 2). If more than max_frames have
   passed (the system was suspended, the keyboard was open) only max_frames count and the clock
   restarts from now. */
int FrameClock_advance(FrameClock * c, unsigned long long now_us, unsigned frame_us, int max_frames);

/* Microseconds until the next frame is due. */
unsigned FrameClock_remaining(const FrameClock * c, unsigned long long now_us, unsigned frame_us);

#endif // FRAME_CLOCK_H
