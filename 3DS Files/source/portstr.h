#pragma once

// PORT: the port's own on-screen words in the player's language.
//
// Every string the port draws that is not one of the game's .fng texts goes
// through here: portstr(PS_x) returns it as UTF-8 for the language in use.
// An entry with a retail hash takes the game's own string out of the
// language's table (LANGUAGES/<lang>.bin), so it reads exactly as the rest of
// the game does; one without (a word retail never needed) carries its own
// eight translations, in kPortLangs' order (portprefs.h). Retail strings are
// trimmed of a trailing ':' and the spaces before it, which many of the
// borrowed labels end in ("Bank:", French "Argent :").
//
// portstr_set_language() resolves every entry once, when the language
// changes; portstr() is then a table read.

#include <stdbool.h>

typedef enum {
	PS_ACCESSIBILITY,
	PS_LANGUAGE,
	PS_LARGE_TEXT,
	PS_HIGH_CONTRAST,
	// loading: the lower screen under Loading.fng (event, controls, tip) and
	// the boot profile screens in the kit
	PS_LOAD_LOADING,
	PS_LOAD_TIP,
	PS_LOAD_CONTROLS,
	PS_LOAD_FREE_ROAM,
	PS_LOAD_CIRCUIT,
	PS_LOAD_SPRINT,
	PS_LOAD_DRAG,
	PS_LOAD_DRIFT,
	PS_LOAD_STREET_X,
	PS_LOAD_URL,
	PS_LOAD_OUTRUN,
	PS_LOAD_TEST_RUN,
	PS_LOAD_ACCELERATE,
	PS_LOAD_BRAKE,
	PS_LOAD_STEER,
	PS_LOAD_NITROUS,
	PS_LOAD_HANDBRAKE,
	PS_LOAD_SHIFT_UP,
	PS_LOAD_SHIFT_DOWN,
	PS_LOAD_CAMERA,
	PS_LOAD_LOOK_BACK,
	PS_LOAD_RESET_CAR,
	PS_LOAD_ENTER_SHOP,
	PS_LOAD_WORLD_MAP,
	PS_LOAD_CAREER_STATS,
	PS_LOAD_SMS,
	PS_BOOT_LOAD,
	PS_BOOT_CREATE,
	PS_BOOT_NO_PROFILE,
	PS_BOOT_CHANGE,
	PS_COUNT
} PortStr;

// `langIndex` into kPortLangs; `langBinPath` the language's string table (NULL
// or unreadable: the entries with a retail hash fall back to their English).
void portstr_set_language(int langIndex, const char* langBinPath);
const char* portstr(PortStr id);

// PORT: the loading screen's tips. Retail ships no loading tips, but its
// career data has ~40 "Game Tips" SMS records (ENGAGE_TIPS_*, the mailbox's
// Game Tips category), each a subject and a body in every language table;
// the ones that read right on this port (no PC keys, no shop directions) are
// resolved here with the rest, when the language changes, so a load never
// reads a table. Each tip says which events it suits (PS_TIP_*).
enum {
	PS_TIP_RACE     = 1 << 0,   // grip racing: circuit, sprint, Street X, URL, Outrun
	PS_TIP_DRAG     = 1 << 1,
	PS_TIP_DRIFT    = 1 << 2,
	PS_TIP_STREET_X = 1 << 3,
	PS_TIP_URL      = 1 << 4,
	PS_TIP_OUTRUN   = 1 << 5,
	PS_TIP_FREE     = 1 << 6,   // free roam
	PS_TIP_TUNE     = 1 << 7,   // the dyno and its test run
};
int portstr_tip_count(void);
unsigned portstr_tip_kinds(int i);
// UTF-8, '^' the table's line break; "" when the tip has none (subject) or the
// table would not load (body: skip the tip).
const char* portstr_tip_subject(int i);
const char* portstr_tip_body(int i);
