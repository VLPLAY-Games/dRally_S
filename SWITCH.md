# dRally for Nintendo Switch

A port of [dRally](https://github.com/BSzili/dRally), an open source engine for **Death Rally (1996)**, to the Nintendo Switch homebrew platform (devkitA64 / libnx / SDL2).

> [!IMPORTANT]
> This repository contains **no Death Rally game data**. You need your own copy of the game (Remedy's free Windows version, also available as *Death Rally (Classic)* on Steam) and you copy its files to your own console.

> [!NOTE]
> **Status: first build, not tested on a console yet.** It builds and links, and the controller mapping is covered by a host test, but nobody has run it on hardware. Please report what works and what does not (see [Reporting problems](#reporting-problems)).

## Requirements

- A Nintendo Switch with custom firmware that runs homebrew (for example Atmosphère) and the Homebrew Menu.
- Death Rally game files (see below).

## Installation

```text
sdmc:/switch/dRally/
├── dRally.nro
├── CDROM.INI              contains the text ./CINEM
├── ENGINE.BPA
├── IBFILES.BPA
├── MENU.BPA
├── MUSICS.BPA
├── TR0.BPA ... TR9.BPA
└── CINEM/
    ├── DR.IDF
    ├── ENDANI.HAF
    ├── ENDANI0.HAF
    └── SANIM.HAF
```

1. Copy `dRally.nro` to `sdmc:/switch/dRally/`.
2. Copy the game files from your Death Rally installation into the same folder. The names must be in upper case, and the `.HAF` and `.IDF` files go into `CINEM/`. `CDROM.INI` is a one-line text file holding `./CINEM`.
3. Start **dRally** from the Homebrew Menu.

The game keeps its settings (`DR.CFG`), the settings of this port (`SWITCH.CFG`, written the first time) and a `log.txt` in the same folder. `log.txt` is the file to attach to a bug report.

If a game file is missing or is not the original one, the game does not start; a text screen lists what is wrong (for example `MENU.BPA: not found`) and waits for a button. The game checks the exact size of each file, as the original does, so the files must be those of Death Rally 1.1.

## Controls

The controller works as the keyboard the game was written for. The game steers, brakes and accelerates with on/off keys (there is no analog steering), and the Up and Down arrows always accelerate and brake, so the stick or D-pad alone is enough to drive.

| Controller | In the menus | In a race |
| --- | --- | --- |
| D-pad | Move | Steer (on or off); up accelerates, down brakes |
| Left stick | Move | Fine steering; pushed clearly up or down it accelerates or brakes |
| Left stick click | (on a name screen: the keyboard) | Gyro steering on or off |
| ZR | | Accelerate |
| ZL | | Brake |
| R | | Machine gun |
| L | | Drop mine |
| X | | Turbo boost |
| Y | | Horn |
| A | Confirm | |
| B | Back | (nothing) |
| + | Back | **Pause** |
| - | Help (F1) | **Leave the race** |
| Right stick click | | Switch the view (Tab) |

In a race the game ends the race the moment Escape is pressed and pauses on P, so **+ pauses and - leaves**, and B does nothing there. The game controls can still be changed in its own options menu. The first 200 controller button presses are written to `log.txt` (`[dRally.INPUT]`), which helps to check the mapping.

### Steering

The game itself only knows steer left, steer right (on or off, once per frame). The port adds:

- **Fine steering with the left stick** (on by default): how far the stick is pushed decides how often the steering flag is on; half way is every second frame, full is every frame. The D-pad is still on or off. In a race the stick only accelerates or brakes when it is pushed clearly up or down (not when steering diagonally). Set `analog_steering = 0` in `SWITCH.CFG` for the old behaviour.
- **Steering by tilting the console** (handheld mode, off by default): turn the console like a wheel. Switch it on in `SWITCH.CFG` (`gyro = 1`) or in a race by clicking the **left stick** (a short vibration confirms); the position of the console at that moment is straight. It adds to the stick. `gyro_range` is the tilt for full steering, `gyro_deadzone` the tilt that does nothing.

### Vibration

In a race the controller vibrates when your car hits something (harder for a harder hit), when you drop a mine and when a mine goes off (nearer is stronger). `rumble` and `rumble_percent` in `SWITCH.CFG` switch it off or change how strong it is.

### Picture timing

The picture waits for the screen (vsync, no tearing) while the game keeps its own clock of 70 frames a second. If the game runs too slowly or stutters, set `vsync = 0` in `SWITCH.CFG`. `log.txt` says whether vsync is on (`[dRally.DISPLAY]`).

### Settings file

`SWITCH.CFG` is a text file with one `name = value` per line, explained in the file itself:

| Setting | Default | Meaning |
| --- | --- | --- |
| `vsync` | 1 | the picture waits for the screen |
| `rumble`, `rumble_percent` | 1, 100 | vibration in races and how strong |
| `analog_steering` | 1 | fine steering with the left stick |
| `stick_deadzone` | 15 | percent of the stick's travel that does nothing |
| `gyro` | 0 | steering by tilting the console |
| `gyro_range`, `gyro_deadzone` | 25, 3 | degrees for full steering, degrees that do nothing |

### Touch screen

In handheld mode the menus can be used by touch:

- **Menus:** tap a row to choose it and confirm.
- **Volume sliders** (Configure): drag along the bar.
- **Yes / no questions:** tap "yes" (left half) or "no" (right half).
- **Anywhere else:** a **tap** is Enter, a **swipe** is the arrow keys (one key per 40 screen pixels of movement) and a **tap with two fingers** is Back (Escape).
- **In a race** touching the screen never steers or confirms; only a tap with two fingers works, and it **pauses**.

The touch screen does not exist when the console is docked.

### Typing names

On a screen where the game asks for a name (driver name, save game name, hall of fame) the console's on-screen keyboard opens by itself. What you type is entered into the game's own screen; confirm it with **A** as usual. If you cancel the keyboard, **click the left stick** to open it again. The game accepts capitals, digits, space and `- = , . /` and limits the length by itself.

### Sound

Sound goes through the console's audio renderer (`audren`), at the sample rate the game asks for. The old `audout` service that SDL uses produced audible noise on loud low tones in a similar project, so it is only a fallback if `audren` cannot start (the log says which one is used: `[dRally.SOUND]`).

## Known limitations

- Network play (IPX) is not part of the Switch build.
- The game's own text entry only knows capitals, digits and a few signs, so other characters from the on-screen keyboard are dropped.
- The menu entries for the PC joystick (Define Gamepad/Joystick, Gamepad/Joystick) and "Multiplayer Race" are switched off on the Switch: the console controller is always active and the network play is not in the build.
- Not tested on hardware yet: sound, the on-screen keyboard, the controller mapping, vibration, fine steering and the tilt of the console have only been tested as far as is possible on a PC (see `tests/`). The direction of the tilt (clockwise is right) and the strength of the vibration are the first things to check.

## Building

You need devkitPro with `switch-dev` and `switch-sdl2`:

```bash
dkp-pacman -S switch-dev switch-sdl2
make -f Makefile.switch        # builds dRally.nro
```

`make -f Makefile.switch clean` removes the build. The Linux build is the plain `Makefile`. Both use the object lists in `objs.mk`.

`tests/run.sh` builds and runs the host tests of the Switch code (controller mapping, on-screen keyboard, touch gestures); a C compiler is enough.

## Repository layout

The sources used to sit in one folder; they are now sorted (the Linux and Switch builds both use `vpath` over `src/`):

| Folder | Contents |
| --- | --- |
| `include/` | all headers |
| `src/core/` | start-up, data files (`.BPA`), configuration, fonts, palettes, graphics helpers |
| `src/game/` | the game logic, functions named after their address in the original program |
| `src/race/`, `src/menu/`, `src/shop/`, `src/underground/` | the race, the menus, the shop and the underground (garage) |
| `src/sound/` | the music and effect player (S3M and XM trackers) and the audio API |
| `src/network/` | IPX network play |
| `src/dfr/` | functions that are still assembly in the original (stubs) |
| `src/platform/` | the SDL layer: window, keyboard, events |
| `src/platform/dos/` | what DOS and the Watcom runtime gave the game: interrupts, memory, FPU, random numbers |
| `src/platform/switch/` | the Nintendo Switch code: controller, audio (`audren`), on-screen keyboard |
| `tools/` | helper scripts; `ipxnet/` is the IPX server; `tests/` are host tests of the Switch code |

## What was changed for the Switch

- `Makefile.switch`, `objs.mk` (the object lists, shared with `Makefile`), the icon in `assets/switch/`.
- `src/platform/switch/switch_input.c`: the controller to keyboard mapping.
- `src/platform/switch/switch_audio.c`: audio through `audren` (with SDL audio as a fallback in `src/sound/sound_api.c`).
- `src/platform/switch/switch_text.c`: the on-screen keyboard; the entry screen (`src/game/___17510h.c`) is wrapped to start and stop it.
- `src/platform/switch/switch_keys.c`: key presses the console itself produces (on-screen keyboard, touch gestures), held until the game has read them.
- `src/platform/switch/switch_touch.c`, `switch_touch_hid.c`: touch gestures and the touch panel (HID); the menus, volume sliders and the yes/no question read taps and drags (`src/game/___146c4h.c`, `src/menu/menu___218b4h.c`, `src/game/___148cch.c`).
- `src/platform/events.c`: controller events and the pumps (Switch only).
- `src/race/race_main.c`: wrapped, so the controls and the touch screen know a race is running.
- `src/menu/menu_data.c`, `menu_main.c`: "Exit Game" instead of "Exit To DOS", switched-off entries, a hint line for the console.
- `src/platform/drally_linux_c.c`: also a limit on how many frames the game catches up after the HOME menu or the system keyboard.
- `src/platform/drally_linux_c.c`: the picture is converted once per frame into one texture kept for the whole run, a fullscreen window, and a software renderer fallback.
- `src/core/drally.c`: start in the folder of the NRO, write `log.txt`, read `SWITCH.CFG` and check the game files (`src/core/datacheck.c`) before anything else; `src/platform/switch/switch_error.c` shows the text screen.
- `src/platform/switch/switch_config.c`: the settings file.
- `src/core/frame_clock.c` and `src/platform/drally_linux_c.c`: vsync with a steady 70 frames a second (a clock that keeps the remainder), and the retrace waits are skipped when the picture waits for the screen.
- `src/platform/switch/switch_steer.c`, `switch_gyro.c`: fine steering (the stick's position as how often the flag is on) and the tilt of the console; `race___40164h.c` adds them to the game's own steering flags.
- `src/platform/switch/switch_rumble.c`: vibration, called from the race code for the player's car (`race_main.c`, `race___514d8h.c`).

All Switch code is under `DR_SWITCH`; the Linux build behaves as before.

## Reporting problems

Open an issue and attach `sdmc:/switch/dRally/log.txt` and, for crashes, the newest file from `sdmc:/atmosphere/crash_reports/`. Say which console you use (Switch, Lite, OLED), handheld or docked, and what you were doing.

## Credits and licence

dRally is by Neomalius (urxp) and contributors, under the MIT licence (see `LICENSE`); this port builds on [BSzili's fork](https://github.com/BSzili/dRally). Death Rally is a game by Remedy Entertainment and Apogee. This project is not affiliated with Remedy.
