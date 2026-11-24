#pragma once

// PORT (development aid, off unless DevCommands=1 in nfsu2_config.txt): a
// command file the game polls while it runs, so the PC can drive it --
// press buttons, tap and drag the touch screen, move the sticks -- and take
// screenshots of both screens, without anyone at the console. Meant for
// Azahar, whose SD card is a plain folder on the PC, but it works the same on
// hardware over FTP or 3dsDL.
//
// The game checks for sdmc:/nfsu2_cmd.txt about four times a second while no
// script is running. It reads the file, deletes it and runs its lines in
// order, one input step per hidScanInput() (one frame). When the last line is
// done it writes sdmc:/nfsu2_cmd_done.txt ("done <lines> <ms>" plus a line
// for each it could not read), so the PC knows when to look.
//
// One command a line; '#' starts a comment. Frames are scans (the game's
// frame rate), waits are milliseconds.
//
//   press KEY[+KEY..] [frames]   held that long (default 6), then let go
//   hold  KEY[+KEY..] frames     the same, for a long hold
//   tap   x y [frames]           a touch at (x, y), 0..319 x 0..239 (default 6)
//   drag  x1 y1 x2 y2 [frames]   a touch moved in a straight line (default 20)
//   stick dx dy frames           the Circle Pad (-156..156 each way)
//   cstick dx dy frames          the C-Stick (New 3DS)
//   wait  ms                     nothing pressed for that long
//   shot  name                   sdmc:/nfsu2_shots/<name>_top.bmp and
//                                <name>_bottom.bmp, as the screens show them
//                                (the back buffer: one frame old)
//   log   text                   a line in the debug log
//
// KEY names: A B X Y L R ZL ZR START SELECT UP DOWN LEFT RIGHT.
//
// main.c routes its hid* calls through here (DEVCMD_WRAP_HID); with the
// commands off the wrappers return libctru's own reads unchanged.

#include <3ds.h>
#include <stdbool.h>

void devcmd_init(bool enabled, void (*log)(const char*));
bool devcmd_enabled(void);
// Injecting this frame: main.c's own direct pad read (worldSimPad) stands
// down so the injected buttons and stick are what the sim sees.
bool devcmd_active(void);

void devcmd_hidScanInput(void);
u32  devcmd_hidKeysDown(void);
u32  devcmd_hidKeysHeld(void);
u32  devcmd_hidKeysUp(void);
void devcmd_hidTouchRead(touchPosition* pos);
void devcmd_hidCircleRead(circlePosition* pos);
void devcmd_hidCstickRead(circlePosition* pos);

#ifdef DEVCMD_WRAP_HID
#define hidScanInput  devcmd_hidScanInput
#define hidKeysDown   devcmd_hidKeysDown
#define hidKeysHeld   devcmd_hidKeysHeld
#define hidKeysUp     devcmd_hidKeysUp
#define hidTouchRead  devcmd_hidTouchRead
#define hidCircleRead devcmd_hidCircleRead
#undef  hidCstickRead   // libctru's irrst.h: an alias of irrstCstickRead
#define hidCstickRead devcmd_hidCstickRead
#endif
