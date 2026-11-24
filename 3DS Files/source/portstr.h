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
	// pause: the top screen's summary while the pause is up (drawPauseTop())
	PS_PAUSE_TITLE,
	PS_PAUSE_BANK,
	PS_PAUSE_STAGE,
	PS_PAUSE_GPS,
	PS_COUNT
} PortStr;

// `langIndex` into kPortLangs; `langBinPath` the language's string table (NULL
// or unreadable: the entries with a retail hash fall back to their English).
void portstr_set_language(int langIndex, const char* langBinPath);
const char* portstr(PortStr id);
