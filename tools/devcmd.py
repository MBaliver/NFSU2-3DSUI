#!/usr/bin/env python3
"""Drive the game through its dev command file (3DS Files/source/devcmd.h).

Needs DevCommands=1 in the SD card's nfsu2_config.txt. Writes the commands to
<sd>/nfsu2_cmd.txt, waits for the game's <sd>/nfsu2_cmd_done.txt, then turns
every `shot NAME` the script took into NAME.png (top screen over the bottom
one) in --out.

    tools/devcmd.py "press A" "wait 500" "shot menu"
    tools/devcmd.py -f script.txt
    echo "tap 160 120" | tools/devcmd.py -

The SD card defaults to Azahar's (Flatpak) virtual card.
"""
import argparse
import os
import struct
import sys
import time
import zlib

AZAHAR_SD = os.path.expanduser("~/.var/app/org.azahar_emu.Azahar/data/azahar-emu/sdmc")


def read_bmp(path):
    with open(path, "rb") as f:
        d = f.read()
    off, = struct.unpack_from("<I", d, 10)
    w, h = struct.unpack_from("<ii", d, 18)
    bpp, = struct.unpack_from("<H", d, 28)
    assert bpp == 24, path
    row = (w * 3 + 3) & ~3
    rows = []
    for r in range(abs(h)):
        src = r if h < 0 else abs(h) - 1 - r
        b = d[off + src * row: off + src * row + w * 3]
        rgb = bytearray(len(b))
        rgb[0::3], rgb[1::3], rgb[2::3] = b[2::3], b[1::3], b[0::3]
        rows.append(bytes(rgb))
    return w, abs(h), rows


def write_png(path, w, h, rows):
    def chunk(t, data):
        c = struct.pack(">I", len(data)) + t + data
        return c + struct.pack(">I", zlib.crc32(t + data) & 0xFFFFFFFF)
    raw = b"".join(b"\0" + r for r in rows)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def stack(top, bottom, out):
    tw, th, trows = read_bmp(top)
    bw, bh, brows = read_bmp(bottom)
    w = max(tw, bw)
    pad = lambda r, rw: b"\0" * (((w - rw) // 2) * 3) + r + b"\0" * ((w - rw - (w - rw) // 2) * 3)
    rows = [pad(r, tw) for r in trows] + [pad(r, bw) for r in brows]
    write_png(out, w, th + bh, rows)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("commands", nargs="*", help="command lines; '-' reads stdin")
    ap.add_argument("-f", "--file", help="a script file")
    ap.add_argument("--sd", default=AZAHAR_SD, help="the SD card's root (default: Azahar's)")
    ap.add_argument("--out", default=".", help="where the PNGs go")
    ap.add_argument("--timeout", type=float, default=120.0)
    a = ap.parse_args()

    lines = []
    if a.file:
        with open(a.file) as f:
            lines += f.read().splitlines()
    for c in a.commands:
        lines += sys.stdin.read().splitlines() if c == "-" else [c]
    if not lines:
        ap.error("no commands")
    shots = [l.split(None, 1)[1].strip() for l in lines
             if l.strip().lower().startswith("shot ") and len(l.split(None, 1)) > 1]

    cmd = os.path.join(a.sd, "nfsu2_cmd.txt")
    done = os.path.join(a.sd, "nfsu2_cmd_done.txt")
    if os.path.exists(done):
        os.remove(done)
    tmp = cmd + ".tmp"
    with open(tmp, "w") as f:
        f.write("\n".join(lines) + "\n")
    os.replace(tmp, cmd)   # whole, so the game never reads half a script

    t0 = time.time()
    while not os.path.exists(done):
        if time.time() - t0 > a.timeout:
            sys.exit("timed out: is the game running with DevCommands=1?")
        time.sleep(0.2)
    time.sleep(0.1)
    with open(done) as f:
        print(f.read().strip())

    os.makedirs(a.out, exist_ok=True)
    for s in shots:
        s = "".join("_" if c in "/\\ :" else c for c in s)
        top = os.path.join(a.sd, "nfsu2_shots", s + "_top.bmp")
        bot = os.path.join(a.sd, "nfsu2_shots", s + "_bottom.bmp")
        if os.path.exists(top) and os.path.exists(bot):
            out = os.path.join(a.out, s + ".png")
            stack(top, bot, out)
            print(out)
        else:
            print(f"shot {s}: no BMPs", file=sys.stderr)


if __name__ == "__main__":
    main()
