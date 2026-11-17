# dRally for Nintendo Switch

A port of [dRally](https://github.com/BSzili/dRally), an open source engine for **Death Rally (1996)**, to the Nintendo Switch homebrew platform (devkitA64 / libnx / SDL2).

> [!IMPORTANT]
> This repository contains **no Death Rally game data**. You need your own copy of the **1996 game** (version 1.1, the DOS release, for example from the original CD) and you copy its files to your own console. The 2012 remake sold on Steam is a different game and does not work.

> [!NOTE]
> **Status: 1.2.0.** Tested in handheld mode (menus, races, steering, vibration and 16:9), including two-player split-screen with external controllers on Switch Lite. See [Reporting problems](#reporting-problems).

## Requirements

- A Nintendo Switch with custom firmware that runs homebrew (for example Atmosphère) and the Homebrew Menu.
- Death Rally game files (see below). No overclocking is needed; lower clocks can be set to save battery (a tool such as Horizon OC or sys-clk; the game never changes clocks itself).

## Installation

```text
sdmc:/switch/dRally/
├── dRally.nro
├── ENGINE.BPA
├── IBFILES.BPA
├── MENU.BPA
├── MUSICS.BPA
├── TR0.BPA ... TR9.BPA
├── SANIM.HAF              optional: the intro animation
├── ENDANI.HAF             optional: the ending animations
└── ENDANI0.HAF
```

1. Copy `dRally.nro` to `sdmc:/switch/dRally/`.
2. Copy the game files from your Death Rally 1.1 (1996) into the same folder. The names must be in upper case. On the original CD the `.BPA` files are in `RALINST` and the `.HAF` files in `CINEM`; `DR.IDF` and the rest are not needed. The 2012 remake is a different game and does not work.
3. Start **dRally** from the Homebrew Menu.

(The original layout, with a `CINEM/` folder and a `CDROM.INI` that holds `./CINEM`, also works.)

The game keeps its settings (`DR.CFG`), the settings of this port (`SWITCH.CFG`, written the first time) and a `log.txt` in the same folder. `log.txt` is the file to attach to a bug report.

If a game file is missing or is not the original one, the game does not start; a text screen lists what is wrong (for example `MENU.BPA: not found`) and waits for a button. The game checks the exact size of each file, as the original does, so the files must be those of Death Rally 1.1.

## Controls

The controller works as the keyboard the game was written for. The game steers, brakes and accelerates with on/off keys (there is no analog steering), and the Up and Down arrows always accelerate and brake, so the stick or D-pad alone is enough to drive.

| Controller | In the menus | In a race |
| --- | --- | --- |
| D-pad | Move | Steer (on or off); up accelerates, down brakes |
| Left stick | Move | Points where the car should go (`stick_mode = 1`), or turns it left and right (`stick_mode = 0`) |
| Left stick click | (on a name screen: the keyboard) | Nothing |
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

### Prompts

Prompts that named a key ("press any key", "press Enter") name the controller button here ("Press A button to continue"); the pause and race-over messages and the first-race help show Switch buttons, and the abort-race question is answered with **Y** (abort) or **A** (continue).

### Steering

The game itself only knows steer left, steer right (on or off, once per frame). The port adds:

- **Direction steering with the left stick** (on by default, `stick_mode = 1`): in a race the stick points where the car should go *on the screen*: stick up and the car turns until it heads up the screen, stick right and it heads right, and so on, by the shortest way round. The further the car is from the wanted heading, the more often it steers (small corrections steer only some frames). The stick does not accelerate or brake: gas and brake stay on **ZR** and **ZL**. In the dead zone it asks for nothing. Set `stick_mode = 0` for the classic behaviour below.
- **Fine steering with the left stick** (`stick_mode = 0`): how far the stick is pushed decides how often the steering flag is on; half way is every second frame, full is every frame. The D-pad is still on or off. In a race the stick only accelerates or brakes when it is pushed clearly up or down (not when steering diagonally). Set `analog_steering = 0` in `SWITCH.CFG` for the old behaviour.

### Vibration

In a race the controller vibrates when your car hits something (harder for a harder hit), when you drop a mine and when a mine goes off (nearer is stronger). `rumble` and `rumble_percent` in `SWITCH.CFG` switch it off or change how strong it is.

### Picture timing

The picture waits for the screen (vsync, no tearing) while the game keeps its own clock of 70 frames a second. If the game runs too slowly or stutters, set `vsync = 0` in `SWITCH.CFG`. `log.txt` says whether vsync is on (`[dRally.DISPLAY]`).

### Settings file

`SWITCH.CFG` is a text file with one `name = value` per line, explained in the file itself:

| Setting | Default | Meaning |
| --- | --- | --- |
| `vsync` | 1 | the picture waits for the screen |
| `ipx_server`, `ipx_port` | empty, 8888 | address and UDP port of the ipxnet server for the network race (empty: network off) |
| `widescreen` | 0 | 1: stretch the picture over the whole 16:9 screen (also in the game's Configure menu, "Screen Mode") |
| `scaling` | 0 | 0: smooth. 1: smart, whole-number enlargement first (sharp pixels), then fitted to the screen (also in the Configure menu, "Smart Scaling") |
| `rumble`, `rumble_percent` | 1, 100 | vibration in races and how strong |
| `analog_steering` | 1 | fine steering with the left stick |
| `stick_deadzone` | 15 | percent of the stick's travel that does nothing |
| `stick_mode` | 1 | 1: the stick points where the car should go on the screen; 0: it turns the car left and right |
| `stick_invert_x` | 0 | 1 swaps left and right of the left stick |

### Local Split-Screen

Open **Local Split-Screen** from the main menu and register 2–4 external
controllers in the Switch controller screen. The applet's player slots set the
race roster. Press **+** once all registered controllers are connected.
Left/Right picks the track, Up/Down picks the shared car model, and **Y** toggles
computer drivers.

Each view follows its own car. The left stick or D-pad steers, **ZR** accelerates,
**ZL** brakes, **X** boosts, **R** fires and **L** drops a mine. Player 1 controls
pause (**+**) and the leave prompt (**−**). With bots enabled, computer drivers
fill the remaining spots. Two players use the left and right halves of the
screen. Three or four players use a 2×2 grid. The quick race uses its own roster
and does not advance the career.

### Network race (experimental)

The game plays over the network with the IPX protocol of old local networks. dRally sends the IPX packets inside UDP to one small server, which hands them on to the other players. Every player connects to the same server, which runs on a computer in your network (not on the console).

1. **Start the server** on a computer in the same network: `python3 ipxnet/ipxnet.py 8888` (only Python 3 is needed; the port is the one the Switch uses by default). `ipxnet/ipxnet.c` is the same server in C and needs SDL2 and SDL2_net. Allow UDP port 8888 in the computer's firewall.
2. **Tell the Switch where it is:** when you choose **Network connection** and have entered your name, the console's keyboard asks for the server's address (for example `192.168.1.20`); the last one is already in the box and is kept in `SWITCH.CFG` (`ipx_server`; `ipx_port` is the port, 8888 by default). Cancelling the keyboard leaves the network.
3. **Play:** every player chooses **Multiplayer Race** in the main menu. The server prints a line for each player that connects, and `log.txt` has `[dRally.Network]` lines.

Players on a PC use the Linux build of dRally, which connects to `localhost:8888` (the source says so in `src/network/network.c`) and so needs the server on the same computer or a change there. The Switch needs a Wi-Fi connection to the same network.

Not tried yet: a race with two players. The network code comes from the original project, which lists multiplayer as not working, so expect problems and send `log.txt` with a description.

**A crash seen once (cause not known).** Right after "Connection established!" the game crashed once in the graphics driver while it uploaded the picture (`nvMapCreate` in the Atmosphère crash report, module `dRally`: a memory allocation that failed or returned nothing, and the driver did not check it). A later run did not crash and its log showed plenty of memory (title takeover mode: about 3 GB for the process, 166 MB of heap in use), so a lack of memory is not confirmed. Start the Homebrew Menu by holding **R** (title takeover, as for Quake 4) rather than through the Album. If you see the crash, send the crash report from `sdmc:/atmosphere/crash_reports/` together with `log.txt`.

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

- The network race (see above) is new and has not been tried with two players yet; one crash right after connecting has been seen once (see the note there).
- The game's own text entry only knows capitals, digits and a few signs, so other characters from the on-screen keyboard are dropped.
- The menu entries for the PC joystick (Define Gamepad/Joystick, Gamepad/Joystick) are switched off on the Switch: the console controller is always active.
- Not tested yet: docked mode, touch in the 16:9 mode, controllers other than the built-in one.

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
| `tools/` | helper scripts; `ipxnet/` is the IPX server (`ipxnet.py` needs only Python 3); `tests/` are host tests of the Switch code |

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
- `src/platform/switch/switch_steer.c`: stick steering (how far it is pushed, or where it points, as how often the flag is on); `race___40164h.c` adds it to the game's own steering flags.
- `src/platform/switch/switch_rumble.c`: vibration, called from the race code for the player's car (`race_main.c`, `race___514d8h.c`).

All Switch code is under `DR_SWITCH`; the Linux build behaves as before.

## Reporting problems

Open an issue and attach `sdmc:/switch/dRally/log.txt` and, for crashes, the newest file from `sdmc:/atmosphere/crash_reports/`. Say which console you use (Switch, Lite, OLED), handheld or docked, and what you were doing.

## Credits and licence

dRally is by Neomalius (urxp) and contributors, under the MIT licence (see `LICENSE`); this port builds on [BSzili's fork](https://github.com/BSzili/dRally). Death Rally is a game by Remedy Entertainment and Apogee. This project is not affiliated with Remedy.
