/* Host test of the game's clock. Build and run: tests/run.sh */
#include <stdio.h>
#include "../include/frame_clock.h"

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

int main(void){

    FrameClock c = {0};
    const unsigned frame_us = 1000000/70;       /* 14285 */
    unsigned long long now;
    int total, calls, i;

    /* the first call only starts the clock */
    CHECK(FrameClock_advance(&c, 5000000ULL, frame_us, 7) == 0);

    /* presenting at 60 Hz for 10 s: the game still gets 70 frames a second */
    total = 0;
    for(i = 1; i <= 600; i++){

        now = 5000000ULL+(unsigned long long)i*16667ULL;
        total += FrameClock_advance(&c, now, frame_us, 7);
    }
    CHECK(total >= 698 && total <= 701);

    /* and never more than 2 frames in one present */
    c.started = 0;
    FrameClock_advance(&c, 0, frame_us, 7);
    for(i = 1, calls = 0; i <= 600; i++){

        int f = FrameClock_advance(&c, (unsigned long long)i*16667ULL, frame_us, 7);
        if(f > 2) calls++;
    }
    CHECK(calls == 0);

    /* polling every millisecond gives a frame about every 14.3 ms */
    c.started = 0;
    FrameClock_advance(&c, 0, frame_us, 7);
    total = 0;
    for(i = 1; i <= 10000; i++) total += FrameClock_advance(&c, (unsigned long long)i*1000ULL, frame_us, 7);
    CHECK(total >= 698 && total <= 701);

    /* after a long stall (the HOME menu) only a few frames are caught up, then it is steady again */
    c.started = 0;
    FrameClock_advance(&c, 0, frame_us, 7);
    CHECK(FrameClock_advance(&c, 30000000ULL, frame_us, 7) == 7);
    CHECK(FrameClock_advance(&c, 30000000ULL+frame_us-1, frame_us, 7) == 0);
    CHECK(FrameClock_advance(&c, 30000000ULL+frame_us, frame_us, 7) == 1);

    /* time until the next frame */
    c.started = 0;
    FrameClock_advance(&c, 1000, frame_us, 7);
    CHECK(FrameClock_remaining(&c, 1000+4000, frame_us) == frame_us-4000);
    CHECK(FrameClock_remaining(&c, 1000+frame_us+5, frame_us) == 0);

    if(failures) printf("%d check(s) failed\n", failures);
    else printf("frame_clock: all checks passed\n");
    return failures != 0;
}
