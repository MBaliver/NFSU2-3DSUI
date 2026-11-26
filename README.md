# Need for Speed: Underground 2 for the New 3DS — UI/UX Fork

> 🌐 **Language / Idioma:** **English** | **[🇧🇷 Português (Brasil)](README.pt-BR.md)**

A UX/UI-focused fork of [Octoblimp/NFSU2-3DS](https://github.com/Octoblimp/NFSU2-3DS) (based on upstream `v1.0.1`): an unofficial native homebrew port of the PC version of **Need for Speed: Underground 2** to the **New Nintendo 3DS / New 3DS XL / New 2DS XL**.

This is the real PC game itself — not a remake. Its engine was reverse-engineered from `SPEED2.EXE` and reimplemented in C (with the GameCube release as a console reference). It plays using data converted from **your own PC copy** of the game; **no copyrighted game data is included** in this repository or in the releases.

What you get is the full PC game — Bayview free roam, Career mode, shops, sponsors, magazine & DVD covers, SMS messages, world map, every race type (Circuit, Sprint, Drift, Drag, Street X, URL, Outrun), Quick Race, car customization, dyno & tuning, movies, EA TRAX, saves, cheat codes, and Local Wireless multiplayer for up to 6 consoles — **upgraded with a redesigned 3DS-native touch interface, 8-language support, accessibility options, motion steering, and quality-of-life improvements.**

> **Already playing Octoblimp's NFSU2-3DS?** Your existing `sdmc:/nfsu2` game data folder and career saves are **100% compatible**. Just install this fork's `NFSU2-3DS.cia` with FBI — your profiles and saves remain untouched!

---

## Table of Contents

- [What's New in This Fork](#whats-new-in-this-fork)
- [Requirements](#requirements)
- [How to Play (Installation)](#how-to-play-installation)
  - [Upgrading from Upstream NFSU2-3DS](#upgrading-from-upstream-nfsu2-3ds)
  - [First-Time Setup (Windows GUI)](#first-time-setup-windows-gui)
  - [First-Time Setup (Linux / macOS / Python CLI)](#first-time-setup-linux--macos--python-cli)
  - [Regional Cars & Audio Notes](#regional-cars--audio-notes)
- [Controls & Fork Options](#controls--fork-options)
- [Local Wireless, Cheats & Mods](#local-wireless-cheats--mods)
- [Configuration File (`nfsu2_config.txt`)](#configuration-file-nfsu2_configtxt)
- [Regions](#regions)
- [Building from Source](#building-from-source)
  - [Prerequisites](#prerequisites)
  - [Building on Linux](#building-on-linux)
  - [Building on Windows](#building-on-windows)
  - [Building the PC Data Tools (Optional)](#building-the-pc-data-tools-optional)
  - [Repository Layout](#repository-layout)
- [Reporting a Bug](#reporting-a-bug)
- [About, Credits & License](#about-credits--license)

---

## What's New in This Fork

Everything below lives inside the 3DS executable while keeping full compatibility with retail save files and upstream game data (see [docs/bottom-screen-ui.md](docs/bottom-screen-ui.md) for the full technical write-up):

- **3DS-Native Lower-Screen UI Kit (`U2-UI`)**
  - ~90 frontend and in-world screens rebuilt for the 320×240 touch screen using NFSU2's own palette and UI language, antialiased vector shapes, minimum 32 px touch targets, readable font floors (12 px labels / 14–16 px body), touch sound feedback, draggable scrollbars, scrollable long dialogs, and direct-tap selection.
  - Covers icon hubs (2×2 / hierarchical grids), Car Select (Garage, Car Lot, Quick Race, LAN, Customize), Body Shop & Performance parts/brands, Paint swatches, Dyno & Tuning graphs, EA Trax jukebox, LAN screens, Mailbox & SMS reader, Career Status, Help & Engage dialogs.
- **Reworked Free Roam & Race Touch HUD**
  - Full-bleed pannable and zoomable touch minimap (`+`/`−` zoom, touch drag, `RECENTER` chip).
  - **Live GPS road route** drawn directly on the minimap with **road-following distance** (not straight-line) and target-colored GPS indicators, rings, and destination strip.
  - Tap any map marker for a detail card + one-tap **SET GPS**; tap the bank or SMS chip to jump straight to Career Status or Mailbox; contextual **Enter / Challenge** touch pill; optional **Touch Camera** button.
  - Unified race bottom UI with floating position/lap/time chips, checkered finish flag, and an expandable **Standings** bar (toggle via D-Pad Down or touch).
- **World Map Overhaul**
  - Full-screen pannable/zoomable map with HUD-matching icons, live free-roam AI racers, direct marker tap, and a scrollable filter drawer.
- **Streamlined Title, Loading, Pause, Post-Race & Replay**
  - Title screen **Resume Career** profile card (jumps straight into free roam without flashing the main menu; single press skips the intro).
  - Faster world-entry loading (primes only nearby track sections) with a contextual bottom-screen loading panel showing controls and game tips.
  - Redesigned bottom-screen **Pause** menu (with live map & status summary on the top screen), navigable **Post-Race Results & Stats**, and a full touch **Replay Deck** with scrubbable timeline and transport buttons.
- **8 Languages + Accessibility (*Options > Accessibility*)**
  - Play in **English, Français, Deutsch, Italiano, Español, Nederlands, Svenska, or Dansk** — covering both the game's `LANGUAGES/<Lang>.bin` tables and all custom port UI strings/tips (including a rendering fix for French no-break spaces).
  - **Large Text** (~1.25×) and **High Contrast** modes, stored per-profile in a separate `port.txt` file so your retail career saves stay byte-for-byte compatible.
- **New 3DS Controls & Quality-of-Life**
  - **Motion Steering** (*Options > Player*): tilt the 3DS to steer while driving using the built-in accelerometer.
  - **Drag Shift: ZL/ZR** (*Options > Gameplay*): shift up with `ZR` and down with `ZL` in Drag races.
  - **Touch Camera** (*Options > Gameplay*): optional camera-cycle touch button in the top-right corner of the bottom HUD.
  - **C-Stick Free Look** in chase views (in addition to hydraulics, dyno camera, and map panning).
- **Modern Toolchain & Linux Build Support**
  - Includes a native Linux build script (`tools/build_cia.sh`), runtime `citro2d` 1.6 / 1.7+ text measurement detection, and `libctru` 2.7.1 compatibility.

---

## Requirements

- A **New Nintendo 3DS, New 3DS XL, or New 2DS XL** with Custom Firmware (**Luma3DS**) and **FBI**.
  - *Original Old 3DS / Old 3DS XL / Old 2DS consoles cannot run this port* (not enough RAM or CPU speed).
- **DSP Firmware** dumped for audio:
  - Press `L + D-Pad Down + SELECT` to open the **Rosalina** menu → **Miscellaneous options** → **Dump DSP firmware**.
  - This creates `sdmc:/3ds/dspfirm.cdc` (for `.3dsx`) or `sdmc:/dspfirm.cdc` (for `.cia` — if you only have it in `/3ds/dspfirm.cdc`, copy it to the SD root `sdmc:/dspfirm.cdc` as well).
- **Your own PC copy** of *Need for Speed: Underground 2* (the folder containing `SPEED2.EXE`).
  - Tested with the **US version patched to v1.2**. Other versions/patches are not guaranteed to work.
- About **2.6 GB** of free space on your SD card.
- A PC (Windows, Linux, or macOS) to convert your PC game files once for the SD card.

---

## How to Play (Installation)

### Upgrading from Upstream NFSU2-3DS

If you already have a working `sdmc:/nfsu2` folder from [Octoblimp/NFSU2-3DS](https://github.com/Octoblimp/NFSU2-3DS):

1. Download the latest **`NFSU2-3DS.cia`** from this repository's [**Releases**](https://github.com/MBaliver/NFSU2-3DSUI/releases) page.
2. Copy `NFSU2-3DS.cia` to your 3DS SD card and install it with **FBI** (it overwrites the app cleanly while preserving all your saves and data).
3. *(Optional — for non-English languages)* Copy any extra language tables you want (`French.bin`, `German.bin`, `Italian.bin`, `Spanish.bin`, `Dutch.bin`, `Swedish.bin`, `Danish.bin`) from your PC game's `LANGUAGES/` folder into `sdmc:/nfsu2/LANGUAGES/`, then select your language in **Options > Accessibility > Language**.
4. Launch **Need for Speed: Underground 2** from the HOME Menu!

### First-Time Setup (Windows GUI)

1. **Convert your PC game files to the SD card:**
   - **Recommended:** Download `NFSU2-3DS-Tools.zip` from [Octoblimp's v1.0.1 release](https://github.com/Octoblimp/NFSU2-3DS/releases) (it bundles `ffmpeg.exe`, `vgmstream`, `dspenc`, and `tex3ds` — nothing to install), **or** clone this repository with **Git LFS** installed and open [`PC Apps/NFSU2-3DS Tools`](PC%20Apps/NFSU2-3DS%20Tools).
     > **Note about GitHub "Download ZIP":** `ffmpeg.exe` is stored via Git LFS, so GitHub's source ZIP only contains a small pointer file instead of the real binary. Use the Tools zip, clone with Git LFS, or have `ffmpeg` (with `libx264`) on your `PATH`.
   - Open **NFSU2-3DS Data Setup** (`NFSU2-3DS Data Setup.exe`).
   - Select your PC NFSU2 installation folder (where `SPEED2.EXE` lives).
   - Set the output game data folder to `nfsu2` at the **root of your SD card** (for example, `E:\nfsu2`).
   - Click **Set up / update**. The first conversion takes a while; subsequent runs only update missing or changed files.
   - *(Optional — extra languages)* If you used the upstream Tools zip and want languages other than English, copy the desired `.bin` files (`French.bin`, `Spanish.bin`, etc.) from your PC game's `LANGUAGES\` folder into `<SD>:\nfsu2\LANGUAGES\`. (Running this fork's `stage.py` does this automatically.)
   - *Windows CLI alternative:*
     ```cmd
     nfsu2-3ds.exe setup --pc "C:\Path\To\NFSU2" --gamedir E:\nfsu2
     nfsu2-3ds.exe setup --pc "C:\Path\To\NFSU2" --gamedir E:\nfsu2 --list
     ```
2. **Install the game on your New 3DS:**
   - Download **`NFSU2-3DS.cia`** from this repository's [**Releases**](https://github.com/MBaliver/NFSU2-3DSUI/releases) page (or [build it yourself](#building-from-source)).
   - Copy `NFSU2-3DS.cia` to your SD card, install it using **FBI**, and start it from the **HOME Menu**.

> **CIA vs `.3dsx`:** The **`.cia`** build is strongly recommended and tested on real hardware — only the CIA gets the New 3DS's 178 MB extended memory mode. On first boot, the CIA automatically creates `sdmc:/nfsu2_config.txt` pointing to `/nfsu2`. The `.3dsx` runs from the Homebrew Launcher using `/3ds/nfsu2`.

> **Saves & Missing-File Checks:** Profiles and career saves live inside `sdmc:/nfsu2/SAVE/` and `sdmc:/nfsu2/profiles/`. Data Setup never touches them. If something is missing (config file, `GameDir`, or DSP firmware), the game shows a clear diagnostic screen naming the exact path it checked instead of booting to a black screen.

### First-Time Setup (Linux / macOS / Python CLI)

You can convert your PC game data on any OS using the Python tools in `tools/`:

1. Install **Python 3** and the required packages:
   ```bash
   pip install numpy Pillow lz4 pefile unicorn capstone
   ```
2. Install external tools on your `PATH`:
   - `ffmpeg` (compiled with `libx264`)
   - `vgmstream-cli`
   - `dspenc` (source is included in `PC Apps/NFSU2-3DS Tools/bin/dspenc.c`)
   - devkitPro's `tex3ds` (`sudo dkp-pacman -S 3dstools` on Linux/macOS)
3. Run the staging script (this also automatically copies all available Latin-script language files!):
   ```bash
   python3 tools/stage.py --pc "/path/to/NFSU2" --gamedir "/path/to/sd/nfsu2"
   ```
   Or launch the graphical window with `python3 tools/nfsu2_setup.pyw`.

### Regional Cars & Audio Notes

- **Peugeot 106 & Vauxhall/Opel Corsa:** These two cars only shipped with the UK/EU PC release (US installs have `CIVIC` and `RSX` instead). Data Setup packs whichever cars exist in your `CARS\` folder and skips missing ones.
  - To add the 106 and Corsa to a US install, copy `CARS\PEUGOT106` and `CARS\CORSA` from a UK/EU copy into your PC install's `CARS` folder before running Data Setup (or re-run Data Setup afterward), then set [`Region=`](#regions) accordingly (`EU`, `ALL`, etc.). Without those folders, keep `Region=US`.
  - Similarly, on a UK/EU install without `CARS\CIVIC` and `CARS\RSX`, set `Region=EU` or copy those two folders from a US copy.
- **Music & Speech in `SDATA\sdat.viv`:** Some PC installs store music and speech inside `SDATA\sdat.viv` instead of unpacked `pfdata` / `speech` folders. Data Setup automatically unpacks it (to a temporary cache, never modifying your PC install).

---

## Controls & Fork Options

### Driving

| Control | Action |
| --- | --- |
| **Circle Pad** *(or 3DS Tilt if Motion Steering is On)* | Steer |
| **A** | Accelerate |
| **B** | Brake / Reverse |
| **R** | Nitrous |
| **L** | Handbrake |
| **X** | Shift up |
| **Y** | Shift down |
| **ZR** | Change camera *(or Shift up in Drag races when **Drag Shift: ZL/ZR** is On)* |
| **ZL** | Reset car *(or Shift down in Drag races when **Drag Shift: ZL/ZR** is On)* |
| **C-Stick** | Free look camera (chase views) / Hydraulics / Pan map |
| **SELECT** | Look back |
| **D-Pad Up** | Enter shop / Challenge opponent |
| **D-Pad Down** | World Map (free roam) / Toggle Standings (in races) |
| **D-Pad Left** | Career Statistics |
| **D-Pad Right** | SMS Messages |
| **START** | Pause |

Every driving action except steering and pause can be remapped per profile in **Options > Controls** (**Default Settings** restores the table above; **Delete All Controls** clears all bindings).

### Menus & Touch Screen

- Navigate with **D-Pad** or **Circle Pad**, confirm with **A**, go back with **B**, and scroll lists or long dialogs by dragging or using **Up/Down**.
- Every reworked bottom-screen menu supports **direct touch**, including the free-roam minimap (drag to pan, `+`/`−` to zoom, tap markers to set GPS), World Map, Mailbox, Pause, Post-Race, and Replay Deck.

### Extra Options Added by This Fork

- **Options > Accessibility:** `Language` (8 languages), `Large Text`, `High Contrast`.
- **Options > Gameplay:** `GPS Color`, `Drag Shift: ZL/ZR`, `Touch Camera`, `Car Damage`.
- **Options > Player:** `Motion Steering` (3DS accelerometer tilt steering while driving).

---

## Local Wireless, Cheats & Mods

- **Local Wireless (LAN):** Choose **LAN** on the main menu to race **2 to 6 New 3DS consoles** in any race type. Each console needs the game and its own SD game data folder built with the same tool version. One console hosts; the others join.
- **Cheat Codes:** All PC cheat codes work on the title screen using 3DS button sequences. See [CHEAT_CODES.MD](CHEAT_CODES.MD).
- **Mods:** Most PC data mods (car models/textures, vinyls, HUD/menu art, text, audio, music) work via **NFSU2-3DS Mod Packer** into `<GameDir>/mods/`. Code mods (`.asi`, `SPEED2.EXE` patches) cannot run on 3DS. See [MODDING.md](MODDING.md).

---

## Configuration File (`nfsu2_config.txt`)

Located at `sdmc:/nfsu2_config.txt` (or beside the `.3dsx`). Uses `Key=Value` lines (`#` starts a comment). Created automatically on first boot and never overwritten afterwards.

| Key | Description |
| --- | --- |
| `GameDir=/nfsu2` | Path to the game data folder on the SD card (**required**). |
| `Language=EN` | Boot language: `EN`, `FR`, `DE`, `IT`, `ES`, `NL`, `SV`, `DA` (or full name). Requires the matching `LANGUAGES/<Name>.bin` in `GameDir`. Can also be changed per profile in *Options > Accessibility*. |
| `Region=US` | Regional variant (`US`, `EU`, `KR`, `JP`, `TW`, `CN`, `TH`, or `ALL`). See [Regions](#regions). |
| `Mods=CarPack,HudSkin` | Comma-separated mod load order from `mods/` (`Mods=off` disables all; omitted loads all). |
| `HwVideo=0` | Decode movies on CPU (software VP6) instead of New 3DS MVD hardware. |
| `DebugLogging=1` | Write diagnostic log to `sdmc:/nfsu2_debug.txt` (off by default). |
| `DevCommands=1` | *(Developer)* Poll `sdmc:/nfsu2_cmd.txt` for scripted input/screenshots (`tools/devcmd.py`) and enable Debug Tools on `L + R + SELECT`. |
| `DbgUiAudit=1` | *(Developer)* Log UI touch targets under 32 px and text under 14 px. |

---

## Regions

The PC release defaults to `US`, while all regional differences exist in the engine:

| `Region=` | Version |
| --- | --- |
| `US` (`NA`) | North America (default) |
| `EU` (`EUROPE`, `UK`) | Europe |
| `KR` (`KOREA`) | Korea |
| `JP` (`JAPAN`) | Japan |
| `TW` (`TAIWAN`) | Taiwan |
| `CN` (`CHINA`) | China |
| `TH` (`THAILAND`) | Thailand |
| `ALL` | All 31 cars combined (US badges + EU 106/Corsa + Best Buy vinyl) |

- **Cars:** `US` includes the Honda Civic and Acura RSX; other regions replace them with the Peugeot 106 and Corsa (when packed from a PC copy that contains them). `ALL` includes all 31 cars.
- **Badges & Logos:** Region-accurate car names and badges (e.g., Miata / MX-5 / Miata Roadster, Tiburon / Coupe / Tuscani, Vauxhall / Opel Corsa, Sprinter Trueno, Altezza, Fairlady Z, Skyline).
- **Best Buy Vinyl:** Present in `US` and `ALL`.

---

## Building from Source

### Prerequisites

- **devkitPro 3DS toolchain:**
  - `3ds-dev` (`devkitARM`, `libctru`, `citro2d`, `citro3d`, `3dstools`, `picasso`)
  - `3ds-zlib` (`libz`)
- **Python 3** *(recommended)* — runs `tools/pica_output_check.py` (catches PICA200 shader issues that hang real hardware) and optionally regenerates the HOME Menu banner (`tools/make_banner.py`). Without Python or PC data, the committed banner in `3DS Files/meta/nfsu2/` is used.
- **Toolchain version note:** Upstream originally tested with `libctru 2.4.1`, `citro2d 1.6.0`, and `citro3d 1.7.1` (and `build_cia.ps1` warns if another Windows libctru is installed). This fork adds runtime `citro2d 1.7.x` text measurement compatibility and `libctru 2.7.1` support, and its releases are built and tested on Linux with modern devkitPro (`libctru 2.7.1` + `citro2d 1.7.x`).

### Building on Linux

1. **Install devkitPro 3DS packages** (via [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman)):
   ```bash
   sudo dkp-pacman -S 3ds-dev 3ds-zlib
   ```
2. **Install Linux `makerom` and `bannertool`:**
   `makerom` and `bannertool` are not shipped in devkitPro's repos (the repository tracks the Windows `.exe` builds in `tools/3ds-bin/`). Download Linux x86_64 binaries for:
   - `makerom` from [3DSGuy/Project_CTR releases](https://github.com/3DSGuy/Project_CTR/releases)
   - `bannertool` from [Steveice10/bannertool releases](https://github.com/Steveice10/bannertool/releases)

   Place both binaries in `tools/3ds-bin/` and make them executable:
   ```bash
   chmod +x tools/3ds-bin/makerom tools/3ds-bin/bannertool
   ```
3. **Build `.3dsx` and `.cia`:**
   Because GNU Make cannot run devkitPro's `3ds_rules` inside a path with spaces (`3DS Files`), `tools/build_cia.sh` automatically bind-mounts `3DS Files/` onto `build_3ds/` inside an unprivileged mount namespace (`unshare -rm`) and runs `make -j$(nproc) cia`:
   ```bash
   # Standard system-wide devkitPro install (/opt/devkitpro):
   DEVKITPRO=/opt/devkitpro bash tools/build_cia.sh

   # Or if devkitPro lives at ~/.local/share/devkitpro/opt/devkitpro (script default):
   bash tools/build_cia.sh
   ```
4. **Outputs:**
   - `3DS Files/NFSU2-3DS.cia`
   - `3DS Files/NFSU2-3DS.3dsx`
   - `3DS Files/NFSU2-3DS.elf`

*(Tip: You can deploy and install the freshly built `.cia` straight to a New 3DS over Wi-Fi using **3dsDL** — see [docs/bottom-screen-ui.md](docs/bottom-screen-ui.md#1-build-and-deploy-on-linux) for the `curl` snippet.)*

### Building on Windows

1. Install **devkitPro** using the [devkitPro Installer](https://github.com/devkitPro/installer/releases) (check **3DS Development**), and ensure `3ds-zlib` is installed inside devkitPro MSYS2:
   ```bash
   pacman -S 3ds-dev 3ds-zlib
   ```
2. Windows binaries (`makerom.exe` and `bannertool.exe`) are already included in `tools/3ds-bin/`.
3. From the repository root in **PowerShell**:
   ```powershell
   powershell -ExecutionPolicy Bypass -File tools/build_cia.ps1
   ```
   The script maps a temporary space-free drive letter (`subst`), checks shaders, builds `3DS Files/NFSU2-3DS.3dsx` and `3DS Files/NFSU2-3DS.cia` inside devkitPro MSYS2 bash, and unmaps the drive automatically when finished.

### Building the PC Data Tools (Optional)

To rebuild the standalone Windows executables in `PC Apps/NFSU2-3DS Tools/` from the Python scripts in `tools/`:

```bash
python tools/build_apps.py
```

### Repository Layout

- `3DS Files/` — devkitARM project: `source/` (port C/C++ code & PICA shaders), `data/` (embedded fonts & 3DS button glyphs), `meta/` (CIA banner, icon, and `app.rsf`).
- `tools/` — build scripts (`build_cia.sh`, `build_cia.ps1`), Python staging/packing scripts (`stage.py`, `devcmd.py`, etc.), Ghidra scripts, and `tools/3ds-bin/` (`makerom` / `bannertool`).
- `PC Apps/NFSU2-3DS Tools/` — prebuilt Windows Data Setup & Mod Packer apps, source, and bundled tools (`ffmpeg`, `vgmstream`, `dspenc`, `tex3ds`) with their licenses.
- `docs/` — technical write-ups of the fork's UI rework (`docs/bottom-screen-ui.md`), file formats, and retail behavior.

---

## Reporting a Bug

Logs are disabled by default. To report a bug:

1. Add `DebugLogging=1` to `sdmc:/nfsu2_config.txt`.
2. Reproduce the issue.
3. Share `sdmc:/nfsu2_debug.txt` along with any Luma3DS crash dump (if the console crashed) and note which release/commit you are running.

---

## About, Credits & License

- **Original Port:** Created by [**Octoblimp**](https://github.com/Octoblimp/NFSU2-3DS).
- **UI/UX Fork:** Maintained by [**MBaliver**](https://github.com/MBaliver/NFSU2-3DSUI).
- **Disclaimer:** This is a non-commercial fan project, not made, endorsed, or supported by Electronic Arts. *Need for Speed* and *Need for Speed: Underground 2* are trademarks of Electronic Arts Inc. This repository contains no copyrighted game data and requires files from a copy you own (the sole exception is the CIA HOME Menu banner, icon, and sound in `3DS Files/meta/nfsu2/` generated by `tools/make_banner.py` so the CIA builds standalone).
- **License:** The port's source code is licensed under the **GNU General Public License v3.0** ([LICENSE](LICENSE)). The game, its data, and its artwork remain property of Electronic Arts. Bundled third-party binaries (`makerom`, `bannertool`, `ffmpeg`, `vgmstream`, `dspenc`, `tex3ds`) keep their respective licenses.

### AI Disclosure

- **Upstream port (Octoblimp):** Claude Opus 5.5 and Claude Fable 5.2 were used to help decompile the PC game, port it to the 3DS, and debug in Azahar, with real-hardware testing on a New Nintendo 3DS XL.
- **UI/UX fork (MBaliver):** Claude Opus 5.5 assisted in implementing and refining the redesigned lower-screen UI kit, localization, accessibility, and touch/motion controls alongside human UX design, emulator verification (Azahar), and real-hardware testing on New 3DS.
