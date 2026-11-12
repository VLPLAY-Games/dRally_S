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

The game keeps its settings (`DR.CFG`) and a `log.txt` in the same folder. `log.txt` is the file to attach to a bug report.

## Controls

The controller works as the keyboard the game was written for.

| Controller | Game |
| --- | --- |
| D-pad, left stick | Steer, move in menus |
| ZR | Accelerate (key A) |
| ZL | Brake (key Z) |
| R | Machine gun (Left Ctrl) |
| L | Drop mine (Left Alt) |
| X | Turbo boost (Left Shift) |
| Y | Horn (Space) |
| A | Confirm (Enter) |
| B, + | Back, pause (Escape) |
| - | Help (F1) |

The game controls can still be changed in its own options menu. The first 200 controller button presses are written to `log.txt` (`[dRally.INPUT]`), which helps to check the mapping.

## Known limitations

- Network play (IPX) is not part of the Switch build.
- Typing a name (for example for the hall of fame) needs a USB keyboard for now; the console keyboard is not hooked up yet.
- Not tested on hardware yet.

## Building

You need devkitPro with `switch-dev` and `switch-sdl2`:

```bash
dkp-pacman -S switch-dev switch-sdl2
make -f Makefile.switch        # builds dRally.nro
```

`make -f Makefile.switch clean` removes the build. The Linux build is the plain `Makefile`. Both use the object lists in `objs.mk`.

`tests/run.sh` builds and runs the host test of the controller mapping (a C compiler is enough).

## What was changed for the Switch

- `Makefile.switch`, `objs.mk` (the object lists, now shared with `Makefile`), the icon in `assets/switch/`.
- `switch_input.c`: the controller to keyboard mapping.
- `events.c`: controller events (Switch only).
- `drally_linux_c.c`: the picture is converted once per frame into one texture kept for the whole run, a fullscreen window, and a software renderer fallback.
- `drally.c`: start in the folder of the NRO and write `log.txt`.
- `system_linux_c.c`: the joystick subsystem.

All Switch code is under `DR_SWITCH`; the Linux build is unchanged.

## Reporting problems

Open an issue and attach `sdmc:/switch/dRally/log.txt` and, for crashes, the newest file from `sdmc:/atmosphere/crash_reports/`. Say which console you use (Switch, Lite, OLED), handheld or docked, and what you were doing.

## Credits and licence

dRally is by Neomalius (urxp) and contributors, under the MIT licence (see `LICENSE`); this port builds on [BSzili's fork](https://github.com/BSzili/dRally). Death Rally is a game by Remedy Entertainment and Apogee. This project is not affiliated with Remedy.
