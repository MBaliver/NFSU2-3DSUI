// PORT: antialiased flat shapes for the touch UI -- see uiaa.h.
#include "uiaa.h"

#include <citro2d.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static unsigned s_tris;

static void tri(float x0, float y0, u32 c0, float x1, float y1, u32 c1, float x2, float y2, u32 c2, float z) {
	C2D_DrawTriangle(x0, y0, c0, x1, y1, c1, x2, y2, c2, z);
	s_tris++;
}

u32 uiaaClear(u32 c) {
	return c & 0x00FFFFFFu;
}

u32 uiaaScale(u32 c, float k) {
	float a = (float)(c >> 24) * k;
	a = a < 0.0f ? 0.0f : a > 255.0f ? 255.0f : a;
	return (c & 0x00FFFFFFu) | ((u32)(a + 0.5f) << 24);
}

u32 uiaaMix(u32 a, u32 b, float t) {
	t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
	u32 out = 0;
	for (int s = 0; s < 32; s += 8) {
		const float ca = (float)((a >> s) & 0xFF), cb = (float)((b >> s) & 0xFF);
		out |= (u32)(ca + (cb - ca) * t + 0.5f) << s;
	}
	return out;
}

unsigned uiaaTakeTris(void) {
	const unsigned n = s_tris;
	s_tris = 0;
	return n;
}

void uiaaStrip(const float* ax, const float* ay, u32 ca, const float* bx, const float* by, u32 cb, int n,
		bool closed, float z) {
	const int m = closed ? n : n - 1;
	for (int i = 0; i < m; i++) {
		const int j = (i + 1) % n;
		tri(ax[i], ay[i], ca, ax[j], ay[j], ca, bx[j], by[j], cb, z);
		tri(ax[i], ay[i], ca, bx[j], by[j], cb, bx[i], by[i], cb, z);
	}
}

void uiaaFan(float cx, float cy, const float* px, const float* py, int n, const u32* cols, u32 col, float z) {
	if (n < 3)
		return;
	if (n > UIAA_MAX)
		n = UIAA_MAX;
	// Outward normals of the edges; a zero-length edge (a corner with no
	// radius repeats its point) borrows its neighbour's.
	float area = 0.0f;
	for (int i = 0; i < n; i++) {
		const int j = (i + 1) % n;
		area += px[i] * py[j] - px[j] * py[i];
	}
	const float sgn = area >= 0.0f ? 1.0f : -1.0f;
	float nx[UIAA_MAX], ny[UIAA_MAX];
	bool ok[UIAA_MAX];
	bool any = false;
	for (int i = 0; i < n; i++) {
		const int j = (i + 1) % n;
		const float dx = px[j] - px[i], dy = py[j] - py[i], l = sqrtf(dx * dx + dy * dy);
		ok[i] = l > 1e-4f;
		nx[i] = ok[i] ? sgn * dy / l : 0.0f;
		ny[i] = ok[i] ? -sgn * dx / l : 0.0f;
		any |= ok[i];
	}
	if (!any)
		return;
	// Each point moves along its corner's mitre, so both edges move UIAA_F;
	// a needle-sharp corner is capped rather than shot out.
	float ix[UIAA_MAX], iy[UIAA_MAX], ox[UIAA_MAX], oy[UIAA_MAX];
	for (int i = 0; i < n; i++) {
		int a = (i + n - 1) % n, b = i;
		while (!ok[a]) a = (a + n - 1) % n;
		while (!ok[b]) b = (b + 1) % n;
		float mx = nx[a] + nx[b], my = ny[a] + ny[b];
		float d = 1.0f + nx[a] * nx[b] + ny[a] * ny[b];
		if (d < 0.32f)   // past ~2.5x
			d = 0.32f;
		mx = mx / d * UIAA_F;
		my = my / d * UIAA_F;
		ix[i] = px[i] - mx; iy[i] = py[i] - my;
		ox[i] = px[i] + mx; oy[i] = py[i] + my;
	}
	for (int i = 0; i < n; i++) {
		const int j = (i + 1) % n;
		const u32 c = cols ? cols[i] : col, c0 = uiaaClear(c);
		tri(cx, cy, c, ix[i], iy[i], c, ix[j], iy[j], c, z);
		tri(ix[i], iy[i], c, ix[j], iy[j], c, ox[j], oy[j], c0, z);
		tri(ix[i], iy[i], c, ox[j], oy[j], c0, ox[i], oy[i], c0, z);
	}
}

void uiaaPoly(const float* px, const float* py, int n, u32 col, float z) {
	float cx = 0.0f, cy = 0.0f;
	for (int i = 0; i < n; i++) {
		cx += px[i];
		cy += py[i];
	}
	uiaaFan(cx / (float)n, cy / (float)n, px, py, n, NULL, col, z);
}

void uiaaTri(float x0, float y0, float x1, float y1, float x2, float y2, u32 col, float z) {
	const float px[3] = { x0, x1, x2 }, py[3] = { y0, y1, y2 };
	uiaaPoly(px, py, 3, col, z);
}

void uiaaRect(float x, float y, float w, float h, u32 col, float z) {
	// Thinner than the fade: draw it a pixel thick and as faint as it is thin.
	if (w < 2.0f * UIAA_F) { col = uiaaScale(col, w / (2.0f * UIAA_F)); x -= UIAA_F - w * 0.5f; w = 2.0f * UIAA_F; }
	if (h < 2.0f * UIAA_F) { col = uiaaScale(col, h / (2.0f * UIAA_F)); y -= UIAA_F - h * 0.5f; h = 2.0f * UIAA_F; }
	const float px[4] = { x, x + w, x + w, x }, py[4] = { y, y, y + h, y + h };
	uiaaFan(x + w * 0.5f, y + h * 0.5f, px, py, 4, NULL, col, z);
}

void uiaaLine(float x0, float y0, float x1, float y1, float thick, u32 col, float z) {
	const float dx = x1 - x0, dy = y1 - y0, l = sqrtf(dx * dx + dy * dy);
	if (l < 1e-4f)
		return;
	if (thick < 2.0f * UIAA_F) {
		col = uiaaScale(col, thick / (2.0f * UIAA_F));
		thick = 2.0f * UIAA_F;
	}
	const float nx = -dy / l * thick * 0.5f, ny = dx / l * thick * 0.5f;
	const float px[4] = { x0 + nx, x1 + nx, x1 - nx, x0 - nx };
	const float py[4] = { y0 + ny, y1 + ny, y1 - ny, y0 - ny };
	uiaaFan((x0 + x1) * 0.5f, (y0 + y1) * 0.5f, px, py, 4, NULL, col, z);
}

void uiaaArc(float cx, float cy, float r0, float r1, float a0, float a1, int seg, u32 col, float z) {
	if (seg < 1)
		seg = 1;
	if (seg > UIAA_MAX - 1)
		seg = UIAA_MAX - 1;
	// Across the ring: clear, solid, solid, clear -- the solid pair meeting
	// in the middle, fainter, when the ring is thinner than the fade.
	float rr[4] = { r0 - UIAA_F, r0 + UIAA_F, r1 - UIAA_F, r1 + UIAA_F };
	u32 cc = col;
	if (r1 - r0 < 2.0f * UIAA_F) {
		rr[1] = rr[2] = (r0 + r1) * 0.5f;
		cc = uiaaScale(col, (r1 - r0) / (2.0f * UIAA_F));
	}
	if (rr[0] < 0.0f)
		rr[0] = 0.0f;
	const u32 cl = uiaaClear(cc);
	const u32 rc[4] = { cl, cc, cc, cl };
	const bool whole = fabsf(a1 - a0) >= 2.0f * (float)M_PI - 1e-3f;
	// An open arc's ends are cut square and faded across half a pixel each
	// way, measured on the middle of the ring.
	const float dir = a1 >= a0 ? 1.0f : -1.0f;
	const float rm = (r0 + r1) * 0.5f > 1.0f ? (r0 + r1) * 0.5f : 1.0f;
	const float da = whole ? 0.0f : dir * UIAA_F / rm;
	const float b0 = a0 + da, b1 = a1 - da;
	const int n = whole ? seg : seg + 1;
	float x[4][UIAA_MAX], y[4][UIAA_MAX];
	for (int i = 0; i < n; i++) {
		const float a = b0 + (b1 - b0) * (float)i / (float)seg, ca = cosf(a), sa = sinf(a);
		for (int k = 0; k < 4; k++) {
			x[k][i] = cx + rr[k] * ca;
			y[k][i] = cy + rr[k] * sa;
		}
	}
	for (int k = 0; k < 3; k++)
		if (rr[k + 1] > rr[k])
			uiaaStrip(x[k], y[k], rc[k], x[k + 1], y[k + 1], rc[k + 1], n, whole, z);
	if (whole)
		return;
	// The ends: from the cut, out half a pixel to clear.
	const float ends[2][2] = { { b0, a0 - da }, { b1, a1 + da } };
	for (int e = 0; e < 2; e++) {
		const float ci = cosf(ends[e][0]), si = sinf(ends[e][0]), co = cosf(ends[e][1]), so = sinf(ends[e][1]);
		for (int k = 0; k < 3; k++) {
			if (rr[k + 1] <= rr[k])
				continue;
			const float ax[2] = { cx + rr[k] * ci, cx + rr[k + 1] * ci }, ay[2] = { cy + rr[k] * si, cy + rr[k + 1] * si };
			const float bx[2] = { cx + rr[k] * co, cx + rr[k + 1] * co }, by[2] = { cy + rr[k] * so, cy + rr[k + 1] * so };
			// The inner edge keeps the ring's profile; the outer one is clear.
			tri(ax[0], ay[0], rc[k], ax[1], ay[1], rc[k + 1], bx[1], by[1], cl, z);
			tri(ax[0], ay[0], rc[k], bx[1], by[1], cl, bx[0], by[0], cl, z);
		}
	}
}
