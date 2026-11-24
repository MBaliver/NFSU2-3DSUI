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
	// post-race
	PS_PR_CONTINUE,
	PS_PR_BACK,
	PS_PR_RACE_STATS,
	PS_PR_NEXT_RACE,
	PS_PR_RACE_RESULTS,
	PS_PR_STANDINGS,
	PS_PR_RACE_REWARD,
	PS_PR_SPONSOR,
	PS_PR_SELECT_SPONSOR,
	PS_PR_POST_RACE,
	PS_PR_POSITION,
	PS_PR_TIME,
	PS_PR_PURSE,
	PS_PR_POINTS,
	PS_PR_SCORE,
	PS_PR_SIGN,
	PS_PR_SIGNED,
	PS_PR_PREVIOUS,
	PS_PR_NEXT,
	PS_PR_TERMS,
	PS_COUNT
} PortStr;

// `langIndex` into kPortLangs; `langBinPath` the language's string table (NULL
// or unreadable: the entries with a retail hash fall back to their English).
void portstr_set_language(int langIndex, const char* langBinPath);
const char* portstr(PortStr id);
