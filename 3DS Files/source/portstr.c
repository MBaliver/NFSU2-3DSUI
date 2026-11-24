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
	// loading: the lower screen under Loading.fng and the boot profile screens
	[PS_LOAD_LOADING] = { 0, { "Loading", "Chargement", "Wird geladen", "Caricamento", "Cargando",
		"Laden", "Laddar", "Indl\xC3\xA6ser" } },
	[PS_LOAD_TIP] = { 0xCBBCB1C5u, { "Tip", "Astuce", "Tipp", "Consiglio", "Consejo", "Tip", "Tips",
		"Tip" } },
	[PS_LOAD_CONTROLS] = { 0xD77A22D8u, { "Controls", "Commandes", "Steuerung", "Controlli",
		"Controles", "Besturing", "Kontroller", "Kommandoer" } },
	[PS_LOAD_FREE_ROAM] = { 0x303BB325u, { "Free Roam", "Parcours libre", "Freie Fahrt",
		"Esplorazione libera", "Circulaci\xC3\xB3n libre", "Vrij rijden", "Fri k\xC3\xB6rning",
		"Fri k\xC3\xB8rsel" } },
	[PS_LOAD_CIRCUIT] = { 0x268D063Cu, { "Circuit", "Circuit", "Rundkurs", "Circuito", "Circuito",
		"Circuitrace", "Bana", "Circuit" } },
	[PS_LOAD_SPRINT] = { 0x9EC5F82Fu, { "Sprint", "Sprint", "Sprint", "Sprint", "Sprint", "Sprint",
		"Sprint", "Sprint" } },
	[PS_LOAD_DRAG] = { 0x35102EDFu, { "Drag", "Drag", "Drag", "Sparo", "Aceleraci\xC3\xB3n", "Drag",
		"Dragrace", "Drag" } },
	[PS_LOAD_DRIFT] = { 0x3A10A7C2u, { "Drift", "Drift", "Drift", "D\xC3\xA9rapage", "Derrape",
		"Drift", "Drift", "Drift" } },
	[PS_LOAD_STREET_X] = { 0x0FDA6758u, { "Street X", "Street X", "Street X", "Street X", "Calle X",
		"Street X", "Street X", "Street X" } },
	[PS_LOAD_URL] = { 0x80B8267Cu, { "URL", "CCU", "URL", "URL", "URL", "URL", "URL", "URL" } },
	[PS_LOAD_OUTRUN] = { 0x613D3106u, { "Outrun", "Poursuite", "Outrun", "Inseguimento",
		"Persecuci\xC3\xB3n", "Outrun", "Outrun-race", "Outrun" } },
	[PS_LOAD_TEST_RUN] = { 0xF62234EFu, { "Test Run", "Tour d'essai", "Testfahrt", "Corsa di prova",
		"Carrera de prueba", "Testrace", "Testrunda", "Testl\xC3\xB8" "b" } },
	[PS_LOAD_ACCELERATE] = { 0x619B23B0u, { "Accelerate", "Acc\xC3\xA9l\xC3\xA9rer",
		"Beschleunigen", "Acceleratore", "Acelerar", "Gas geven", "Accelerera", "Accelerer" } },
	[PS_LOAD_BRAKE] = { 0x6AB80AFAu, { "Brake / Reverse", "Freiner/Reculer",
		"Bremsen / R\xC3\xBC" "ckw\xC3\xA4rts", "Freno/Retro", "Frenar/Marcha atr\xC3\xA1s",
		"Remmen / achteruit", "Bromsa/Backa", "Brems / Bak" } },
	[PS_LOAD_STEER] = { 0xECD20F7Fu, { "Steer", "Direction", "Lenken", "Sterzo", "Girar", "Sturen",
		"Styr", "Styring" } },
	[PS_LOAD_NITROUS] = { 0x5E5C5F2Eu, { "Nitrous", "Nitro", "Nitro", "N2O", "Nitroso", "Nitro",
		"Lustgas", "Nitro" } },
	[PS_LOAD_HANDBRAKE] = { 0xB70FF4A4u, { "Handbrake", "Frein \xC3\xA0 main", "Handbremse",
		"Freno a mano", "Freno de mano", "Handrem", "Handbroms", "H\xC3\xA5ndbremse" } },
	[PS_LOAD_SHIFT_UP] = { 0xDC1CD97Du, { "Shift Up", "Rapport sup\xC3\xA9rieur", "Hochschalten",
		"Marcia +", "Subir marcha", "Opschakelen", "V\xC3\xA4xla upp", "Skift op" } },
	[PS_LOAD_SHIFT_DOWN] = { 0x56AFE190u, { "Shift Down", "Rapport inf\xC3\xA9rieur",
		"Runterschalten", "Marcia -", "Bajar marcha", "Terugschakelen", "V\xC3\xA4xla ned",
		"Skift ned" } },
	[PS_LOAD_CAMERA] = { 0xD38DF194u, { "Change Camera", "Changer cam\xC3\xA9ra", "Kamera wechseln",
		"Cambia visuale", "Cambiar c\xC3\xA1mara", "Andere camera", "Byt kamera",
		"Skift kameravinkel" } },
	[PS_LOAD_LOOK_BACK] = { 0x13121940u, { "Look Back", "R\xC3\xA9tro", "Zur\xC3\xBC" "ckblicken",
		"Guarda indietro", "Mirar atr\xC3\xA1s", "Achterom kijken", "Titta bak\xC3\xA5t",
		"Se bagud" } },
	[PS_LOAD_RESET_CAR] = { 0x1CEDC132u, { "Reset Car", "R\xC3\xA9tablir voiture",
		"Wagen zur\xC3\xBC" "cksetzen", "Ripristina auto", "Reiniciar coche", "Wagen resetten",
		"\xC3\x85terst\xC3\xA4ll bil", "Gendan bil" } },
	[PS_LOAD_ENTER_SHOP] = { 0xAB1032FAu, { "Enter Shop / Challenge Opponent",
		"Acc\xC3\xA9" "der atelier/ Acc\xC3\xA9" "der \xC3\xA9preuve",
		"Betreten / Gegner herausfordern", "Entra in negozio/ Sfida avversario",
		"Entrar en tienda/ Desafiar a oponente", "Shop ingaan / Tegenstander uitdagen",
		"K\xC3\xB6r in/ Utmana motst\xC3\xA5ndare", "Bes\xC3\xB8g butik / Udfordr modstander" } },
	[PS_LOAD_WORLD_MAP] = { 0x6026C91Fu, { "World Map", "Carte du monde", "Weltkarte",
		"Mappa generale", "Mapa del mundo", "Wereldkaart", "V\xC3\xA4rldskarta", "Verdenskort" } },
	[PS_LOAD_CAREER_STATS] = { 0x70A30056u, { "Career Statistics", "Statut carri\xC3\xA8re",
		"Karriere-Statistik", "Statistiche carriera", "Estad\xC3\xADsticas de carrera",
		"Carri\xC3\xA8restatistieken", "Karri\xC3\xA4rstatistik", "Karrierestatus" } },
	[PS_LOAD_SMS] = { 0x6EF09D0Du, { "Display SMS", "Afficher SMS", "SMS anzeigen", "Mostra SMS",
		"Mostrar SMS", "Sms tonen", "Visa SMS", "Vis SMS" } },
	[PS_BOOT_LOAD] = { 0x0018ECFFu, { "Load", "Charger", "Laden", "Carica", "Cargar", "Laden",
		"Ladda", "Indl\xC3\xA6s" } },
	[PS_BOOT_CREATE] = { 0x55423473u, { "Create", "Cr\xC3\xA9" "er", "Erstellen", "Crea", "Crear",
		"Aanmaken", "Skapa", "Opret" } },
	[PS_BOOT_NO_PROFILE] = { 0xCF4F80C2u, { "No Profile", "Aucun profil", "Kein Profil",
		"Nessun profilo", "Sin perfil", "Geen profiel", "Ingen profil", "Ingen profil" } },
	[PS_BOOT_CHANGE] = { 0x548B4385u, { "Change", "Changer", "\xC3\x84ndern", "Cambia", "Cambiar",
		"Wijzigen", "\xC3\x84ndra", "Skift" } },
};

static const char* s_cur[PS_COUNT];
static char* s_owned[PS_COUNT];   // retail strings copied out of the table

// The loading screen's tips (portstr.h): ENGAGE_TIPS_<NAME>_SUBJECT / _BODY,
// keyed as eventdb_feng_hash() keys them. The four without a record of their
// own are the race-type hints the same tables carry beside them; they borrow
// the subject of their record's family or go without one.
typedef struct {
	u32 subject, body;   // retail keys, subject 0 = none
	unsigned kinds;      // PS_TIP_*
} PortTipDef;

static const PortTipDef kTips[] = {
	{ 0x53679B54u, 0x21707032u, PS_TIP_RACE },                    // BBRK, basic braking
	{ 0x40D7A271u, 0x4B27846Fu, PS_TIP_RACE },                    // SLBRK, straight line braking
	{ 0x6A88B9A0u, 0xFEC151FEu, PS_TIP_RACE },                    // SLWIN, slow in - fast out
	{ 0xAE735E44u, 0x04A99922u, PS_TIP_RACE },                    // TRLBRK, trail braking
	{ 0x037304ECu, 0xDDA280CAu, PS_TIP_RACE | PS_TIP_TUNE },      // ENGBRK, engine braking
	{ 0x4C8770BEu, 0x2EF02DDCu, PS_TIP_RACE | PS_TIP_DRAG | PS_TIP_FREE },   // RCEBRKR
	{ 0xD92EF843u, 0x46905B81u, PS_TIP_RACE | PS_TIP_FREE },      // WEATHER
	{ 0x9354D1C2u, 0x907D8D60u, PS_TIP_STREET_X },                // STREETX
	{ 0x9354D1C2u, 0xB372757Au, PS_TIP_STREET_X },                // Street X, the short tracks
	{ 0,           0x4C22CEECu, PS_TIP_URL },                     // URL
	{ 0x227EBA64u, 0x56756942u, PS_TIP_DRAG },                    // DRAFT, drag drafting
	{ 0,           0x31E50D09u, PS_TIP_DRAG },                    // drag lane change
	{ 0,           0x7B04ECC5u, PS_TIP_DRAG | PS_TIP_TUNE },      // perfect shift points
	{ 0x4D64646Cu, 0xB884104Au, PS_TIP_DRIFT },                   // DRIFT, the bonus areas
	{ 0x4D64646Cu, 0x6E86D064u, PS_TIP_DRIFT },                   // drift scoring
	{ 0x586CA736u, 0x92243754u, PS_TIP_DRIFT },                   // ORGANIC
	{ 0,           0x6E11E175u, PS_TIP_DRIFT },                   // downhill drift
	{ 0x6452D482u, 0xD8BC8820u, PS_TIP_OUTRUN },                  // ORSTRAT
	{ 0xC923C8FFu, 0xF2DC85BDu, PS_TIP_FREE },                    // HIDDEN
	{ 0xC0398786u, 0x0B9479A4u, PS_TIP_FREE },                    // SMS
	{ 0xF9B92E68u, 0xCA50BBC6u, PS_TIP_FREE },                    // MAG
	{ 0x5BE4A3BAu, 0xAD75E258u, PS_TIP_FREE },                    // MAG2
	{ 0xD707383Du, 0x0FCE0DBBu, PS_TIP_FREE },                    // NEW, new parts SMS
	{ 0x70DF6656u, 0x73DA4A74u, PS_TIP_FREE | PS_TIP_TUNE },      // RBRKR, Racebreaker
	{ 0xA85772B3u, 0x36D56BF1u, PS_TIP_TUNE },                    // REALTUNE
	{ 0,           0x46B41BB8u, PS_TIP_TUNE },                    // nitrous tuning
};
#define PORT_TIPS ((int)(sizeof(kTips) / sizeof(kTips[0])))
static char* s_tipSubject[PORT_TIPS];
static char* s_tipBody[PORT_TIPS];

static char* tipCopy(const LangTable* lang, u32 hash) {
	const char* r = lang && hash ? langbin_lookup(lang, hash) : NULL;
	return r && r[0] ? strdup(r) : NULL;
}

static void tipsResolve(const LangTable* lang) {
	for (int i = 0; i < PORT_TIPS; i++) {
		free(s_tipSubject[i]);
		free(s_tipBody[i]);
		s_tipSubject[i] = tipCopy(lang, kTips[i].subject);
		s_tipBody[i] = tipCopy(lang, kTips[i].body);
	}
}

int portstr_tip_count(void) {
	return PORT_TIPS;
}

unsigned portstr_tip_kinds(int i) {
	return i >= 0 && i < PORT_TIPS ? kTips[i].kinds : 0u;
}

const char* portstr_tip_subject(int i) {
	return i >= 0 && i < PORT_TIPS && s_tipSubject[i] ? s_tipSubject[i] : "";
}

const char* portstr_tip_body(int i) {
	return i >= 0 && i < PORT_TIPS && s_tipBody[i] ? s_tipBody[i] : "";
}

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
	tipsResolve(have ? &lang : NULL);
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
