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
	// post-race: the lower screen's action pages and the top summary
	// (drawUiPostRace(), drawPostRaceSummaryTop() in main.c).
	[PS_PR_CONTINUE] = { 0x9F1B2C7Au, { "Continue", "Continuer", "Weiter", "Continua",
		"Continuar", "Doorgaan", "Forts\xC3\xA4tt", "Forts\xC3\xA6t" } },
	[PS_PR_BACK] = { 0x001335F0u, { "Back", "Retour", "Zur\xC3\xBC" "ck", "Indietro",
		"Atr\xC3\xA1s", "Terug", "Bak\xC3\xA5t", "Tilbage" } },
	[PS_PR_RACE_STATS] = { 0xF82DF37Eu, { "Race Stats", "Stats course", "Rennstatistik", "Statistiche di gara",
		"Estad\xC3\xADsticas de carrera", "Racestatistieken", "T\xC3\xA4vlingsstatistik", "L\xC3\xB8" "bsstatistik" } },
	[PS_PR_NEXT_RACE] = { 0x23F359B8u, { "Next Race", "Course suivante", "N\xC3\xA4" "chstes Rennen", "Gara successiva",
		"Carrera siguiente", "Volgende race", "N\xC3\xA4sta lopp", "N\xC3\xA6ste l\xC3\xB8" "b" } },
	[PS_PR_RACE_RESULTS] = { 0x6D4C5049u, { "Race Results", "R\xC3\xA9sultats course", "Rennergebnis", "Risultati gara",
		"Resultados de la carrera", "Raceresultaten", "Loppresultat", "L\xC3\xB8" "bsresultat" } },
	[PS_PR_STANDINGS] = { 0x7E891F93u, { "Tournament Standings", "Position tournoi", "Turnier-Tabelle", "Classifiche torneo",
		"Clasificaci\xC3\xB3n de torneo", "Toernooistanden", "Totalresultat f\xC3\xB6r turnering", "Turneringsstilling" } },
	[PS_PR_RACE_REWARD] = { 0x8BEC8F84u, { "Race Reward", "R\xC3\xA9" "compense de course", "Renn-Pr\xC3\xA4mie", "Premio gara",
		"Premios de carrera", "Resultaat van de race", "Racebel\xC3\xB6ning", "L\xC3\xB8" "bsbel\xC3\xB8nning" } },
	[PS_PR_SPONSOR] = { 0x34E6ADBAu, { "Sponsor", "Sponsor", "Sponsor", "Sponsor",
		"Patrocinador", "Sponsor", "Sponsor", "Sponsor" } },
	[PS_PR_SELECT_SPONSOR] = { 0x3F0DCE03u, { "Select Sponsor", "S\xC3\xA9lectionner sponsor", "Sponsor ausw\xC3\xA4hlen", "Seleziona sponsor",
		"Seleccionar patrocinador", "Sponsor selecteren", "V\xC3\xA4lj sponsor", "V\xC3\xA6lg sponsor" } },
	[PS_PR_POST_RACE] = { 0xE35CC4C0u, { "Post Race", "Apr\xC3\xA8s course", "Nach dem Rennen", "Post gara",
		"Final de carrera", "Na race", "Efter racet", "Efter l\xC3\xB8" "bet" } },
	[PS_PR_POSITION] = { 0x75B6CA33u, { "Position", "Position", "Position", "Pos.",
		"Posici\xC3\xB3n", "Positie", "Placering", "Position" } },
	[PS_PR_TIME] = { 0xA1F8CC0Du, { "Time", "Temps", "Zeit", "Tempo",
		"Tiempo", "Tijd", "Tid", "Tid" } },
	[PS_PR_PURSE] = { 0x2CA40D1Fu, { "Purse", "Prix", "Preisgeld", "Premio",
		"Bolsa", "Prijzengeld", "Prispott", "Pulje" } },
	[PS_PR_POINTS] = { 0x70802830u, { "Points", "Points", "Punkte", "Punti",
		"Puntos", "Punten", "Po\xC3\xA4ng", "Point" } },
	[PS_PR_SCORE] = { 0xC03F9F19u, { "Score", "Score", "Punkte", "Punteggio",
		"Puntos", "Score", "Po\xC3\xA4ng", "Score" } },
	[PS_PR_SIGN] = { 0, { "Sign", "Signer", "Unterschreiben", "Firma",
		"Firmar", "Tekenen", "Skriv p\xC3\xA5", "Skriv under" } },
	[PS_PR_SIGNED] = { 0, { "Signed", "Sign\xC3\xA9", "Unterschrieben", "Firmato",
		"Firmado", "Getekend", "Undertecknad", "Underskrevet" } },
	[PS_PR_PREVIOUS] = { 0, { "Previous", "Pr\xC3\xA9" "c\xC3\xA9" "dent", "Vorheriger", "Precedente",
		"Anterior", "Vorige", "F\xC3\xB6reg\xC3\xA5" "ende", "Forrige" } },
	[PS_PR_NEXT] = { 0, { "Next", "Suivant", "N\xC3\xA4" "chster", "Successivo",
		"Siguiente", "Volgende", "N\xC3\xA4sta", "N\xC3\xA6ste" } },
	[PS_PR_TERMS] = { 0, { "Terms", "Conditions", "Bedingungen", "Condizioni",
		"Condiciones", "Voorwaarden", "Villkor", "Vilk\xC3\xA5r" } },
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
