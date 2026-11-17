#include "switch_input.h"
#include "switch_text.h"
#include "switch_config.h"
#include "switch_steer.h"
#include "switch_rumble.h"

/* The controller drives the game through the keyboard it was written for. Death Rally steers
   with the arrows and uses a few keys in a race (defaults of the original): A accelerate, Z brake,
   Left Ctrl machine gun, Left Alt drop mine, Left Shift turbo, Space horn. Several controller
   inputs can map to one key, so the key states are recomputed from the raw input state and a
   make/break is sent only when a key changes. */

#include <stdlib.h>
#if defined(__SWITCH__)
#include <switch.h>
#include <stdio.h>
#endif

#define SW_MAX_BUTTONS 32
#define SW_AXIS_THRESHOLD 12000

/* Button order of the SDL2 Switch joystick driver (the libnx HID button bits). */
enum {
    SW_A, SW_B, SW_X, SW_Y, SW_LSTICK, SW_RSTICK, SW_L, SW_R, SW_ZL, SW_ZR, SW_PLUS, SW_MINUS,
    SW_DLEFT, SW_DUP, SW_DRIGHT, SW_DDOWN, SW_STICK_LEFT, SW_STICK_UP, SW_STICK_RIGHT, SW_STICK_DOWN
};

static struct {
    int             race;
    unsigned char   button[SW_MAX_BUTTONS];
    int             axis_x, axis_y;
    int             hat;
    unsigned char   key[SDL_NUM_SCANCODES];
} SW = {0};

static void set_key(SDL_Scancode sc, int down){

    down = !!down;
    if(SW.key[sc] != down){

        SW.key[sc] = down;
        if(down) dRally_Keyboard_make(sc);
        else dRally_Keyboard_break(sc);
    }
}

/* In a race with fine steering on, the stick does not press Left and Right (SwitchSteer turns its
   position into steering), and it only accelerates or brakes when it is pushed clearly up or down:
   steering diagonally must not brake. The D-pad is always on or off. */
static int fine_steering(void){

    return SW.race && SwitchConfig.analog_steering;
}

static void update_keys(void){

    int left, right, up, down;
    int stick_left, stick_right, stick_up, stick_down;

    stick_left  = SW.button[SW_STICK_LEFT]  || SW.axis_x < -SW_AXIS_THRESHOLD;
    stick_right = SW.button[SW_STICK_RIGHT] || SW.axis_x >  SW_AXIS_THRESHOLD;
    stick_up    = SW.button[SW_STICK_UP]    || SW.axis_y < -SW_AXIS_THRESHOLD;
    stick_down  = SW.button[SW_STICK_DOWN]  || SW.axis_y >  SW_AXIS_THRESHOLD;

    if(fine_steering()){

        const int ay = abs(SW.axis_y), ax = abs(SW.axis_x);
        const int vertical = (ay > 2*SW_AXIS_THRESHOLD) && (ay > 2*ax);     /* clearly up or down */

        stick_left = stick_right = 0;
        stick_up = vertical && (SW.axis_y < 0);
        stick_down = vertical && (SW.axis_y > 0);
    }

    if(SW.race && SwitchConfig.analog_steering && SwitchConfig.stick_mode){

        /* direction steering: the stick only points, ZR and ZL are the gas and the brake */
        stick_left = stick_right = stick_up = stick_down = 0;
    }

    left  = SW.button[SW_DLEFT]  || stick_left  || (SW.hat & SDL_HAT_LEFT);
    right = SW.button[SW_DRIGHT] || stick_right || (SW.hat & SDL_HAT_RIGHT);
    up    = SW.button[SW_DUP]    || stick_up    || (SW.hat & SDL_HAT_UP);
    down  = SW.button[SW_DDOWN]  || stick_down  || (SW.hat & SDL_HAT_DOWN);

    set_key(SDL_SCANCODE_LEFT,   left);
    set_key(SDL_SCANCODE_RIGHT,  right);
    set_key(SDL_SCANCODE_UP,     up);
    set_key(SDL_SCANCODE_DOWN,   down);

    set_key(SDL_SCANCODE_RETURN, SW.button[SW_A]);                       /* confirm */
    if(SW.race){

        /* Escape ends the race, P pauses it: Plus pauses, Minus leaves, B is left alone */
        set_key(SDL_SCANCODE_P,      SW.button[SW_PLUS]);
        set_key(SDL_SCANCODE_ESCAPE, SW.button[SW_MINUS]);
        set_key(SDL_SCANCODE_TAB,    SW.button[SW_RSTICK]);              /* view */
        set_key(SDL_SCANCODE_F1,     0);
    }
    else {

        set_key(SDL_SCANCODE_P,      0);
        set_key(SDL_SCANCODE_TAB,    0);
        set_key(SDL_SCANCODE_ESCAPE, SW.button[SW_B] || SW.button[SW_PLUS]); /* back */
        set_key(SDL_SCANCODE_F1,     SW.button[SW_MINUS]);                   /* help */
    }
    set_key(SDL_SCANCODE_LSHIFT, SW.button[SW_X]);                       /* turbo */
    set_key(SDL_SCANCODE_SPACE,  SW.button[SW_Y]);                       /* horn */
    set_key(SDL_SCANCODE_LALT,   SW.button[SW_L]);                       /* drop mine */
    set_key(SDL_SCANCODE_LCTRL,  SW.button[SW_R]);                       /* machine gun */
    set_key(SDL_SCANCODE_Z,      SW.button[SW_ZL]);                      /* brake */
    set_key(SDL_SCANCODE_A,      SW.button[SW_ZR]);                      /* accelerate */
}

void SwitchInput_setRace(int race){

    SW.race = !!race;
    update_keys();
}

void SwitchInput_button(int button, int down){

    if((button >= 0) && (button < SW_MAX_BUTTONS)){

        SW.button[button] = !!down;
        update_keys();

        /* a click of the left stick on a name entry screen opens the on-screen keyboard again */
        if((button == SW_LSTICK) && down && !SW.race) SwitchText_request();
    }
}

void SwitchInput_hat(int mask){

    SW.hat = mask;
    update_keys();
}

void SwitchInput_axis(int axis, int value){

    if(axis == 0){

        if(SwitchConfig.stick_invert_x) value = (value == -32768) ? 32767 : -value;
        SW.axis_x = value;
        SwitchSteer_setStick(value/32767.0f);
    }
    else if(axis == 1){

        SW.axis_y = value;
        SwitchSteer_setStickY(value/32767.0f);
    }
    update_keys();
}

#if defined(DR_SWITCH)
static SDL_JoystickID localPlayers[SWITCH_LOCAL_MAX_PLAYERS];
static unsigned char mineHeld[SWITCH_LOCAL_MAX_PLAYERS];
static SwitchSteerState localSteering[SWITCH_LOCAL_MAX_PLAYERS];
static int localCount, joinedCount, localLobby;

#if defined(__SWITCH__)
static PadState localPads[SWITCH_LOCAL_MAX_PLAYERS];

Result __real_hidLaShowControllerSupportForSystem(HidLaControllerSupportResultInfo *, const HidLaControllerSupportArg *, bool);

Result __wrap_hidLaShowControllerSupportForSystem(HidLaControllerSupportResultInfo *info,
        const HidLaControllerSupportArg *args, bool flag){
    /* SDL invokes its eight-slot applet when controller assignments change. */
    if(localLobby || localCount) return MAKERESULT(Module_Libnx, LibnxError_LibAppletBadExit);
    return __real_hidLaShowControllerSupportForSystem(info, args, flag);
}

static int local_pad_connected(int player){
    padUpdate(&localPads[player]);
    return padIsConnected(&localPads[player]);
}
#endif

int SwitchLocal_openLobby(void){
    SwitchLocal_end();
    localLobby = 1;
#if defined(__SWITCH__)
    static const HidNpadIdType ids[SWITCH_LOCAL_MAX_PLAYERS] = {
        HidNpadIdType_No1, HidNpadIdType_No2, HidNpadIdType_No3, HidNpadIdType_No4
    };
    HidLaControllerSupportArg args;
    HidLaControllerSupportResultInfo info = {0};
    u32 originalStyles;
    Result rc = hidGetSupportedNpadStyleSet(&originalStyles);
    if(R_FAILED(rc)){
        SwitchLocal_end();
        return 0;
    }

    hidLaCreateControllerSupportArg(&args);
    args.hdr.player_count_min = 2;
    args.hdr.player_count_max = SWITCH_LOCAL_MAX_PLAYERS;
    rc = hidSetSupportedNpadStyleSet(HidNpadStyleSet_NpadFullCtrl);
    if(R_SUCCEEDED(rc)) rc = hidSetSupportedNpadIdType(ids, SWITCH_LOCAL_MAX_PLAYERS);
    if(R_SUCCEEDED(rc)) rc = hidLaShowControllerSupport(&info, &args);
    /* Restore the eight numbered slots and handheld input configured by SDL. */
    padConfigureInput(8, originalStyles);
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_JOYBUTTONDOWN, SDL_JOYBUTTONUP);
    printf("[dRally.LOCAL] Controller applet: result=0x%x, external=%d\n", rc, info.player_count);
    if(R_FAILED(rc) || info.player_count < 2 || info.player_count > SWITCH_LOCAL_MAX_PLAYERS){
        SwitchLocal_end();
        return 0;
    }

    /* The applet assigns accepted controllers to consecutive slots from No1. */
    joinedCount = info.player_count;
    for(int i = 0; i < joinedCount; ++i){
        padInitialize(&localPads[i], ids[i]);
        localPlayers[i] = ids[i];
        printf("[dRally.LOCAL] Player %d: Npad %d\n", i+1, localPlayers[i]);
    }
#endif
    return 1;
}

int SwitchLocal_joined(void){
#if defined(__SWITCH__)
    int connected = 0;
    for(int i = 0; i < joinedCount; ++i) connected += local_pad_connected(i);
    return connected;
#else
    for(int i = 0; i < joinedCount; ){
        SDL_Joystick *pad = SDL_JoystickFromInstanceID(localPlayers[i]);
        if(pad && SDL_JoystickGetAttached(pad)){
            ++i;
            continue;
        }
        for(int j = i; j+1 < joinedCount; ++j) localPlayers[j] = localPlayers[j+1];
        --joinedCount;
    }
    return joinedCount;
#endif
}

int SwitchLocal_lobbyButton(SDL_JoystickID id, int button, int down){
    if(!localLobby) return 0;
    if(button == SW_A){
#if !defined(__SWITCH__)
        if(down && SwitchLocal_joined() < SWITCH_LOCAL_MAX_PLAYERS){
            for(int i = 0; i < joinedCount; ++i){
                if(localPlayers[i] == id) return 1;
            }
            localPlayers[joinedCount++] = id;
        }
#endif
        return 1;
    }
    if(button == SW_PLUS){
        if(down) dRally_Keyboard_make(SDL_SCANCODE_RETURN);
        else dRally_Keyboard_break(SDL_SCANCODE_RETURN);
        return 1;
    }
    return 0;
}

int SwitchLocal_begin(void){
    const int count = SwitchLocal_joined();
    if(count < 2) return 0;
#if defined(__SWITCH__)
    if(count != joinedCount) return 0;
    for(int i = 0; i < count; ++i){
        mineHeld[i] = !!(padGetButtons(&localPads[i]) & HidNpadButton_L);
    }
#else
    for(int i = 0; i < count; ++i){
        SDL_Joystick *pad = SDL_JoystickFromInstanceID(localPlayers[i]);
        mineHeld[i] = pad && SDL_JoystickGetButton(pad, SW_L);
    }
#endif
    localCount = count;
    localLobby = 0;
    for(int i = 0; i < count; ++i) localSteering[i] = (SwitchSteerState){0};
    dRally_Keyboard_break(SDL_SCANCODE_RETURN);
    return localCount;
}

void SwitchLocal_end(void){
    localCount = localLobby = joinedCount = 0;
    dRally_Keyboard_break(SDL_SCANCODE_RETURN);
    for(int i = 0; i < SW_MAX_BUTTONS; ++i) SW.button[i] = 0;
    SW.axis_x = SW.axis_y = SW.hat = 0;
    update_keys();
}

int SwitchLocal_count(void){ return localCount; }

int SwitchLocal_acceptMenuInput(SDL_JoystickID id){
    for(int i = 0; i < localCount; ++i){
#if defined(__SWITCH__)
        if(local_pad_connected(i)) return id == SDL_JoystickGetDeviceInstanceID(localPlayers[i]);
#else
        SDL_Joystick *pad = SDL_JoystickFromInstanceID(localPlayers[i]);
        if(pad && SDL_JoystickGetAttached(pad)) return id == localPlayers[i];
#endif
    }
    return 1;
}

unsigned char SwitchLocal_controls(int player, float direction){
    unsigned char controls = 0;
    int x, y, left, right, mine;
    unsigned int buttons;

#if defined(__SWITCH__)
    if(!local_pad_connected(player)){
        localSteering[player] = (SwitchSteerState){0};
        return 0;
    }
    const HidAnalogStickState stick = padGetStickPos(&localPads[player], 0);
    x = stick.x;
    y = -stick.y; /* libnx uses positive up, the game uses positive down */
    buttons = (unsigned int)padGetButtons(&localPads[player]);
#else
    SDL_Joystick *pad = SDL_JoystickFromInstanceID(localPlayers[player]);
    if(!pad || !SDL_JoystickGetAttached(pad)){
        localSteering[player] = (SwitchSteerState){0};
        return 0;
    }
    x = SDL_JoystickGetAxis(pad, 0);
    y = SDL_JoystickGetAxis(pad, 1);
    buttons = 0;
    for(int i = SW_X; i <= SW_STICK_RIGHT; ++i) buttons |= (unsigned int)SDL_JoystickGetButton(pad, i) << i;
    if(SDL_JoystickNumHats(pad)){
        const int hat = SDL_JoystickGetHat(pad, 0);
        if(hat & SDL_HAT_LEFT) buttons |= 1u << SW_DLEFT;
        if(hat & SDL_HAT_RIGHT) buttons |= 1u << SW_DRIGHT;
    }
#endif
    if(SwitchConfig.stick_invert_x) x = -x;
    SwitchSteer_tickStick(&localSteering[player], x/32767.0f, y/32767.0f, direction, &left, &right);
    if(!SwitchConfig.analog_steering){
        left = (buttons & (1u << SW_STICK_LEFT)) || x < -SW_AXIS_THRESHOLD;
        right = (buttons & (1u << SW_STICK_RIGHT)) || x > SW_AXIS_THRESHOLD;
    }
    if(left || (buttons & (1u << SW_DLEFT))) controls |= SWITCH_RACE_LEFT;
    if(right || (buttons & (1u << SW_DRIGHT))) controls |= SWITCH_RACE_RIGHT;
    if(buttons & (1u << SW_ZR)) controls |= SWITCH_RACE_GAS;
    if(buttons & (1u << SW_ZL)) controls |= SWITCH_RACE_BRAKE;
    if(buttons & (1u << SW_X)) controls |= SWITCH_RACE_TURBO | SWITCH_RACE_GAS;
    if(buttons & (1u << SW_R)) controls |= SWITCH_RACE_GUN;
    mine = !!(buttons & (1u << SW_L));
    if(mine && !mineHeld[player]) controls = (controls & ~SWITCH_RACE_BRAKE) | SWITCH_RACE_MINE;
    mineHeld[player] = mine;
    return controls;
}
#endif
