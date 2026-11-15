#!/usr/bin/env python3
"""Unpack an EA BIG archive (BIGF / BIG4 / BIGH), e.g. NFSU2's SDATA\\sdat.viv.

Some PC installs (the retail DVD's, some digital copies) keep the music and
speech in one archive, SDATA\\sdat.viv, instead of the loose pfdata\\ and
speech\\ folders. Unpacked into the game folder it gives exactly those:

    pfdata\\MusicSFx.mus, pfdata\\MusicSFx.mpf
    speech\\UG2_Speech.big, speech\\UG2_Speech.idx, speech\\nisaudio.big, ...

tools/stage.py runs this by itself when pfdata\\MusicSFx.mus is missing and
SDATA\\sdat.viv is there.

Layout (all big-endian): magic[4], u32 archive size, u32 file count,
u32 header size; then per file u32 offset, u32 size, NUL-terminated name
(backslash-separated).

    python viv_unpack.py <archive.viv> <out dir> [--list]
"""
import argparse
import os
import struct
import sys

MAGICS = (b"BIGF", b"BIG4", b"BIGH")


def entries(path):
    with open(path, "rb") as f:
        head = f.read(16)
        if len(head) < 16 or head[:4] not in MAGICS:
            raise SystemExit("%s: not an EA BIG archive (magic %r)" % (path, head[:4]))
        _size, count, hsize = struct.unpack(">III", head[4:16])
        table = f.read(hsize - 16)
    out, p = [], 0
    for _ in range(count):
        off, size = struct.unpack_from(">II", table, p)
        end = table.index(b"\0", p + 8)
        name = table[p + 8:end].decode("latin-1").replace("\\", "/")
        p = end + 1
        out.append((name, off, size))
    return out


def safe_join(root, name):
    parts = [x for x in name.split("/") if x not in ("", ".")]
    if any(x == ".." for x in parts) or not parts or ":" in parts[0]:
        raise SystemExit("unsafe name in archive: %r" % name)
    return os.path.join(root, *parts)


def unpack(path, out_dir, log=print):
    ents = entries(path)
    with open(path, "rb") as f:
        for name, off, size in ents:
            dst = safe_join(out_dir, name)
            if os.path.isfile(dst) and os.path.getsize(dst) == size:
                continue
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            f.seek(off)
            left = size
            with open(dst + ".part", "wb") as o:
                while left:
                    chunk = f.read(min(left, 1 << 22))
                    if not chunk:
                        raise SystemExit("%s: truncated at %s" % (path, name))
                    o.write(chunk)
                    left -= len(chunk)
            os.replace(dst + ".part", dst)
            log("  %s (%d bytes)" % (name, size))
    return ents


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("archive")
    ap.add_argument("out", nargs="?", default=".")
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args(argv)
    if a.list:
        for name, off, size in entries(a.archive):
            print("%10d  %10d  %s" % (off, size, name))
        return 0
    unpack(a.archive, a.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
