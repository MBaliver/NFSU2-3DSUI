#ifndef NFSU2_UIAA_H
#define NFSU2_UIAA_H

// PORT: antialiased flat shapes for the touch UI.
//
// citro2d fills triangles with hard pixel edges, while the UI font is a
// filtered texture whose edges ramp over about a pixel. Next to the text, a
// drawn icon read whiter and jagged (the map's X, + and - against "Garage").
// Every shape here is its outline pulled in by half a pixel, filled solid,
// plus a strip a pixel wide round it that fades from the colour to clear --
// roughly the coverage each pixel would get, without multisampling. An edge
// that lies on a whole pixel stays crisp.
//
// Pieces share edges only, so a translucent colour still blends once.
// citro2d's own circles are already soft-edged and are left to it.

#include <3ds.h>
#include <stdbool.h>

#define UIAA_F   0.5f   // half the fade's width, px
#define UIAA_MAX 64     // most points a polygon may have

// A polygon whose every edge faces (cx, cy) from inside: convex, or a star
// round that point. Either winding. cols[i] colours the wedge from point i to
// point i + 1; NULL colours it all `col`.
void uiaaFan(float cx, float cy, const float* px, const float* py, int n, const u32* cols, u32 col, float z);
// A convex polygon, fanned from its centroid.
void uiaaPoly(const float* px, const float* py, int n, u32 col, float z);
void uiaaTri(float x0, float y0, float x1, float y1, float x2, float y2, u32 col, float z);
void uiaaRect(float x, float y, float w, float h, u32 col, float z);
// A straight stroke `thick` wide with square ends, like C2D_DrawLine's.
void uiaaLine(float x0, float y0, float x1, float y1, float thick, u32 col, float z);
// A ring between radii r0 < r1 from angle a0 to a1 (screen radians: 0 is
// right, growing clockwise) in `seg` steps; square, faded ends. A whole turn
// closes on itself.
void uiaaArc(float cx, float cy, float r0, float r1, float a0, float a1, int seg, u32 col, float z);
// The quads between two outlines of n points: ca along a, cb along b.
void uiaaStrip(const float* ax, const float* ay, u32 ca, const float* bx, const float* by, u32 cb, int n,
	bool closed, float z);

u32 uiaaClear(u32 c);                  // c with no alpha
u32 uiaaMix(u32 a, u32 b, float t);    // a..b, per channel
u32 uiaaScale(u32 c, float k);         // c's alpha times k

// Triangles drawn since the last call (development aid for the vertex budget).
unsigned uiaaTakeTris(void);

#endif
