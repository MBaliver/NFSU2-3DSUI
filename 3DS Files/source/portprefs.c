// PORT: the port's per-profile settings file -- see portprefs.h.
#include "portprefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

const PortLang kPortLangs[PORT_LANG_COUNT] = {
	{ "EN", "English", "English" },
	{ "FR", "French",  "Fran\xC3\xA7" "ais" },
	{ "DE", "German",  "Deutsch" },
	{ "IT", "Italian", "Italiano" },
	{ "ES", "Spanish", "Espa\xC3\xB1" "ol" },
	{ "NL", "Dutch",   "Nederlands" },
	{ "SV", "Swedish", "Svenska" },
	{ "DA", "Danish",  "Dansk" },
};

int portprefs_lang_index(const char* s) {
	if (!s || !s[0])
		return -1;
	for (int i = 0; i < PORT_LANG_COUNT; i++)
		if (strcasecmp(s, kPortLangs[i].code) == 0 || strcasecmp(s, kPortLangs[i].file) == 0 ||
				strcasecmp(s, kPortLangs[i].native) == 0)
			return i;
	return -1;
}

void portprefs_defaults(PortPrefs* p) {
	memset(p, 0, sizeof(*p));
}

static char* trim(char* s) {
	while (*s == ' ' || *s == '\t')
		s++;
	char* e = s + strlen(s);
	while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n'))
		*--e = '\0';
	return s;
}

bool portprefs_read(const char* path, PortPrefs* p) {
	FILE* f = fopen(path, "r");
	if (!f)
		return false;
	char line[160];
	while (fgets(line, sizeof(line), f)) {
		char* k = trim(line);
		if (!k[0] || k[0] == '#')
			continue;
		char* eq = strchr(k, '=');
		if (!eq)
			continue;
		*eq = '\0';
		k = trim(k);
		const char* v = trim(eq + 1);
		if (strcasecmp(k, "Language") == 0) {
			const int i = portprefs_lang_index(v);
			if (i >= 0)
				p->language = i;
		} else if (strcasecmp(k, "LargeText") == 0) {
			p->largeText = atoi(v) != 0;
		} else if (strcasecmp(k, "HighContrast") == 0) {
			p->highContrast = atoi(v) != 0;
		}
	}
	fclose(f);
	return true;
}

bool portprefs_write(const char* path, const PortPrefs* p) {
	// Whole, through a temporary: a save cut short leaves the old file.
	char tmp[320];
	snprintf(tmp, sizeof(tmp), "%s.tmp", path);
	FILE* f = fopen(tmp, "w");
	if (!f)
		return false;
	const int li = p->language >= 0 && p->language < PORT_LANG_COUNT ? p->language : 0;
	fprintf(f, "# NFSU2-3DS port settings for this profile (the save itself is retail's format)\n");
	fprintf(f, "Language=%s\n", kPortLangs[li].code);
	fprintf(f, "LargeText=%d\n", p->largeText ? 1 : 0);
	fprintf(f, "HighContrast=%d\n", p->highContrast ? 1 : 0);
	const bool ok = fclose(f) == 0;
	if (!ok) {
		remove(tmp);
		return false;
	}
	remove(path);
	return rename(tmp, path) == 0;
}

void portprefs_path_for(const char* savePath, bool legacy, char* out, size_t cap) {
	if (legacy) {
		snprintf(out, cap, "%s.port.txt", savePath);
		return;
	}
	const char* slash = strrchr(savePath, '/');
	const int dirLen = slash ? (int)(slash - savePath) : 0;
	snprintf(out, cap, "%.*s/port.txt", dirLen, savePath);
}

void portprefs_last_path(const char* saveRoot, char* out, size_t cap) {
	snprintf(out, cap, "%s/port_last.txt", saveRoot);
}
