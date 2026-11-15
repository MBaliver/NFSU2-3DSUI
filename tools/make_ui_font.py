"""Rebuild 3DS Files/data/helvetica.bcfnt, the UI font (TASKS.md #425), from the
user's Helvetica 67 Condensed Medium Oblique .ttf in the repo root.

The .ttf stores its descender as +176 in both hhea and OS/2 (typo), where the
OpenType spec wants it negative. mkbcfnt then takes ascender - descender =
714 - 176 as the line height and writes a line feed of 20 px instead of 33,
and every point-size text size in the port (g_fontUnitPx, the HUD, the bank
strip, the chyron, the title screen) came out 1.7x too big. This fixes the
sign in a temporary copy before converting; nothing else in the font changes.

Then (#529) the glyph sheet is laid out again, one line of glyphs to a
1024x64 sheet and a 58 px cell stride, by relayout(). mkbcfnt packs the
cells 1 px apart, and the port mipmaps the sheet (fontmip.c, levels 0..3):
at the small sizes text is drawn at, a glyph's quad then picked up the
glyph above it in the sheet as faint lines over its top (worst under '['),
and the one beside it at its edges. The glyph indices, widths and CMAP stay
as they are; only TGLP's cell width, row count, line count, sheet height,
sheet size and sheet count change, which citro2d reads as it always does.

Usage (from the repo root, PowerShell):  python tools/make_ui_font.py
"""
import os
import subprocess
import sys
import tempfile

from fontTools.ttLib import TTFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TTF = os.path.join(ROOT, "Helvetica67-CondensedMedium MediumItalic.ttf")
OUT = os.path.join(ROOT, "3DS Files", "data", "helvetica.bcfnt")
# DEVKITPRO is often MSYS-style (/opt/devkitpro), which Windows cannot open.
MKBCFNT = next((p for p in (
    os.path.join(os.environ.get("DEVKITPRO", ""), "tools", "bin", "mkbcfnt.exe"),
    r"C:\devkitPro\tools\bin\mkbcfnt.exe") if os.path.isfile(p)), "mkbcfnt")
SIZE = "28"  # a 41x44 cell, the size conduit.bcfnt's was

# The Helvetica .ttf is Latin only, so a Russian English.bin (the localised
# PC releases replace that file) drew every letter as '?'. Its Cyrillic comes
# from Liberation Sans Italic (SIL Open Font License 1.1), the copy every
# devkitPro install has in its custom-font example: only U+0400..U+045F and
# '№', narrowed and scaled to Helvetica 67's cap height and capital widths
# (measured on the Latin capitals both fonts share), with Helvetica's own
# ascent and descent so the cell and line feed stay as they were. mkbcfnt
# takes the second font only for codepoints the first lacks.
CYR_TTF = next((p for p in (
    os.path.join(os.environ.get("DEVKITPRO", ""), "examples", "3ds", "graphics", "printing",
                 "custom-font", "gfx", "liberationitalic.ttf"),
    r"C:\devkitPro\examples\3ds\graphics\printing\custom-font\gfx\liberationitalic.ttf")
    if os.path.isfile(p)), None)
CYRILLIC = list(range(0x0400, 0x0460)) + [0x2116]
MATCH = "ABCEHKMOPTX"   # Latin capitals drawn the same as Cyrillic ones


def _cap_and_width(font):
    gs, cm, upm = font.getGlyphSet(), font.getBestCmap(), font["head"].unitsPerEm
    from fontTools.pens.boundsPen import BoundsPen
    pen = BoundsPen(gs)
    gs[cm[ord("H")]].draw(pen)
    cap = pen.bounds[3] / upm
    adv = sum(font["hmtx"][cm[ord(c)]][0] for c in MATCH) / len(MATCH) / upm
    return cap, adv


def cyrillic_font(ui, src, dst):
    """Write `dst`: `src`'s Cyrillic only, in `ui`'s proportions."""
    from fontTools import subset
    from fontTools.ttLib.scaleUpem import scale_upem
    f = TTFont(src)
    cap_ui, adv_ui = _cap_and_width(ui)
    cap_s, adv_s = _cap_and_width(f)
    opts = subset.Options()
    opts.hinting = False
    opts.layout_features = []
    opts.name_IDs = ["*"]
    sub = subset.Subsetter(opts)
    sub.populate(unicodes=CYRILLIC)
    sub.subset(f)
    upm = ui["head"].unitsPerEm
    scale_upem(f, upm)
    sy = cap_ui / cap_s
    sx = adv_ui / adv_s
    # Helvetica's own ink extent: a glyph past it (Й's breve, Щ's tail) would
    # make mkbcfnt's cell taller than 44, so the part above the cap height or
    # below the baseline is pressed into it instead.
    ug, ucm = ui["glyf"], ui.getBestCmap()
    hi = max(ug[n].yMax for n in ucm.values() if hasattr(ug[n], "yMax"))
    lo = min(ug[n].yMin for n in ucm.values() if hasattr(ug[n], "yMin"))
    cap = cap_ui * upm
    glyf, hmtx = f["glyf"], f["hmtx"]
    order = f.getGlyphOrder()
    # Composites last: their bounds come from the simple glyphs they use.
    for name in sorted(order, key=lambda n: glyf[n].isComposite()):
        g = glyf[name]
        if g.isComposite():
            for c in g.components:
                c.x, c.y = round(c.x * sx), round(c.y * sy)
        elif g.numberOfContours > 0:
            g.coordinates.scale((sx, sy))
            g.recalcBounds(glyf)
            top = hi / g.yMax if g.yMax > hi else 1.0
            low = lo / g.yMin if g.yMin < lo else 1.0
            if top != 1.0 or low != 1.0:
                co = g.coordinates
                for i, (x, y) in enumerate(co):
                    if y > cap and top != 1.0:
                        y = cap + (y - cap) * (hi - cap) / (g.yMax - cap)
                    elif y < 0:
                        y = y * low
                    co[i] = (x, y)
            g.coordinates.toInt()
        g.recalcBounds(glyf)
        adv, _lsb = hmtx[name]
        hmtx[name] = (round(adv * sx), getattr(g, "xMin", 0))
    for table, fields in (("hhea", ("ascent", "descent", "lineGap")),
                          ("OS/2", ("sTypoAscender", "sTypoDescender", "sTypoLineGap",
                                    "usWinAscent", "usWinDescent"))):
        for fld in fields:
            setattr(f[table], fld, getattr(ui[table], fld))
    # A modified OFL font may not keep its Reserved Font Name.
    for rec in f["name"].names:
        if rec.nameID in (1, 3, 4, 6, 16, 17):
            rec.string = "NFSU2-3DS UI Cyrillic"
    f.save(dst)
    return sx, sy


def main():
    t = TTFont(TTF)
    for table, field in (("hhea", "descent"), ("OS/2", "sTypoDescender")):
        v = getattr(t[table], field)
        if v > 0:
            setattr(t[table], field, -v)
    with tempfile.TemporaryDirectory() as d:
        fixed = os.path.join(d, "ui.ttf")
        t.save(fixed)
        inputs = [fixed]
        args = []
        cyr = sys.argv[1] if len(sys.argv) > 1 else CYR_TTF
        if cyr:
            cyr_out = os.path.join(d, "cyr.ttf")
            sx, sy = cyrillic_font(t, cyr, cyr_out)
            inputs.append(cyr_out)
            # Helvetica's own codepoints plus the Cyrillic, nothing else.
            wl = os.path.join(d, "whitelist.txt")
            with open(wl, "w") as w:
                w.write(" ".join(str(c) for c in sorted(set(t.getBestCmap()) | set(CYRILLIC))))
            args = ["-w", wl]
            print("Cyrillic from %s (x %.3f, y %.3f)" % (cyr, sx, sy))
        else:
            print("warning: no Liberation Sans Italic (devkitPro's custom-font example) -- "
                  "Latin only; pass a Cyrillic .ttf as the argument", file=sys.stderr)
        tmp = OUT + ".tmp"
        subprocess.run([MKBCFNT, "-s", SIZE] + args + ["-o", tmp] + inputs, check=True)
        relayout(tmp)
        os.replace(tmp, OUT)
    b = open(OUT, "rb").read()
    finf = int.from_bytes(b[6:8], "little")
    print("wrote %s: %d bytes, line feed %d" % (OUT, len(b), b[finf + 9]))
    for name in ("helvetica.bcfnt", "conduit.bcfnt"):
        print("  %s: H is %d px tall in its cell (main.c's cap-height match uses 27 and 24)"
              % (name, cap_ink(os.path.join(os.path.dirname(OUT), name))))
    return 0


STRIDE_X = 58     # cell stride across: 17 px between two glyphs' ink (41 px cells)
SHEET_H = 64      # one line of 45 px cells, 19 blank rows under it


def _morton(x, y):
    m = 0
    for i in range(3):
        m |= ((x >> i) & 1) << (2 * i) | ((y >> i) & 1) << (2 * i + 1)
    return m


def relayout(path):
    """One line of glyphs to a sheet, cells STRIDE_X apart (#529).

    citro2d places glyph k of a sheet at x = (k % nRows) * (cellWidth + 1) + 1,
    y = (k // nRows) * (cellHeight + 1) + 1 and draws it cellHeight tall, so
    the cell height (and the line feed and text scale) stays 44 and only the
    width and the line count change. A 64-row sheet leaves 19 blank rows under
    each glyph and the edge clamp over it; the stride leaves 17 px across.
    Mip level 3 (8 px texels, bilinear) reaches 12 px past a quad's edge."""
    import struct
    b = bytearray(open(path, "rb").read())
    finf = struct.unpack_from("<H", b, 6)[0]
    # FINF's default width (left, glyph width, char width), which this
    # mkbcfnt leaves uninitialised; the font shipped through #528 had zeros.
    b[finf + 0xC:finf + 0xF] = bytes(3)
    tglp, cwdh, cmap = struct.unpack_from("<III", b, finf + 0x10)
    cw, ch = b[tglp], b[tglp + 1]
    size, = struct.unpack_from("<I", b, tglp + 4)
    n, fmt, cols, rows, sw, sh = struct.unpack_from("<HHHHHH", b, tglp + 8)
    data, = struct.unpack_from("<I", b, tglp + 0x14)
    assert fmt == 11 and size == sw * sh // 2, "an A4 sheet expected (fmt %d)" % fmt
    assert STRIDE_X >= cw + 1 + 16 and SHEET_H >= ch + 2 + 16
    end = data + n * size
    assert cwdh > end and cmap > end, "CWDH/CMAP expected after the sheets"

    def texel(sheet, x, y):
        t = ((y // 8) * (sw // 8) + x // 8) * 64 + _morton(x & 7, y & 7)
        v = b[data + sheet * size + t // 2]
        return v >> 4 if t & 1 else v & 15

    # Every glyph cell, by index: the sheets hold cols * rows apiece.
    count = n * cols * rows
    per = sw // STRIDE_X
    nsh = -(-count // per)
    nsize = sw * SHEET_H // 2
    out = bytearray(nsh * nsize)
    for g in range(count):
        s, k = divmod(g, cols * rows)
        ox, oy = (k % cols) * (cw + 1) + 1, (k // cols) * (ch + 1) + 1
        ns, nk = divmod(g, per)
        nx, ny = nk * STRIDE_X + 1, 1
        for y in range(ch):
            for x in range(cw):
                v = texel(s, ox + x, oy + y)
                if v:
                    t = (((ny + y) // 8) * (sw // 8) + (nx + x) // 8) * 64 + _morton((nx + x) & 7, (ny + y) & 7)
                    out[ns * nsize + t // 2] |= v << 4 if t & 1 else v
    # Trim the trailing sheets no glyph uses (the last CWDH index decides).
    last = 0
    p = cwdh
    while p:
        lo, hi, nxt = struct.unpack_from("<HHI", b, p)
        last = max(last, hi)
        p = nxt
    nsh = last // per + 1
    out = out[:nsh * nsize]
    delta = len(out) - (end - data)

    def fix(off):
        v, = struct.unpack_from("<I", b, off)
        if v:
            struct.pack_into("<I", b, off, v + delta)

    p = cwdh
    while p:
        q = p
        p, = struct.unpack_from("<I", b, q + 4)
        fix(q + 4)
    p = cmap
    while p:
        q = p
        p, = struct.unpack_from("<I", b, q + 8)
        fix(q + 8)
    fix(finf + 0x14)
    fix(finf + 0x18)
    b[tglp] = STRIDE_X - 1
    struct.pack_into("<I", b, tglp + 4, nsize)
    struct.pack_into("<HHHHHH", b, tglp + 8, nsh, fmt, per, 1, sw, SHEET_H)
    b = b[:data] + out + b[end:]
    struct.pack_into("<I", b, 0x0C, len(b))
    open(path, "wb").write(b)
    print("relaid %s: %d sheet(s) of %dx%d, %d cells a sheet, %d bytes of sheets (was %d)"
          % (os.path.basename(path), nsh, sw, SHEET_H, per, len(out), end - data))


def cap_ink(path, code=ord("H")):
    """Ink rows of one glyph in a .bcfnt's A4 sheet (8x8 tiles, Morton order)."""
    import struct
    b = open(path, "rb").read()
    finf = struct.unpack_from("<H", b, 6)[0]
    tglp, _w, cmap = struct.unpack_from("<III", b, finf + 0x10)
    cw, ch = b[tglp], b[tglp + 1]
    sheet_size, = struct.unpack_from("<I", b, tglp + 4)
    _n, _fmt, cols, rows, sw, _sh = struct.unpack_from("<HHHHHH", b, tglp + 8)
    data, = struct.unpack_from("<I", b, tglp + 0x14)
    idx, p = None, cmap
    while p and idx is None:
        lo, hi, meth, _r, nxt = struct.unpack_from("<HHHHI", b, p)
        if lo <= code <= hi:
            if meth == 0:
                idx = struct.unpack_from("<H", b, p + 12)[0] + code - lo
            elif meth == 1:
                idx = struct.unpack_from("<H", b, p + 12 + 2 * (code - lo))[0]
            else:
                n, = struct.unpack_from("<H", b, p + 12)
                for k in range(n):
                    c, i = struct.unpack_from("<HH", b, p + 14 + 4 * k)
                    if c == code:
                        idx = i
        p = nxt
    sheet, k = divmod(idx, cols * rows)
    x0, y0 = (k % cols) * (cw + 1), (k // cols) * (ch + 1)

    def texel(x, y):
        m = 0
        for i in range(3):
            m |= ((x >> i) & 1) << (2 * i) | ((y >> i) & 1) << (2 * i + 1)
        t = ((y // 8) * (sw // 8) + x // 8) * 64 + m
        v = b[data + sheet * sheet_size + t // 2]
        return v >> 4 if t & 1 else v & 15

    ys = [y for y in range(ch + 1)
          if any(texel(x0 + x, y0 + y) > 7 for x in range(cw + 1))]
    return ys[-1] - ys[0] + 1


if __name__ == "__main__":
    sys.exit(main())
