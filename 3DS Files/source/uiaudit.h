#pragma once

// PORT, dev only: DbgUiAudit=1 (config) logs, once each, every lower-screen
// touch target under UIAUDIT_TOUCH_MIN px a side and every string drawn under
// UIAUDIT_TEXT_MIN px, with the main.c line (or the .fng screen and widget)
// that made it -- the audit behind the text / target floors. Off, it costs a
// flag test.

#include <stdbool.h>

#define UIAUDIT_TOUCH_MIN 32.0f
#define UIAUDIT_TEXT_MIN  14.0f

extern bool g_uiAudit;

void uiaudit_touch(int line, const char* kind, float w, float h);
void uiaudit_text(int line, float px, const char* sample);
void uiaudit_fng(const char* screen, int widget, float px);
void uiaudit_fng_touch(const char* screen, int widget, float w, float h);
