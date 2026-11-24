#pragma once

// PORT: the port's own on-screen words in the player's language.
//
// Every string the port draws that is not one of the game's .fng texts goes
// through here: portstr(PS_x) returns it as UTF-8 for the language in use.
// An entry with a retail hash takes the game's own string out of the
// language's table (LANGUAGES/<lang>.bin), so it reads exactly as the rest of
// the game does; one without (a word retail never needed) carries its own
// eight translations, in kPortLangs' order (portprefs.h). Retail strings are
// trimmed of a trailing ':' and the spaces (and no-break spaces) round it,
// which many of the borrowed labels end in ("Bank:", French "Argent :",
// Spanish "Bolsa: "), and the table's '^' line breaks become spaces.
//
// portstr_set_language() resolves every entry once, when the language
// changes; portstr() is then a table read. The pointers it returns stay
// valid for the whole run (the text behind a retail entry changes in place
// with the language), so an options row may keep one.
//
// Formats: an entry whose English has printf conversions ("Racer %d") has
// the same conversions, in the same order, in every language -- the port's
// own translations are written so, and a retail one that does not match is
// passed over for the entry's own text.

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
	PS_BACK,
	PS_HELP,
	PS_SELECT,
	PS_BANK,
	PS_OK,
	PS_YES,
	PS_NO,
	PS_DONE,
	PS_CONTINUE,
	PS_TIP,
	PS_RESET,
	PS_TIME,
	PS_FEED_NEW_SMS,
	PS_FEED_GPS_OFF,
	PS_FEED_GPS_FMT,
	PS_RECENTER,
	PS_GPS,
	PS_NEAREST,
	PS_SET_GPS,
	PS_GPS_OFF,
	PS_SEARCHING,
	PS_SHOP,
	PS_EVENT,
	PS_UNIT_M,
	PS_UNIT_KM,
	PS_UNIT_FT,
	PS_UNIT_MI,
	PS_MK_SHOWCASE,
	PS_MK_GARAGE,
	PS_MK_PAINT,
	PS_MK_PARTS,
	PS_MK_ICE,
	PS_MK_PERF,
	PS_MK_CARLOT,
	PS_MK_CIRCUIT,
	PS_MK_DRAG,
	PS_MK_STREETX,
	PS_MK_DRIFT,
	PS_MK_SPRINT,
	PS_MK_URL,
	PS_MK_SUV,
	PS_MK_MARKER,
	PS_SPONSOR_RACE,
	PS_RACE,
	PS_MK_GARAGE_INFO,
	PS_SHOP_INFO,
	PS_NO_MAP_ART,
	PS_POSITION,
	PS_ORD_TH,
	PS_ORD_ST,
	PS_ORD_ND,
	PS_ORD_RD,
	PS_ORD_10,
	PS_LAP,
	PS_BEST,
	PS_SCORES,
	PS_STANDINGS,
	PS_RACER_FMT,
	PS_DISTANCE,
	PS_PURSE,
	PS_WORLD_CHALLENGE,
	PS_CHALLENGE,
	PS_VISUAL,
	PS_COMPLETE,
	PS_STAGE_FMT,
	PS_WORLD_RACES,
	PS_SPONSOR_RACES,
	PS_URL_RACES,
	PS_DVDS_WON,
	PS_NO_COVERS,
	PS_WORLD_MAP,
	PS_SHOW_ON_MAP,
	PS_SHOW_ALL,
	PS_HIDE_ALL,
	PS_FILTERS_FMT,
	PS_MAILBOX,
	PS_NO_MESSAGES,
	PS_READ,
	PS_DELETE,
	PS_INBOX,
	PS_FROM_FMT,
	PS_DRIVE,
	PS_LOT_BUY,
	PS_LOT_PICK,
	PS_LOT_TRADE_IN,
	PS_SLOT,
	PS_MY_CAR_FMT,
	PS_CAREER_CAR_FMT,
	PS_SPONSOR_CAR,
	PS_STOCK,
	PS_DRIVING,
	PS_SELECT_CAR,
	PS_BUY,
	PS_TRADE,
	PS_TRADE_IN,
	PS_CUSTOMIZE,
	PS_LOCKED,
	PS_LOCKED_MIXED,
	PS_FITTED,
	PS_OWNED,
	PS_OPTION_FMT,
	PS_INSTALL,
	PS_NONE,
	PS_T_TRACK_SELECT,
	PS_T_PERFORMANCE,
	PS_T_PAINT,
	PS_CHOOSE,
	PS_APPLY,
	PS_T_PROFILES,
	PS_T_TRAX,
	PS_RANDOM,
	PS_SEQUENTIAL,
	PS_PREVIEW,
	PS_MOVE,
	PS_RELEASE,
	PS_T_BRANDS,
	PS_BOOST,
	PS_TORQUE,
	PS_TUNE_HINT,
	PS_NITROUS,
	PS_POWER,
	PS_DURATION,
	PS_DYNO_RUN,
	PS_RPM_AXIS,
	PS_RESULTS,
	PS_STOP,
	PS_CHART,
	PS_DYNO_RESULTS,
	PS_THIS_RUN,
	PS_LAST_RUN,
	PS_FIRST_RUN,
	PS_TOP_SPEED,
	PS_OVERVIEW,
	PS_DONE_TAG,
	PS_T_LAN_SERVERS,
	PS_T_LAN_NAME,
	PS_T_LAN_LOBBY,
	PS_T_GAME_ROOM,
	PS_CREATE,
	PS_JOIN,
	PS_REFRESH,
	PS_FILTER,
	PS_LOOKING_SERVERS,
	PS_NO_GAMES,
	PS_YOUR_NAME,
	PS_TAP_NAME,
	PS_OPEN_SLOT,
	PS_TRACK,
	PS_LAPS,
	PS_DIRECTION,
	PS_OPTIONS,
	PS_OPT_GPS_COLOR,
	PS_OPT_DRAG_SHIFT,
	PS_COL_BLUE,
	PS_COL_CYAN,
	PS_COL_GREEN,
	PS_COL_LIME,
	PS_COL_YELLOW,
	PS_COL_ORANGE,
	PS_COL_RED,
	PS_COL_PINK,
	PS_COL_PURPLE,
	PS_COL_WHITE,
	PS_OPT_ENTER_SHOP,
	PS_OPT_DELETE_ALL,
	PS_OPT_3D_STYLE,
	PS_OPT_3D_DEPTH,
	PS_OPT_3D_FOCUS,
	PS_OPT_3D_HUD,
	PS_OPT_MOTION_STEER,
	PS_NOT_USED,
	PS_3D_POP,
	PS_3D_INTO,
	PS_BTN_CPAD,
	PS_BTN_DPAD_UP,
	PS_BTN_DPAD_DOWN,
	PS_BTN_DPAD_LEFT,
	PS_BTN_DPAD_RIGHT,
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
// or unreadable: the entries with a retail hash fall back to their own text).
void portstr_set_language(int langIndex, const char* langBinPath);
const char* portstr(PortStr id);
// The same in capitals (portstr_upper()), for the kit's caps labels; a
// format's conversions ("%d", "%s") are left as they are.
const char* portstr_caps(PortStr id);

// Upper-cases UTF-8 text in place: ASCII and the Latin-1 letters the eight
// languages use (any accented vowel, c-cedilla, n-tilde, a-ring, o-slash,
// ae, thorn, eth), plus oe. Every mapping keeps the byte length, so the text
// never grows. German sharp s stays as it is: its capital is two letters
// ("SS"), which would grow the text, and a lower-case ß in a caps label reads
// as German print does. y-diaeresis stays too (its capital is outside
// Latin-1, and none of the eight languages needs it).
void portstr_upper(char* s);

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
