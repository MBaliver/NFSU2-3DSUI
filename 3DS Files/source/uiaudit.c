#include "uiaudit.h"

#include <stdio.h>
#include <string.h>

#include "dbglog.h"

// See uiaudit.h.

bool g_uiAudit;

#define SEEN_MAX 512
static unsigned s_seen[SEEN_MAX];
static int s_seenN;

static unsigned hashStr(unsigned h, const char* s) {
	while (s && *s)
		h = h * 31u + (unsigned char)*s++;
	return h;
}

// True the first time `key` is seen.
static bool firstTime(unsigned key) {
	for (int i = 0; i < s_seenN; i++)
		if (s_seen[i] == key)
			return false;
	if (s_seenN < SEEN_MAX)
		s_seen[s_seenN++] = key;
	return true;
}

void uiaudit_touch(int line, const char* kind, float w, float h) {
	if (!g_uiAudit || (w >= UIAUDIT_TOUCH_MIN && h >= UIAUDIT_TOUCH_MIN))
		return;
	if (!firstTime(hashStr(0x7001u + (unsigned)line * 977u + (unsigned)(w + 0.5f) * 131u +
			(unsigned)(h + 0.5f), kind)))
		return;
	char b[96];
	snprintf(b, sizeof(b), "touch %s main.c:%d %.0fx%.0f", kind, line, (double)w, (double)h);
	dbglog_line("ui audit: ", b);
}

void uiaudit_text(int line, float px, const char* sample) {
	if (!g_uiAudit || px >= UIAUDIT_TEXT_MIN)
		return;
	if (!firstTime(0x7E00u + (unsigned)line * 7919u + (unsigned)(px * 10.0f + 0.5f)))
		return;
	char b[128];
	snprintf(b, sizeof(b), "text main.c:%d %.1f px \"%.40s\"", line, (double)px, sample ? sample : "");
	dbglog_line("ui audit: ", b);
}

void uiaudit_fng(const char* screen, int widget, float px) {
	if (!g_uiAudit || px >= UIAUDIT_TEXT_MIN)
		return;
	if (!firstTime(hashStr(0xF00u + (unsigned)widget * 613u + (unsigned)(px * 10.0f + 0.5f), screen)))
		return;
	char b[128];
	snprintf(b, sizeof(b), "text %s widget %d %.1f px", screen ? screen : "?", widget, (double)px);
	dbglog_line("ui audit: ", b);
}

void uiaudit_fng_touch(const char* screen, int widget, float w, float h) {
	if (!g_uiAudit || (w >= UIAUDIT_TOUCH_MIN && h >= UIAUDIT_TOUCH_MIN))
		return;
	if (!firstTime(hashStr(0xFA0u + (unsigned)widget * 389u, screen)))
		return;
	char b[128];
	snprintf(b, sizeof(b), "touch %s widget %d %.0fx%.0f", screen ? screen : "?", widget,
		(double)w, (double)h);
	dbglog_line("ui audit: ", b);
}
