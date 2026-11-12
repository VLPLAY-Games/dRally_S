#include "frame_clock.h"

int FrameClock_advance(FrameClock * c, unsigned long long now_us, unsigned frame_us, int max_frames){

    unsigned long long elapsed, frames;

    if(!c->started || (now_us < c->base_us)){

        c->started = 1;
        c->base_us = now_us;
        return 0;
    }

    elapsed = now_us-c->base_us;
    frames = elapsed/frame_us;

    if(frames == 0) return 0;

    if(frames > (unsigned long long)max_frames){

        c->base_us = now_us;
        return max_frames;
    }

    c->base_us += frames*frame_us;
    return (int)frames;
}

unsigned FrameClock_remaining(const FrameClock * c, unsigned long long now_us, unsigned frame_us){

    const unsigned long long elapsed = c->started ? (now_us-c->base_us) : 0;

    return (elapsed >= frame_us) ? 0 : (unsigned)(frame_us-elapsed);
}
