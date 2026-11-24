// PORT: the port's own words in eight languages -- see portstr.h.
#include "portstr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3ds/types.h>
#include "langbin.h"
#include "portprefs.h"

// A retail entry whose string may be empty: the ordinal suffixes, which
// Swedish leaves blank ("1", not "1st") -- empty is that language's answer,
// not a hole to fill with English.
#define PSF_EMPTY_OK 1u

typedef struct {
	u32 hash;                          // retail key, 0 = port-only
	u8 flags;                          // PSF_
	// EN FR DE IT ES NL SV DA; NULL = English. A retail entry's list is its
	// fallback: the English for a missing key or table, and a language's own
	// word where its table leaves that key empty (Preview in three of them).
	const char* t[PORT_LANG_COUNT];
} PortStrDef;

static const PortStrDef kStr[PS_COUNT] = {
	[PS_ACCESSIBILITY] = { 0, 0, { "Accessibility", "Accessibilit\xC3\xA9", "Barrierefreiheit",
		"Accessibilit\xC3\xA0", "Accesibilidad", "Toegankelijkheid", "Tillg\xC3\xA4nglighet",
		"Tilg\xC3\xA6ngelighed" } },
	[PS_LANGUAGE] = { 0, 0, { "Language", "Langue", "Sprache", "Lingua", "Idioma", "Taal",
		"Spr\xC3\xA5k", "Sprog" } },
	[PS_LARGE_TEXT] = { 0, 0, { "Large Text", "Texte agrandi", "Gro\xC3\x9F" "e Schrift", "Testo grande",
		"Texto grande", "Grote tekst", "Stor text", "Stor tekst" } },
	[PS_HIGH_CONTRAST] = { 0, 0, { "High Contrast", "Contraste \xC3\xA9lev\xC3\xA9", "Hoher Kontrast",
		"Contrasto elevato", "Alto contraste", "Hoog contrast", "H\xC3\xB6g kontrast",
		"H\xC3\xB8j kontrast" } },
	// pause: the top screen's summary while the pause is up (drawPauseTop()).
	// All four are retail's words; the texts are their fallbacks. STAGE is a
	// format with one %d (the career stage).
	[PS_PAUSE_TITLE] = { 0x6C839FBEu, 0, { "Pause", "Pause", "Pause", "Pausa", "Pausa", "Pauze", "Paus",
		"Pause" } },
	[PS_PAUSE_BANK] = { 0x13B49408u, 0, { "Bank", "Argent", "Konto", "Denaro", "Banco", "Bankrekening",
		"Pengar", "Kontanter" } },
	[PS_PAUSE_STAGE] = { 0x8B84E940u, 0, { "Stage %d", "Niveau %d", "Stufe %d", "Livello %d", "Etapa %d",
		"Niveau %d", "Niv\xC3\xA5 %d", "Niveau %d" } },
	[PS_PAUSE_GPS] = { 0x8C00BBDDu, 0, { "GPS", "GPS", "GPS-System", "GPS", "GPS", "GPS", "GPS", "GPS" } },
	// post-race: the lower screen's action pages and the top summary
	// (drawUiPostRace(), drawPostRaceSummaryTop() in main.c).
	[PS_PR_CONTINUE] = { 0x9F1B2C7Au, 0, { "Continue", "Continuer", "Weiter", "Continua",
		"Continuar", "Doorgaan", "Forts\xC3\xA4tt", "Forts\xC3\xA6t" } },
	[PS_PR_BACK] = { 0x001335F0u, 0, { "Back", "Retour", "Zur\xC3\xBC" "ck", "Indietro",
		"Atr\xC3\xA1s", "Terug", "Bak\xC3\xA5t", "Tilbage" } },
	[PS_PR_RACE_STATS] = { 0xF82DF37Eu, 0, { "Race Stats", "Stats course", "Rennstatistik", "Statistiche di gara",
		"Estad\xC3\xADsticas de carrera", "Racestatistieken", "T\xC3\xA4vlingsstatistik", "L\xC3\xB8" "bsstatistik" } },
	[PS_PR_NEXT_RACE] = { 0x23F359B8u, 0, { "Next Race", "Course suivante", "N\xC3\xA4" "chstes Rennen", "Gara successiva",
		"Carrera siguiente", "Volgende race", "N\xC3\xA4sta lopp", "N\xC3\xA6ste l\xC3\xB8" "b" } },
	[PS_PR_RACE_RESULTS] = { 0x6D4C5049u, 0, { "Race Results", "R\xC3\xA9sultats course", "Rennergebnis", "Risultati gara",
		"Resultados de la carrera", "Raceresultaten", "Loppresultat", "L\xC3\xB8" "bsresultat" } },
	[PS_PR_STANDINGS] = { 0x7E891F93u, 0, { "Tournament Standings", "Position tournoi", "Turnier-Tabelle", "Classifiche torneo",
		"Clasificaci\xC3\xB3n de torneo", "Toernooistanden", "Totalresultat f\xC3\xB6r turnering", "Turneringsstilling" } },
	[PS_PR_RACE_REWARD] = { 0x8BEC8F84u, 0, { "Race Reward", "R\xC3\xA9" "compense de course", "Renn-Pr\xC3\xA4mie", "Premio gara",
		"Premios de carrera", "Resultaat van de race", "Racebel\xC3\xB6ning", "L\xC3\xB8" "bsbel\xC3\xB8nning" } },
	[PS_PR_SPONSOR] = { 0x34E6ADBAu, 0, { "Sponsor", "Sponsor", "Sponsor", "Sponsor",
		"Patrocinador", "Sponsor", "Sponsor", "Sponsor" } },
	[PS_PR_SELECT_SPONSOR] = { 0x3F0DCE03u, 0, { "Select Sponsor", "S\xC3\xA9lectionner sponsor", "Sponsor ausw\xC3\xA4hlen", "Seleziona sponsor",
		"Seleccionar patrocinador", "Sponsor selecteren", "V\xC3\xA4lj sponsor", "V\xC3\xA6lg sponsor" } },
	[PS_PR_POST_RACE] = { 0xE35CC4C0u, 0, { "Post Race", "Apr\xC3\xA8s course", "Nach dem Rennen", "Post gara",
		"Final de carrera", "Na race", "Efter racet", "Efter l\xC3\xB8" "bet" } },
	[PS_PR_POSITION] = { 0x75B6CA33u, 0, { "Position", "Position", "Position", "Pos.",
		"Posici\xC3\xB3n", "Positie", "Placering", "Position" } },
	[PS_PR_TIME] = { 0xA1F8CC0Du, 0, { "Time", "Temps", "Zeit", "Tempo",
		"Tiempo", "Tijd", "Tid", "Tid" } },
	[PS_PR_PURSE] = { 0x2CA40D1Fu, 0, { "Purse", "Prix", "Preisgeld", "Premio",
		"Bolsa", "Prijzengeld", "Prispott", "Pulje" } },
	[PS_PR_POINTS] = { 0x70802830u, 0, { "Points", "Points", "Punkte", "Punti",
		"Puntos", "Punten", "Po\xC3\xA4ng", "Point" } },
	[PS_PR_SCORE] = { 0xC03F9F19u, 0, { "Score", "Score", "Punkte", "Punteggio",
		"Puntos", "Score", "Po\xC3\xA4ng", "Score" } },
	[PS_PR_SIGN] = { 0, 0, { "Sign", "Signer", "Unterschreiben", "Firma",
		"Firmar", "Tekenen", "Skriv p\xC3\xA5", "Skriv under" } },
	[PS_PR_SIGNED] = { 0, 0, { "Signed", "Sign\xC3\xA9", "Unterschrieben", "Firmato",
		"Firmado", "Getekend", "Undertecknad", "Underskrevet" } },
	[PS_PR_PREVIOUS] = { 0, 0, { "Previous", "Pr\xC3\xA9" "c\xC3\xA9" "dent", "Vorheriger", "Precedente",
		"Anterior", "Vorige", "F\xC3\xB6reg\xC3\xA5" "ende", "Forrige" } },
	[PS_PR_NEXT] = { 0, 0, { "Next", "Suivant", "N\xC3\xA4" "chster", "Successivo",
		"Siguiente", "Volgende", "N\xC3\xA4sta", "N\xC3\xA6ste" } },
	[PS_PR_TERMS] = { 0, 0, { "Terms", "Conditions", "Bedingungen", "Condizioni",
		"Condiciones", "Voorwaarden", "Villkor", "Vilk\xC3\xA5r" } },
	[PS_BACK] = { 0x001335F0u, 0, { "Back" } },
	[PS_HELP] = { 0x53FCAFE5u, 0, { "Help" } },
	[PS_SELECT] = { 0x8202C364u, 0, { "Select" } },
	[PS_BANK] = { 0x13B49408u, 0, { "Bank" } },
	[PS_OK] = { 0x00000639u, 0, { "OK" } },
	[PS_YES] = { 0x0000F770u, 0, { "Yes" } },
	[PS_NO] = { 0x0000061Cu, 0, { "No" } },
	[PS_DONE] = { 0, 0, { "Done", "Termin\xC3\xA9", "Fertig", "Fatto", "Hecho", "Klaar",
		"Klar", "F\xC3\xA6rdig" } },
	[PS_CONTINUE] = { 0x9F1B2C7Au, 0, { "Continue" } },
	[PS_TIP] = { 0xCBBCB1C5u, 0, { "Tip" } },
	[PS_RESET] = { 0, 0, { "Reset", "R\xC3\xA9tablir", "Zur\xC3\xBC" "cksetzen",
		"Ripristina", "Restablecer", "Herstellen", "\xC3\x85terst\xC3\xA4ll", "Nulstil" } },
	[PS_TIME] = { 0xA1F8CC0Du, 0, { "Time" } },
	[PS_FEED_NEW_SMS] = { 0, 0, { "New SMS message", "Nouveau SMS", "Neue SMS",
		"Nuovo SMS", "Nuevo SMS", "Nieuw sms-bericht", "Nytt SMS", "Ny SMS" } },
	[PS_FEED_GPS_OFF] = { 0, 0, { "GPS off", "GPS d\xC3\xA9sactiv\xC3\xA9", "GPS aus",
		"GPS disattivato", "GPS desactivado", "GPS uit", "GPS av", "GPS sl\xC3\xA5" "et fra" } },
	[PS_FEED_GPS_FMT] = { 0, 0, { "GPS: %s", "GPS : %s" } },
	[PS_RECENTER] = { 0, 0, { "Recenter", "Recentrer", "Zentrieren", "Centra", "Centrar",
		"Centreren", "Centrera", "Centr\xC3\xA9r" } },
	[PS_GPS] = { 0, 0, { "GPS" } },
	[PS_NEAREST] = { 0, 0, { "NEAREST", "PLUS PROCHE", "N\xC3\x84" "CHSTES",
		"PI\xC3\x99 VICINO", "M\xC3\x81S CERCANO", "DICHTSTBIJ", "N\xC3\x84RMAST", "N\xC3\x86RMESTE" } },
	[PS_SET_GPS] = { 0, 0, { "Set GPS", "Activer GPS", "GPS aktivieren", "Attiva GPS",
		"Activar GPS", "GPS activeren", "Aktivera GPS", "Aktiver GPS" } },
	[PS_GPS_OFF] = { 0, 0, { "GPS Off", "D\xC3\xA9sactiver GPS", "GPS deaktivieren",
		"Disattiva GPS", "Desactivar GPS", "GPS deactiveren", "Inaktivera GPS", "Deaktiver GPS" } },
	[PS_SEARCHING] = { 0x2F0CFB04u, 0, { "Searching..." } },
	[PS_SHOP] = { 0, 0, { "SHOP", "ATELIER", "SHOP", "NEGOZIO", "TIENDA", "SHOP", "BUTIK",
		"BUTIK" } },
	[PS_EVENT] = { 0, 0, { "EVENT", "\xC3\x89PREUVE", "EVENT", "EVENTO", "EVENTO",
		"EVENT", "T\xC3\x84VLING", "L\xC3\x98" "B" } },
	[PS_UNIT_M] = { 0xF0AEB103u, 0, { "m" } },
	[PS_UNIT_KM] = { 0xB5D640B1u, 0, { "km" } },
	[PS_UNIT_FT] = { 0xD0B86077u, 0, { "ft" } },
	[PS_UNIT_MI] = { 0xAC3A4F4Cu, 0, { "mi" } },
	[PS_MK_SHOWCASE] = { 0x69B14E86u, 0, { "Showcase" } },
	[PS_MK_GARAGE] = { 0x9D53BD10u, 0, { "Garage" } },
	[PS_MK_PAINT] = { 0x373DAD1Au, 0, { "Graphics Shop" } },
	[PS_MK_PARTS] = { 0x40C7F697u, 0, { "Body Shop" } },
	[PS_MK_ICE] = { 0xB46342BAu, 0, { "Car Specialties Shop" } },
	[PS_MK_PERF] = { 0xD6F5819Bu, 0, { "Performance Shop" } },
	[PS_MK_CARLOT] = { 0x93FF4A6Eu, 0, { "Car Lot" } },
	[PS_MK_CIRCUIT] = { 0x268D063Cu, 0, { "Circuit" } },
	[PS_MK_DRAG] = { 0x40C91BA7u, 0, { "Drag" } },
	[PS_MK_STREETX] = { 0x0FDA6758u, 0, { "Street X" } },
	[PS_MK_DRIFT] = { 0x59ECB2C2u, 0, { "Drift" } },
	[PS_MK_SPRINT] = { 0xBA613729u, 0, { "Sprint" } },
	[PS_MK_URL] = { 0x80B8267Cu, 0, { "URL" } },
	[PS_MK_SUV] = { 0xB4636FA7u, 0, { "SUV" } },
	[PS_MK_MARKER] = { 0, 0, { "Marker", "Rep\xC3\xA8re", "Markierung", "Segnalino",
		"Marcador", "Markering", "Mark\xC3\xB6r", "Mark\xC3\xB8r" } },
	[PS_SPONSOR_RACE] = { 0, 0, { "Sponsor race", "Course sponsor", "Sponsorenrennen",
		"Gara sponsor", "Carrera patrocinada", "Sponsorrace", "Sponsorlopp", "Sponsorl\xC3\xB8" "b" } },
	[PS_RACE] = { 0, 0, { "Race", "Course", "Rennen", "Gara", "Carrera", "Race", "Lopp",
		"L\xC3\xB8" "b" } },
	[PS_MK_GARAGE_INFO] = { 0, 0, { "Save and change cars", "Sauvegarde, changer d'auto",
		"Speichern, Wagen wechseln", "Salva e cambia auto", "Guardar y cambiar coche",
		"Opslaan en wagen wisselen", "Spara och byt bil", "Gem og skift bil" } },
	[PS_SHOP_INFO] = { 0, 0, { "Shop", "Atelier", "Shop", "Negozio", "Tienda", "Shop",
		"Butik", "Butik" } },
	[PS_NO_MAP_ART] = { 0, 0, {
		"No map art.\nCopy %s from the game data\nto the GameDir folder on the card.",
		"Pas d'image de carte.\nCopiez %s depuis les donn\xC3\xA9" "es du jeu\ndans le dossier GameDir de la carte SD.",
		"Keine Kartengrafik.\nKopiere %s aus den Spieldaten\nin den GameDir-Ordner auf der SD-Karte.",
		"Grafica della mappa assente.\nCopia %s dai dati del gioco\nnella cartella GameDir della scheda SD.",
		"Falta la imagen del mapa.\nCopia %s de los datos del juego\na la carpeta GameDir de la tarjeta SD.",
		"Geen kaartafbeelding.\nKopieer %s uit de spelgegevens\nnaar de map GameDir op de SD-kaart.",
		"Kartbild saknas.\nKopiera %s fr\xC3\xA5n speldata\ntill mappen GameDir p\xC3\xA5 SD-kortet.",
		"Kortbillede mangler.\nKopi\xC3\xA9r %s fra spildata\ntil mappen GameDir p\xC3\xA5 SD-kortet." } },
	[PS_POSITION] = { 0x9ABAF068u, 0, { "POSITION" } },
	[PS_ORD_TH] = { 0x7447C0E3u, PSF_EMPTY_OK, { "th" } },
	[PS_ORD_ST] = { 0xBC4CE12Fu, PSF_EMPTY_OK, { "st" } },
	[PS_ORD_ND] = { 0x63EB6A03u, PSF_EMPTY_OK, { "nd" } },
	[PS_ORD_RD] = { 0xBD498562u, PSF_EMPTY_OK, { "rd" } },
	[PS_ORD_10] = { 0xBCDD9448u, PSF_EMPTY_OK, { "th" } },
	[PS_LAP] = { 0x90038D25u, 0, { "Lap" } },
	[PS_BEST] = { 0x3298DE8Du, 0, { "Best" } },
	[PS_SCORES] = { 0, 0, { "SCORES", "SCORES", "PUNKTE", "PUNTEGGI", "PUNTUACIONES",
		"SCORES", "PO\xC3\x84NG", "POINT" } },
	[PS_STANDINGS] = { 0, 0, { "STANDINGS", "CLASSEMENT", "RANGLISTE", "CLASSIFICA",
		"POSICIONES", "STAND", "ST\xC3\x84LLNING", "STILLING" } },
	[PS_RACER_FMT] = { 0, 0, { "Racer %d", "Pilote %d", "Fahrer %d", "Pilota %d",
		"Piloto %d", "Coureur %d", "F\xC3\xB6rare %d", "K\xC3\xB8rer %d" } },
	[PS_DISTANCE] = { 0x0642806Bu, 0, { "Distance" } },
	[PS_PURSE] = { 0x2CA40D1Fu, 0, { "Purse" } },
	[PS_WORLD_CHALLENGE] = { 0, 0, { "WORLD CHALLENGE", "D\xC3\x89" "FI EN VILLE",
		"STADT-HERAUSFORDERUNG", "SFIDA IN CITT\xC3\x80", "DESAF\xC3\x8DO URBANO", "STADSUITDAGING",
		"STADSUTMANING", "BYUDFORDRING" } },
	[PS_CHALLENGE] = { 0, 0, { "CHALLENGE", "D\xC3\x89" "FI", "HERAUSFORDERUNG", "SFIDA",
		"DESAF\xC3\x8DO", "UITDAGING", "UTMANING", "UDFORDRING" } },
	[PS_VISUAL] = { 0, 0, { "VISUAL", "LOOK", "OPTIK", "ESTETICA", "EST\xC3\x89TICA",
		"UITERLIJK", "STIL", "STIL" } },
	[PS_COMPLETE] = { 0xB1EE4CA9u, 0, { "Complete" } },
	[PS_STAGE_FMT] = { 0x0A96B270u, 0, { "Stage %d" } },
	[PS_WORLD_RACES] = { 0, 0, { "WORLD RACES", "\xC3\x89PREUVES CLASSIQUES",
		"NORMALE EVENTS", "EVENTI GENERALI", "EVENTOS MUNDIALES", "WORLD EVENTS",
		"VANLIGA T\xC3\x84VLINGAR", "ALMINDELIGE L\xC3\x98" "B" } },
	[PS_SPONSOR_RACES] = { 0, 0, { "SPONSOR RACES", "COURSES SPONSOR", "SPONSORENRENNEN",
		"GARE SPONSOR", "CARRERAS PATROCINADAS", "SPONSORRACES", "SPONSORLOPP",
		"SPONSORL\xC3\x98" "B" } },
	[PS_URL_RACES] = { 0, 0, { "URL RACES", "COURSES CCU", "URL-RENNEN", "GARE URL",
		"CARRERAS URL", "URL-RACES", "URL-LOPP", "URL-L\xC3\x98" "B" } },
	[PS_DVDS_WON] = { 0, 0, { "DVDS WON", "DVD GAGN\xC3\x89S", "DVDS GEWONNEN",
		"DVD VINTI", "DVD GANADOS", "DVD'S GEWONNEN", "VUNNA DVD", "VUNDNE DVD'ER" } },
	[PS_NO_COVERS] = { 0, 0, { "No covers this stage",
		"Aucune jaquette \xC3\xA0 cette \xC3\xA9tape", "Keine Cover in dieser Stufe",
		"Nessuna copertina in questo livello", "Ninguna portada en esta etapa",
		"Geen covers in dit niveau", "Inga omslag p\xC3\xA5 den h\xC3\xA4r niv\xC3\xA5n",
		"Ingen covers p\xC3\xA5 dette niveau" } },
	[PS_WORLD_MAP] = { 0x9C07CA52u, 0, { "World Map" } },
	[PS_SHOW_ON_MAP] = { 0, 0, { "SHOW ON MAP", "AFFICHER SUR LA CARTE",
		"AUF KARTE ZEIGEN", "MOSTRA SU MAPPA", "MOSTRAR EN EL MAPA", "TONEN OP KAART",
		"VISA P\xC3\x85 KARTAN", "VIS P\xC3\x85 KORTET" } },
	[PS_SHOW_ALL] = { 0, 0, { "Show all", "Tout afficher", "Alle zeigen", "Mostra tutto",
		"Mostrar todo", "Alles tonen", "Visa alla", "Vis alle" } },
	[PS_HIDE_ALL] = { 0, 0, { "Hide all", "Tout masquer", "Alle ausblenden",
		"Nascondi tutto", "Ocultar todo", "Alles verbergen", "D\xC3\xB6lj alla", "Skjul alle" } },
	[PS_FILTERS_FMT] = { 0, 0, { "FILTERS  %d/%d", "FILTRES  %d/%d", "FILTER  %d/%d",
		"FILTRI  %d/%d", "FILTROS  %d/%d", "FILTERS  %d/%d", "FILTER  %d/%d", "FILTRE  %d/%d" } },
	[PS_MAILBOX] = { 0, 0, { "Mailbox", "Messagerie", "Postfach", "Posta", "Buz\xC3\xB3n",
		"Postvak", "Brevl\xC3\xA5" "da", "Postkasse" } },
	[PS_NO_MESSAGES] = { 0, 0, { "No messages", "Aucun message", "Keine Mitteilungen",
		"Nessun messaggio", "No hay mensajes", "Geen berichten", "Inga meddelanden",
		"Ingen beskeder" } },
	[PS_READ] = { 0, 0, { "Read", "Lire", "Lesen", "Leggi", "Leer", "Lezen", "L\xC3\xA4s",
		"L\xC3\xA6s" } },
	[PS_DELETE] = { 0x56B00632u, 0, { "Delete" } },
	[PS_INBOX] = { 0, 0, { "Inbox", "Messages", "Eingang", "In arrivo", "Entrada",
		"Inbox", "Inkorg", "Indbakke" } },
	[PS_FROM_FMT] = { 0x9F29D70Du, 0, { "From: %s" } },
	[PS_DRIVE] = { 0, 0, { "Drive", "Conduire", "Fahren", "Guida", "Conducir", "Rijden",
		"K\xC3\xB6r", "K\xC3\xB8r" } },
	[PS_LOT_BUY] = { 0, 0, { "BUY A CAR", "ACHETER UNE VOITURE", "WAGEN KAUFEN",
		"ACQUISTA UN'AUTO", "COMPRAR UN COCHE", "WAGEN KOPEN", "K\xC3\x96P EN BIL",
		"K\xC3\x98" "B EN BIL" } },
	[PS_LOT_PICK] = { 0, 0, { "PICK YOUR CAR TO TRADE",
		"CHOISIS LA VOITURE \xC3\x80 \xC3\x89" "CHANGER", "WAGEN ZUM EINTAUSCHEN W\xC3\x84HLEN",
		"SCEGLI L'AUTO DA SCAMBIARE", "ELIGE EL COCHE A CAMBIAR", "KIES DE WAGEN OM IN TE RUILEN",
		"V\xC3\x84LJ BIL ATT BYTA IN", "V\xC3\x86LG BILEN DU VIL BYTTE" } },
	[PS_LOT_TRADE_IN] = { 0, 0, { "TRADE IN", "REPRISE", "EINTAUSCH", "PERMUTA", "CAMBIO",
		"INRUIL", "INBYTE", "BYTTE" } },
	[PS_SLOT] = { 0x93475966u, 0, { "Slot" } },
	[PS_MY_CAR_FMT] = { 0, 0, { "MY CAR %d", "MA VOITURE %d", "MEIN WAGEN %d",
		"LA MIA AUTO %d", "MI COCHE %d", "MIJN WAGEN %d", "MIN BIL %d", "MIN BIL %d" } },
	[PS_CAREER_CAR_FMT] = { 0, 0, { "CAREER CAR %d", "VOITURE CARRI\xC3\x88RE %d",
		"KARRIEREWAGEN %d", "AUTO CARRIERA %d", "COCHE MODO CARRERA %d", "CARRI\xC3\x88REWAGEN %d",
		"KARRI\xC3\x84RBIL %d", "KARRIEREBIL %d" } },
	[PS_SPONSOR_CAR] = { 0, 0, { "SPONSOR CAR", "VOITURE SPONSOR", "SPONSORWAGEN",
		"AUTO SPONSOR", "COCHE PATROCINADO", "SPONSORWAGEN", "SPONSORBIL", "SPONSORBIL" } },
	[PS_STOCK] = { 0x310BF423u, 0, { "Stock" } },
	[PS_DRIVING] = { 0, 0, { "DRIVING", "AU VOLANT", "AKTUELL", "IN USO", "EN USO",
		"IN GEBRUIK", "I BRUK", "I BRUG" } },
	[PS_SELECT_CAR] = { 0xC0BEDF15u, 0, { "Select Car" } },
	[PS_BUY] = { 0, 0, { "Buy", "Acheter", "Kaufen", "Compra", "Comprar", "Kopen",
		"K\xC3\xB6p", "K\xC3\xB8" "b" } },
	[PS_TRADE] = { 0xE0F68E78u, 0, { "Trade Car" } },
	[PS_TRADE_IN] = { 0, 0, { "Trade In", "\xC3\x89" "changer", "Eintauschen", "Permuta",
		"Cambiar", "Inruilen", "Byt in", "Byt" } },
	[PS_CUSTOMIZE] = { 0xA53039A3u, 0, { "Customize" } },
	[PS_LOCKED] = { 0xC9F7F7E5u, 0, { "LOCKED" } },
	[PS_LOCKED_MIXED] = { 0, 0, { "Locked", "Verrouill\xC3\xA9", "Gesperrt", "Bloccato",
		"Bloqueado", "Vergrendeld", "L\xC3\xA5st", "L\xC3\xA5st" } },
	[PS_FITTED] = { 0, 0, { "FITTED", "MONT\xC3\x89", "MONTIERT", "MONTATO", "INSTALADO",
		"GEMONTEERD", "MONTERAD", "MONTERET" } },
	[PS_OWNED] = { 0x0371711Cu, 0, { "Owned" } },
	[PS_OPTION_FMT] = { 0, 0, { "Option %d", "Option %d", "Option %d", "Opzione %d",
		"Opci\xC3\xB3n %d", "Optie %d", "Alternativ %d", "Mulighed %d" } },
	[PS_INSTALL] = { 0x56DA42E0u, 0, { "Install" } },
	[PS_NONE] = { 0x001A076Fu, 0, { "None" } },
	[PS_T_TRACK_SELECT] = { 0x766A0EDFu, 0, { "Track Select" } },
	[PS_T_PERFORMANCE] = { 0xD6F5819Bu, 0, { "Performance" } },
	[PS_T_PAINT] = { 0x6A4C46A6u, 0, { "Paint" } },
	[PS_CHOOSE] = { 0, 0, { "Choose", "Choisir", "W\xC3\xA4hlen", "Scegli", "Elegir",
		"Kiezen", "V\xC3\xA4lj", "V\xC3\xA6lg" } },
	[PS_APPLY] = { 0, 0, { "Apply", "Appliquer", "Anwenden", "Applica", "Aplicar",
		"Toepassen", "Anv\xC3\xA4nd", "Anvend" } },
	[PS_T_PROFILES] = { 0, 0, { "Profiles", "Profils", "Profile", "Profili", "Perfiles",
		"Profielen", "Profiler", "Profiler" } },
	[PS_T_TRAX] = { 0x22FCCA3Cu, 0, { "EA Trax" } },
	[PS_RANDOM] = { 0xA9C9C8F7u, 0, { "Random" } },
	[PS_SEQUENTIAL] = { 0x4CA36B89u, 0, { "Sequential" } },
	[PS_PREVIEW] = { 0x78D5F70Fu, 0, { "Preview", "Aper\xC3\xA7u", "Vorschau", "Anteprima",
		"Previsualizar", "Voorbeeld", "F\xC3\xB6rhandsvisa", "Smugkig" } },
	[PS_MOVE] = { 0, 0, { "Move", "D\xC3\xA9placer", "Verschieben", "Sposta", "Mover",
		"Verplaatsen", "Flytta", "Flyt" } },
	[PS_RELEASE] = { 0, 0, { "Release", "L\xC3\xA2" "cher", "Loslassen", "Rilascia",
		"Soltar", "Loslaten", "Sl\xC3\xA4pp", "Slip" } },
	[PS_T_BRANDS] = { 0, 0, { "Brands", "Marques", "Marken", "Marche", "Marcas", "Merken",
		"M\xC3\xA4rken", "M\xC3\xA6rker" } },
	[PS_BOOST] = { 0, 0, { "Boost", "Pression turbo", "Ladedruck", "Pressione turbo",
		"Presi\xC3\xB3n turbo", "Turbodruk", "Laddtryck", "Ladetryk" } },
	[PS_TORQUE] = { 0x03CB927Eu, 0, { "Torque" } },
	[PS_TUNE_HINT] = { 0, 0, { "LEFT / RIGHT  point     UP / DOWN  value",
		"GAUCHE / DROITE  point     HAUT / BAS  valeur",
		"LINKS / RECHTS  Punkt     OBEN / UNTEN  Wert",
		"SINISTRA / DESTRA  punto     SU / GI\xC3\x99  valore",
		"IZQ. / DER.  punto     ARRIBA / ABAJO  valor",
		"LINKS / RECHTS  punt     OMHOOG / OMLAAG  waarde",
		"V\xC3\x84NSTER / H\xC3\x96GER  punkt     UPP / NED  v\xC3\xA4rde",
		"VENSTRE / H\xC3\x98JRE  punkt     OP / NED  v\xC3\xA6rdi" } },
	[PS_NITROUS] = { 0x135C04DAu, 0, { "Nitrous" } },
	[PS_POWER] = { 0x824BC854u, 0, { "Power" } },
	[PS_DURATION] = { 0x67E53439u, 0, { "Duration" } },
	[PS_DYNO_RUN] = { 0x436C91D4u, 0, { "Dyno Run" } },
	[PS_RPM_AXIS] = { 0x3389F685u, 0, { "RPM x1000" } },
	[PS_RESULTS] = { 0, 0, { "Results", "R\xC3\xA9sultats", "Ergebnisse", "Risultati",
		"Resultados", "Resultaten", "Resultat", "Resultater" } },
	[PS_STOP] = { 0, 0, { "Stop", "Arr\xC3\xAAter", "Stopp", "Ferma", "Detener",
		"Stoppen", "Stoppa", "Stop" } },
	[PS_CHART] = { 0, 0, { "Chart", "Courbes", "Diagramm", "Grafico", "Gr\xC3\xA1" "fica",
		"Grafiek", "Graf", "Graf" } },
	[PS_DYNO_RESULTS] = { 0x37B7246Au, 0, { "Dyno Results" } },
	[PS_THIS_RUN] = { 0, 0, { "THIS RUN", "CE TEST", "DIESER LAUF", "QUESTA PROVA",
		"ESTA PRUEBA", "DEZE RUN", "DENNA RUNDA", "DETTE L\xC3\x98" "B" } },
	[PS_LAST_RUN] = { 0, 0, { "LAST RUN", "DERNIER TEST", "LETZTER LAUF", "ULTIMA PROVA",
		"\xC3\x9ALTIMA PRUEBA", "VORIGE RUN", "F\xC3\x96RRA RUNDAN", "SIDSTE L\xC3\x98" "B" } },
	[PS_FIRST_RUN] = { 0xCB5E7E4Au, 0, { "First Run" } },
	[PS_TOP_SPEED] = { 0x0EF34382u, 0, { "TOP SPEED" } },
	[PS_OVERVIEW] = { 0, 0, { "Overview", "Vue d'ensemble", "\xC3\x9C" "bersicht",
		"Panoramica", "Vista general", "Overzicht", "\xC3\x96versikt", "Oversigt" } },
	[PS_DONE_TAG] = { 0, 0, { "DONE", "TERMIN\xC3\x89", "ERLEDIGT", "FATTO", "HECHO",
		"KLAAR", "KLAR", "KLARET" } },
	[PS_T_LAN_SERVERS] = { 0x4DE319CCu, 0, { "LAN Servers" } },
	[PS_T_LAN_NAME] = { 0, 0, { "LAN Name", "Nom r\xC3\xA9seau local", "LAN-Name",
		"Nome LAN", "Nombre LAN", "LAN-naam", "LAN-namn", "LAN-navn" } },
	[PS_T_LAN_LOBBY] = { 0xE50258B1u, 0, { "LAN Lobby" } },
	[PS_T_GAME_ROOM] = { 0, 0, { "Game Room", "Salle de jeu", "Spielraum", "Sala partita",
		"Sala de juego", "Spelruimte", "Spelrum", "Spillerum" } },
	[PS_CREATE] = { 0x55423473u, 0, { "Create" } },
	[PS_JOIN] = { 0, 0, { "Join", "Rejoindre", "Beitreten", "Unisciti", "Unirse",
		"Meedoen", "G\xC3\xA5 med", "Deltag" } },
	[PS_REFRESH] = { 0, 0, { "Refresh", "Actualiser", "Neu laden", "Aggiorna",
		"Actualizar", "Vernieuwen", "Uppdatera", "Opdater" } },
	[PS_FILTER] = { 0x3A36F0A8u, 0, { "Filter" } },
	[PS_LOOKING_SERVERS] = { 0, 0, { "Looking for servers...", "Recherche de serveurs...",
		"Suche nach Servern ...", "Ricerca server...", "Buscando servidores...",
		"Zoeken naar servers...", "S\xC3\xB6ker efter servrar...", "Leder efter servere..." } },
	[PS_NO_GAMES] = { 0, 0, { "No games yet", "Aucune partie", "Noch keine Spiele",
		"Ancora nessuna partita", "A\xC3\xBAn no hay partidas", "Nog geen spellen",
		"Inga matcher \xC3\xA4n", "Ingen spil endnu" } },
	[PS_YOUR_NAME] = { 0, 0, { "YOUR NAME", "VOTRE NOM", "DEIN NAME", "IL TUO NOME",
		"TU NOMBRE", "JOUW NAAM", "DITT NAMN", "DIT NAVN" } },
	[PS_TAP_NAME] = { 0, 0, { "Tap to type a name", "Touchez pour saisir un nom",
		"Tippen, um einen Namen einzugeben", "Tocca per inserire un nome",
		"Toca para escribir un nombre", "Tik om een naam te typen",
		"Tryck f\xC3\xB6r att skriva ett namn", "Tryk for at skrive et navn" } },
	[PS_OPEN_SLOT] = { 0, 0, { "Open", "Libre", "Frei", "Libero", "Libre", "Vrij",
		"Ledig", "Ledig" } },
	[PS_TRACK] = { 0xE1170A13u, 0, { "Track" } },
	[PS_LAPS] = { 0x48494E83u, 0, { "Laps" } },
	[PS_DIRECTION] = { 0, 0, { "DIRECTION", "SENS", "RICHTUNG", "DIREZIONE", "SENTIDO",
		"RICHTING", "RIKTNING", "RETNING" } },
	[PS_OPTIONS] = { 0xB34993C9u, 0, { "Options" } },
	[PS_OPT_GPS_COLOR] = { 0, 0, { "GPS Color", "Couleur GPS", "GPS-Farbe", "Colore GPS",
		"Color del GPS", "GPS-kleur", "GPS-f\xC3\xA4rg", "GPS-farve" } },
	[PS_OPT_DRAG_SHIFT] = { 0, 0, { "Drag Shift: ZL/ZR", "Vitesses en drag : ZL/ZR",
		"Drag-Schalten: ZL/ZR", "Cambio drag: ZL/ZR", "Cambio en drag: ZL/ZR",
		"Schakelen drag: ZL/ZR", "Dragv\xC3\xA4xling: ZL/ZR", "Drag-gearskift: ZL/ZR" } },
	[PS_COL_BLUE] = { 0, 0, { "Blue", "Bleu", "Blau", "Blu", "Azul", "Blauw",
		"Bl\xC3\xA5", "Bl\xC3\xA5" } },
	[PS_COL_CYAN] = { 0, 0, { "Cyan", "Cyan", "Cyan", "Ciano", "Cian", "Cyaan", "Cyan",
		"Cyan" } },
	[PS_COL_GREEN] = { 0, 0, { "Green", "Vert", "Gr\xC3\xBCn", "Verde", "Verde", "Groen",
		"Gr\xC3\xB6n", "Gr\xC3\xB8n" } },
	[PS_COL_LIME] = { 0, 0, { "Lime", "Vert citron", "Hellgr\xC3\xBCn", "Verde lime",
		"Lima", "Limoen", "Limegr\xC3\xB6n", "Limegr\xC3\xB8n" } },
	[PS_COL_YELLOW] = { 0, 0, { "Yellow", "Jaune", "Gelb", "Giallo", "Amarillo", "Geel",
		"Gul", "Gul" } },
	[PS_COL_ORANGE] = { 0, 0, { "Orange", "Orange", "Orange", "Arancione", "Naranja",
		"Oranje", "Orange", "Orange" } },
	[PS_COL_RED] = { 0, 0, { "Red", "Rouge", "Rot", "Rosso", "Rojo", "Rood",
		"R\xC3\xB6" "d", "R\xC3\xB8" "d" } },
	[PS_COL_PINK] = { 0, 0, { "Pink", "Rose", "Pink", "Rosa", "Rosa", "Roze", "Rosa",
		"Pink" } },
	[PS_COL_PURPLE] = { 0, 0, { "Purple", "Violet", "Lila", "Viola", "Morado", "Paars",
		"Lila", "Lilla" } },
	[PS_COL_WHITE] = { 0, 0, { "White", "Blanc", "Wei\xC3\x9F", "Bianco", "Blanco", "Wit",
		"Vit", "Hvid" } },
	[PS_OPT_ENTER_SHOP] = { 0, 0, { "Enter Shop / Challenge", "Atelier / D\xC3\xA9" "fi",
		"Shop / Herausfordern", "Negozio / Sfida", "Tienda / Desaf\xC3\xADo", "Shop / Uitdagen",
		"Butik / Utmana", "Butik / Udfordr" } },
	[PS_OPT_DELETE_ALL] = { 0, 0, { "Delete All Controls", "Effacer toutes les commandes",
		"Alle Tasten l\xC3\xB6schen", "Elimina tutti i comandi", "Borrar todos los controles",
		"Alle toewijzingen wissen", "Radera alla kontroller", "Slet alle styringer" } },
	[PS_OPT_3D_STYLE] = { 0, 0, { "3-D Style", "Style 3D", "3D-Stil", "Stile 3D",
		"Estilo 3D", "3D-stijl", "3D-stil", "3D-stil" } },
	[PS_OPT_3D_DEPTH] = { 0, 0, { "3-D Depth", "Profondeur 3D", "3D-Tiefe",
		"Profondit\xC3\xA0 3D", "Profundidad 3D", "3D-diepte", "3D-djup", "3D-dybde" } },
	[PS_OPT_3D_FOCUS] = { 0, 0, { "3-D Focus", "Mise au point 3D", "3D-Fokus", "Fuoco 3D",
		"Enfoque 3D", "3D-focus", "3D-fokus", "3D-fokus" } },
	[PS_OPT_3D_HUD] = { 0, 0, { "3-D HUD Depth", "Profondeur HUD 3D", "3D-HUD-Tiefe",
		"Profondit\xC3\xA0 HUD 3D", "Profundidad HUD 3D", "3D-HUD-diepte", "3D-HUD-djup",
		"3D-HUD-dybde" } },
	[PS_NOT_USED] = { 0xF8B4303Bu, 0, { "Not Used" } },
	[PS_3D_POP] = { 0, 0, { "Pop Out", "En relief", "Nach vorn", "In rilievo",
		"Hacia fuera", "Naar voren", "Ut\xC3\xA5t", "Udad" } },
	[PS_3D_INTO] = { 0, 0, { "Into Screen", "En profondeur", "Nach hinten",
		"Nello schermo", "Hacia dentro", "Het scherm in", "In\xC3\xA5t", "Indad" } },
	[PS_BTN_CPAD] = { 0, 0, { "Circle Pad", "Pad circulaire", "Schiebepad",
		"Pad scorrevole", "Bot\xC3\xB3n deslizante", "Schuifknop" } },
	[PS_BTN_DPAD_UP] = { 0, 0, { "D-Pad Up", "Croix haut", "Steuerkreuz oben", "Croce su",
		"Cruceta arriba", "D-pad omhoog", "Styrkors upp", "Styrekors op" } },
	[PS_BTN_DPAD_DOWN] = { 0, 0, { "D-Pad Down", "Croix bas", "Steuerkreuz unten",
		"Croce gi\xC3\xB9", "Cruceta abajo", "D-pad omlaag", "Styrkors ned", "Styrekors ned" } },
	[PS_BTN_DPAD_LEFT] = { 0, 0, { "D-Pad Left", "Croix gauche", "Steuerkreuz links",
		"Croce sinistra", "Cruceta izquierda", "D-pad links", "Styrkors v\xC3\xA4nster",
		"Styrekors venstre" } },
	[PS_BTN_DPAD_RIGHT] = { 0, 0, { "D-Pad Right", "Croix droite", "Steuerkreuz rechts",
		"Croce destra", "Cruceta derecha", "D-pad rechts", "Styrkors h\xC3\xB6ger",
		"Styrekors h\xC3\xB8jre" } },
};

// Room for a resolved retail string and for any entry's capitals; the longest
// retail text used is well under it (the caps of a long own string, such as
// the missing-map message, are never asked for).
#define PS_BUF 64

static const char* s_cur[PS_COUNT];
static char s_retail[PS_COUNT][PS_BUF];   // retail strings copied out of the table
static char s_caps[PS_COUNT][PS_BUF];

// Copies at most cap-1 bytes and never half a UTF-8 sequence.
static void copyUtf8(char* dst, const char* src, size_t cap) {
	size_t n = strlen(src);
	if (n >= cap) {
		n = cap - 1;
		while (n > 0 && ((u8)src[n] & 0xC0) == 0x80)
			n--;
	}
	memcpy(dst, src, n);
	dst[n] = '\0';
}

// The printf conversions of `s` in order ("%d%s"), "%%" left out.
static void conversions(const char* s, char* out, size_t cap) {
	size_t o = 0;
	for (const char* p = s; *p; p++) {
		if (*p != '%')
			continue;
		p++;
		if (*p == '%')
			continue;
		while (*p && strchr("-+ #0123456789.lhz", *p))
			p++;
		if (!*p)
			break;
		if (o + 1 < cap)
			out[o++] = *p;
	}
	out[o] = '\0';
}

static bool sameConversions(const char* a, const char* b) {
	char ca[16], cb[16];
	conversions(a, ca, sizeof(ca));
	conversions(b, cb, sizeof(cb));
	return strcmp(ca, cb) == 0;
}

static bool trimmable(const char* c, size_t n, size_t* len) {
	if (n >= 1 && (c[n - 1] == ' ' || c[n - 1] == ':')) {
		*len = 1;
		return true;
	}
	if (n >= 2 && (u8)c[n - 2] == 0xC2 && (u8)c[n - 1] == 0xA0) {
		*len = 2;
		return true;
	}
	return false;
}

// One retail string as the kit shows it: '^' breaks as spaces, the trailing
// ':' with its spaces and no-break spaces gone, the leading spaces too.
static void cleanRetail(const char* r, char* out, size_t cap) {
	while (*r == ' ')
		r++;
	copyUtf8(out, r, cap);
	for (char* p = out; *p; p++)
		if (*p == '^')
			*p = ' ';
	size_t n = strlen(out), k;
	while (trimmable(out, n, &k))
		out[n -= k] = '\0';
}

static void upperImpl(char* s, bool keepConversions) {
	for (u8* p = (u8*)s; *p; p++) {
		if (keepConversions && *p == '%') {
			// Over the flags, width and length to the conversion's letter.
			p++;
			while (*p && strchr("-+ #0123456789.lhz", (char)*p))
				p++;
			if (!*p)
				break;
			continue;
		}
		if (*p >= 'a' && *p <= 'z') {
			*p = (u8)(*p - 32);
		} else if (*p == 0xC3 && p[1]) {
			// U+00E0..U+00FE are their capitals + 0x20 (C3 A0..BE -> C3 80..9E),
			// except the division sign U+00F7; U+00DF (sharp s) and U+00FF stay.
			if (p[1] >= 0xA0 && p[1] <= 0xBE && p[1] != 0xB7)
				p[1] = (u8)(p[1] - 0x20);
			p++;
		} else if (*p == 0xC5 && p[1] == 0x93) {
			p[1] = 0x92;   // oe
			p++;
		}
	}
}

void portstr_upper(char* s) {
	if (s)
		upperImpl(s, false);
}

void portstr_set_language(int langIndex, const char* langBinPath) {
	if (langIndex < 0 || langIndex >= PORT_LANG_COUNT)
		langIndex = 0;
	LangTable lang;
	const bool have = langBinPath && langbin_load(langBinPath, &lang);
	for (int i = 0; i < PS_COUNT; i++) {
		const PortStrDef* d = &kStr[i];
		const char* own = d->t[langIndex] ? d->t[langIndex] : d->t[0];
		s_cur[i] = own;
		if (d->hash && have) {
			const char* r = langbin_lookup(&lang, d->hash);
			if (r) {
				char c[PS_BUF];
				cleanRetail(r, c, sizeof(c));
				// An empty string is a hole in that language's table, but for
				// the few entries where empty is the answer; a format that
				// does not take the English's arguments would print garbage.
				if ((c[0] || (d->flags & PSF_EMPTY_OK)) && sameConversions(c, d->t[0])) {
					memcpy(s_retail[i], c, sizeof(c));
					s_cur[i] = s_retail[i];
				}
			}
		}
		copyUtf8(s_caps[i], s_cur[i], sizeof(s_caps[i]));
		upperImpl(s_caps[i], strchr(d->t[0], '%') != NULL);
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

const char* portstr_caps(PortStr id) {
	if ((int)id < 0 || id >= PS_COUNT)
		return "";
	if (!s_cur[id])
		return kStr[id].t[0];   // before the first language: as written
	return s_caps[id];
}
