#pragma once

#include <citro2d.h>

#include "fontmip.h"
#include <math.h>
#include <stdbool.h>

// C2D_TextGetDimensions as C2D_DrawText draws. citro2d 1.6.0 normalises every
// font to a 30-pixel cell: C2D_DrawText multiplies both scales by the font's
// textScale (30 / its cell height) before placing a glyph, but
// C2D_TextGetDimensions only applies it to the height -- a C2D_Text's width is
// rescaled at parse time for the system font alone (text.c, "text->width *=
// s_textScale"). conduit.bcfnt's cells are 44 pixels tall, so the raw call
// reports every Conduit string 1.47x wider than it is drawn. Text parsed with
// the system font (font NULL) comes back as it was.
//
// citro2d 1.7.0 fixed that: C2D_TextFontParse now multiplies the width by the
// font's own textScale too (text.o, C2D_TextFontParseLine +0x17c), so the
// correction above applied a second time made every custom-font string
// measure ~60% of its drawn width -- which threw off every layout built on
// it. Which library this was linked against is found out once per font, by
// parsing "M" and comparing the reported width with the glyph's own advance.
static inline bool textWidthNeedsFontScale(C2D_Font font) {
	static C2D_Font seen[4];
	static signed char need[4];
	for (int i = 0; i < 4; i++)
		if (seen[i] == font)
			return need[i] > 0;
	bool res = true; // 1.6.0's behaviour, the one this port was written on
	C2D_TextBuf b = C2D_TextBufNew(4);
	if (b) {
		C2D_Text probe;
		C2D_TextFontParse(&probe, font, b, "M");
		float w = 0.0f;
		C2D_TextGetDimensions(&probe, 1.0f, 1.0f, &w, NULL);
		const charWidthInfo_s* cw = C2D_FontGetCharWidthInfo(font,
			C2D_FontGlyphIndexFromCodePoint(font, 'M'));
		const float ts = fontmip_text_scale(font);
		if (cw && ts > 0.0f && ts != 1.0f) {
			const float raw = (float)cw->charWidth;
			res = fabsf(w - raw) < fabsf(w - raw * ts);
		}
		C2D_TextBufDelete(b);
	}
	for (int i = 0; i < 4; i++)
		if (!seen[i]) {
			seen[i] = font;
			need[i] = res ? 1 : -1;
			break;
		}
	return res;
}

static inline void textDims(const C2D_Text* t, float scaleX, float scaleY,
		float* outW, float* outH) {
	float w = 0.0f, h = 0.0f;
	C2D_TextGetDimensions(t, scaleX, scaleY, &w, &h);
	// The font's own textScale rather than 30 / its cell height: the UI font's
	// is set below that (main.c, the cap-height match, TASKS.md #425).
	if (t->font && textWidthNeedsFontScale(t->font)) {
		const float ts = fontmip_text_scale(t->font);
		const FINF_s* fi = C2D_FontGetInfo(t->font);
		if (ts > 0.0f)
			w *= ts;
		else if (fi && fi->tglp && fi->tglp->cellHeight)
			w *= 30.0f / (float)fi->tglp->cellHeight;
	}
	if (outW)
		*outW = w;
	if (outH)
		*outH = h;
}
