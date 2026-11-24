#pragma once

// PORT: the port's own per-profile settings -- the ones retail has no slot for
// (language, accessibility). Kept in a small text file beside the profile's
// save, so the save itself stays byte-for-byte retail's format:
//
//   <save root>/<Profile>/port.txt     (a legacy .sav: <file>.port.txt)
//
// key=value lines; unknown keys are ignored and missing ones keep their
// defaults, so later versions add keys without a format change. The last
// profile loaded also writes its settings to <save root>/port_last.txt, which
// the boot reads before any profile exists (title screen, the new-profile
// question, the name entry), so those already show the player's language and
// text size.

#include <stdbool.h>
#include <stddef.h>

// The Latin-script string tables the PC ships (LANGUAGES/<file>.bin).
typedef struct {
	const char* code;    // "EN", "FR", ...
	const char* file;    // "English", "French", ... (LANGUAGES/<file>.bin)
	const char* native;  // the language's own name, UTF-8
} PortLang;
#define PORT_LANG_COUNT 8
extern const PortLang kPortLangs[PORT_LANG_COUNT];

// The table index for a code, file name or native name (any case), -1 if none.
int portprefs_lang_index(const char* s);

typedef struct {
	int  language;       // kPortLangs index; 0 = English
	bool largeText;      // the kit (and the top HUD) at ~1.25x
	bool highContrast;   // solid panels, stronger text and rims
	bool motionSteer;    // 3DS accelerometer tilt steering (driving only)
} PortPrefs;

void portprefs_defaults(PortPrefs* p);
// Reads `path` over `p` (defaults are the caller's): false if it is not there.
bool portprefs_read(const char* path, PortPrefs* p);
bool portprefs_write(const char* path, const PortPrefs* p);

// Where a profile's file lives, from its save's path (ProfileEntry.path).
void portprefs_path_for(const char* savePath, bool legacy, char* out, size_t cap);
// <save root>/port_last.txt
void portprefs_last_path(const char* saveRoot, char* out, size_t cap);
