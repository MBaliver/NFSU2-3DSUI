// PORT: the port's own words in eight languages -- see portstr.h.
#include "portstr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3ds/types.h>
#include "langbin.h"
#include "portprefs.h"

typedef struct {
	u32 hash;                          // retail key, 0 = port-only
	const char* t[PORT_LANG_COUNT];    // EN FR DE IT ES NL SV DA (EN also the fallback)
} PortStrDef;

static const PortStrDef kStr[PS_COUNT] = {
	[PS_ACCESSIBILITY] = { 0, { "Accessibility", "Accessibilit\xC3\xA9", "Barrierefreiheit",
		"Accessibilit\xC3\xA0", "Accesibilidad", "Toegankelijkheid", "Tillg\xC3\xA4nglighet",
		"Tilg\xC3\xA6ngelighed" } },
	[PS_LANGUAGE] = { 0, { "Language", "Langue", "Sprache", "Lingua", "Idioma", "Taal",
		"Spr\xC3\xA5k", "Sprog" } },
	[PS_LARGE_TEXT] = { 0, { "Large Text", "Texte agrandi", "Gro\xC3\x9F" "e Schrift", "Testo grande",
		"Texto grande", "Grote tekst", "Stor text", "Stor tekst" } },
	[PS_HIGH_CONTRAST] = { 0, { "High Contrast", "Contraste \xC3\xA9lev\xC3\xA9", "Hoher Kontrast",
		"Contrasto elevato", "Alto contraste", "Hoog contrast", "H\xC3\xB6g kontrast",
		"H\xC3\xB8j kontrast" } },
	// pause: the top screen's summary while the pause is up (drawPauseTop()).
	// All four are retail's words; the texts are their fallbacks. STAGE is a
	// format with one %d (the career stage).
	[PS_PAUSE_TITLE] = { 0x6C839FBEu, { "Pause", "Pause", "Pause", "Pausa", "Pausa", "Pauze", "Paus",
		"Pause" } },
	[PS_PAUSE_BANK] = { 0x13B49408u, { "Bank", "Argent", "Konto", "Denaro", "Banco", "Bankrekening",
		"Pengar", "Kontanter" } },
	[PS_PAUSE_STAGE] = { 0x8B84E940u, { "Stage %d", "Niveau %d", "Stufe %d", "Livello %d", "Etapa %d",
		"Niveau %d", "Niv\xC3\xA5 %d", "Niveau %d" } },
	[PS_PAUSE_GPS] = { 0x8C00BBDDu, { "GPS", "GPS", "GPS-System", "GPS", "GPS", "GPS", "GPS", "GPS" } },
};

static const char* s_cur[PS_COUNT];
static char* s_owned[PS_COUNT];   // retail strings copied out of the table

void portstr_set_language(int langIndex, const char* langBinPath) {
	if (langIndex < 0 || langIndex >= PORT_LANG_COUNT)
		langIndex = 0;
	LangTable lang;
	const bool have = langBinPath && langbin_load(langBinPath, &lang);
	for (int i = 0; i < PS_COUNT; i++) {
		free(s_owned[i]);
		s_owned[i] = NULL;
		const PortStrDef* d = &kStr[i];
		const char* own = d->t[langIndex] ? d->t[langIndex] : d->t[0];
		s_cur[i] = own;
		if (!d->hash || !have)
			continue;
		const char* r = langbin_lookup(&lang, d->hash);
		if (!r || !r[0])
			continue;
		char* c = strdup(r);
		if (!c)
			continue;
		// A trailing ':' and the spaces (and no-break spaces) before it.
		size_t n = strlen(c);
		if (n && c[n - 1] == ':')
			c[--n] = '\0';
		while (n && (c[n - 1] == ' ' ||
				(n >= 2 && (u8)c[n - 2] == 0xC2 && (u8)c[n - 1] == 0xA0))) {
			if (c[n - 1] == ' ')
				c[--n] = '\0';
			else
				c[n -= 2] = '\0';
		}
		s_owned[i] = c;
		s_cur[i] = c;
	}
	if (have)
		langbin_free(&lang);
}

const char* portstr(PortStr id) {
	if ((int)id < 0 || id >= PS_COUNT)
		return "";
	if (!s_cur[id])
		return kStr[id].t[0];
	return s_cur[id];
}
