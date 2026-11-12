# dRally for Nintendo Switch

**Death Rally (1996) on the Nintendo Switch.** A port of the open source [dRally](https://github.com/BSzili/dRally) engine to the Switch homebrew platform (devkitA64 / libnx / SDL2), built around your own copy of the game.

[![Platform](https://img.shields.io/badge/platform-Nintendo%20Switch-e60012.svg)](#requirements)
[![Status](https://img.shields.io/badge/status-1.1.0-d97a1f.svg)](#status)
[![Licence](https://img.shields.io/badge/Licence-MIT-blue.svg)](LICENSE)

> [!IMPORTANT]
> This repository contains **no Death Rally game data**. You need your own copy of the **1996 game** (version 1.1, the DOS release) and you copy its files to your own console. The 2012 remake sold on Steam is a different game and does **not** work.

- [Status](#status)
- [Requirements](#requirements)
- [Installation](#installation)
- [Playing](#playing)
- [Settings](#settings)
- [Known limitations](#known-limitations)
- [Building from source](#building-from-source)
- [Branches](#branches)
- [Credits](#credits)
- [Кратко по-русски](#кратко-по-русски)

---

## Status

The game is playable from the menus to the races, with sound, vibration and touch control in the menus. This is the 1.1.0 release line of the Switch port, on the `master` branch. The `switch` branch is used for testing, and `upstream-master` keeps the state of the repository before the port.

| Area | State |
| --- | --- |
| Game | Menus, shop, races, saves |
| Picture | 640x480 menus and 320x240 races, smooth scaling by default; optional **smart scaling** (whole-number enlargement, sharp pixels) and **16:9** stretch, both in the game's Configure menu |
| Sound | Through the console's audio renderer (`audren`) |
| Controls | The console's controllers; the left stick **points where the car should go on the screen**; vibration; **touch screen in the menus**; system on-screen keyboard for names |
| Network play | **Experimental** (new): an IPX race through a small server on a computer in your network (`ipxnet`), set in `SWITCH.CFG`; not tried with two players yet |

**New in 1.1.0:** the **network race** (experimental): choose Multiplayer Race, enter your name, then the server address with the keyboard; the other players join through one small server (`ipxnet`, a Python script in the sources) that runs on a computer in your network. The connection to the server works; a race with two players has not been tried yet. See [SWITCH.md](SWITCH.md#network-race-experimental).

**What was tested:** a Switch in handheld mode, with the original game files from the Death Rally 1.1 CD: menus, races, steering with the stick, vibration and the 16:9 mode. Not tested yet: docked mode, touch in the 16:9 mode, and controllers other than the built-in one.

---

## Requirements

- A Nintendo Switch with custom firmware that runs homebrew (for example Atmosphère) and the Homebrew Menu (`hbmenu`).
- **No overclocking is needed.** The game is from 1996 and is light for the console; it runs at the stock clocks. If you like, you can do the opposite and set **lower clocks** with a tool such as Horizon OC or sys-clk to save battery. How low you can go has not been measured, so try it for yourself. The game never changes clocks itself.
- An SD card (the game files are about 45 MB).
- The game files of **Death Rally 1.1 (1996)**.

---

## Installation

Everything goes into one folder, `sdmc:/switch/dRally/`:

```text
sdmc:/switch/dRally/
├── dRally.nro
├── ENGINE.BPA
├── IBFILES.BPA
├── MENU.BPA
├── MUSICS.BPA
├── TR0.BPA ... TR9.BPA
├── SANIM.HAF        optional: the intro animation
├── ENDANI.HAF       optional: the ending animations
└── ENDANI0.HAF
```

1. Copy `dRally.nro` (from the releases page) to `sdmc:/switch/dRally/`.
2. Copy the game files into the same folder. The names must be in upper case. On the original Death Rally CD they are in the `RALINST` folder (the `.BPA` files) and in `CINEM` (the `.HAF` files). You do not need `DR.IDF`, `RALLY.EXE`, `SETUP.EXE` or anything else.
3. Start **dRally** from the Homebrew Menu.

The game keeps its settings (`DR.CFG`), the settings of this port (`SWITCH.CFG`, written the first time) and `log.txt` in the same folder. `log.txt` is the file to attach to a bug report.

If a game file is missing or is not the original one, the game does not start; a text screen lists what is wrong (for example `MENU.BPA: not found`) and waits for a button. The game checks the exact size of each file, as the original does.

---

## Playing

The controller works as the keyboard the game was written for.

| Controller | In the menus | In a race |
| --- | --- | --- |
| D-pad | Move | Steer; up accelerates, down brakes |
| Left stick | Move | **Points where the car should go on the screen** |
| ZR / ZL | | Accelerate / brake |
| R | | Machine gun |
| L | | Drop mine |
| X | | Turbo boost |
| Y | | Horn |
| A | Confirm | |
| B | Back | |
| + | Back | **Pause** |
| - | Help | **Leave the race** (then Y confirms, A goes back) |
| Right stick click | | Switch the view |

- **Steering with the stick:** push the stick where you want to go, as seen on the screen (up: up the screen, right: to the right...). The car turns by the shortest way. Gas and brake stay on ZR and ZL. If you prefer a wheel, set `stick_mode = 0` in `SWITCH.CFG`.
- **Touch screen** (handheld mode): tap a menu row, drag the volume sliders, tap yes or no. A tap is Enter, a swipe is the arrow keys and a tap with two fingers is Back (and Pause in a race).
- **Names** (driver, saves): the console's on-screen keyboard opens by itself; click the left stick to open it again.
- **Vibration** when your car hits something, and for mines. Strength in `SWITCH.CFG`.

Details of all of this are in [SWITCH.md](SWITCH.md).

---

## Settings

The Configure menu of the game has two rows for the picture:

- **Smart Scaling**: the picture is enlarged by a whole number first, then fitted to the screen (sharper pixels).
- **Screen Mode**: 4:3 with bars, or 16:9 stretched over the whole screen.

The other settings are in `sdmc:/switch/dRally/SWITCH.CFG` (a text file, explained inside): vsync, vibration, stick behaviour. See [SWITCH.md](SWITCH.md#settings-file).

---

## Known limitations

- Network play is experimental and needs the `ipxnet` server on a computer (see [SWITCH.md](SWITCH.md#network-race-experimental)).
- The game's text entry only knows capitals, digits and a few signs; other characters from the on-screen keyboard are dropped.
- The menu entries for the PC joystick are switched off.
- The touch screen only works in the menus and does not exist when the console is docked.
- 16:9 stretches everything; the game was made for 4:3.

---

## Building from source

You need devkitPro with `switch-dev` and `switch-sdl2`:

```bash
dkp-pacman -S switch-dev switch-sdl2
make -f Makefile.switch        # builds dRally.nro
```

`tests/run.sh` builds and runs the host tests of the Switch code; a C compiler is enough. The Linux build is the plain `Makefile` (needs SDL2 and the original game files). The layout of the sources is in [SWITCH.md](SWITCH.md#repository-layout).

---

## Branches

- `master`: the Switch port (releases are made from here).
- `switch`: the branch new work is done in first; it is merged into `master` for releases.
- `upstream-master`: the repository as it was before the port (the fork of BSzili's dRally).

---

## Credits

dRally is by Neomalius (urxp) and contributors, under the MIT licence (see `LICENSE`); this port builds on [BSzili's fork](https://github.com/BSzili/dRally). Death Rally is a game by Remedy Entertainment and Apogee. This project is not affiliated with Remedy.

---

## Кратко по-русски

**Death Rally (1996) на Nintendo Switch.** Порт открытого движка dRally. В репозитории нет файлов игры: нужна ваша копия **Death Rally 1.1 (1996)**; ремейк 2012 года из Steam не подходит.

- **Разгон не требуется:** игра лёгкая и работает на штатных частотах. При желании можно, наоборот, поставить частоты ниже, чтобы экономить заряд (насколько низко можно, не измерялось).
- **Установка:** положите `dRally.nro` и файлы игры (`ENGINE.BPA`, `IBFILES.BPA`, `MENU.BPA`, `MUSICS.BPA`, `TR0.BPA`…`TR9.BPA`, по желанию `SANIM.HAF`, `ENDANI.HAF`, `ENDANI0.HAF`) в одну папку `sdmc:/switch/dRally/`. Имена в верхнем регистре. На оригинальном диске `.BPA` лежат в `RALINST`, `.HAF` в `CINEM`.
- **Управление:** левый стик показывает, куда ехать на экране; ZR газ, ZL тормоз, + пауза, − выйти из гонки (Y подтвердить). Вибрация, тач в меню, системная клавиатура для имён.
- **Картинка:** в меню Configure есть «Smart Scaling» (чёткие пиксели) и «Screen Mode» (4:3 или растянуть до 16:9).
- Если файл не тот или отсутствует, на текстовом экране будет список. При проблемах приложите `log.txt` из папки игры.
