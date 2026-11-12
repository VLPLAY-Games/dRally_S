#include "switch_rumble.h"
#include "switch_config.h"

int SwitchRumble_shape(int kind, int strength, int percent, unsigned * low, unsigned * high, unsigned * ms){

    float s = strength/65536.0f, l, h, m;

    if(percent <= 0) return 0;
    if(s < 0.0f) s = 0.0f;
    if(s > 1.0f) s = 1.0f;

    switch(kind){
    case SWITCHRUMBLE_IMPACT:           /* the harder the hit, the stronger and the longer */
        if(s < 0.05f) return 0;
        l = 0.25f+0.75f*s;
        h = 0.15f+0.60f*s;
        m = 60.0f+140.0f*s;
        break;
    case SWITCHRUMBLE_MINE_DROP:        /* a light tick */
        l = 0.0f;
        h = 0.35f;
        m = 40.0f;
        break;
    case SWITCHRUMBLE_MINE_BLAST:       /* the nearer, the stronger */
        if(s < 0.05f) return 0;
        l = 0.40f+0.60f*s;
        h = 0.30f+0.50f*s;
        m = 150.0f+250.0f*s;
        break;
    case SWITCHRUMBLE_BLIP:             /* a short confirmation (a setting was changed) */
        l = 0.0f;
        h = 0.6f;
        m = 70.0f;
        break;
    default:
        return 0;
    }

    l *= percent/100.0f;
    h *= percent/100.0f;
    *low = (unsigned)(l*65535.0f);
    *high = (unsigned)(h*65535.0f);
    *ms = (unsigned)m;
    return 1;
}

#if defined(DR_SWITCH)

#include <SDL.h>

void SwitchRumble_event(int kind, int strength){

    unsigned low, high, ms;
    int n;

    if(!SwitchConfig.rumble && (kind != SWITCHRUMBLE_BLIP)) return;
    if(!SwitchRumble_shape(kind, strength, (kind == SWITCHRUMBLE_BLIP) ? 100 : SwitchConfig.rumble_percent, &low, &high, &ms)) return;

    /* every controller that is connected, so a pair of Joy-Con or a Pro controller both shake */
    for(n = 0; n < SDL_NumJoysticks(); n++){

        SDL_Joystick * joystick = SDL_JoystickFromInstanceID(SDL_JoystickGetDeviceInstanceID(n));

        if(joystick) SDL_JoystickRumble(joystick, (Uint16)low, (Uint16)high, ms);
    }
}

#endif // DR_SWITCH
