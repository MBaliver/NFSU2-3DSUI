"""Where the packers' outside programs are (dspenc, vgmstream-cli, ffmpeg,
ffprobe, tex3ds), on any system.

Looked for, in order:
  - a `bin` folder beside the NFSU2-3DS app's exe (tools/build_apps.py puts
    them there, so a Windows PC needs nothing installed);
  - tools/bin and tools/bin/vgmstream in this folder (the repo's own copies);
  - the PATH (Linux and macOS: the system's ffmpeg and vgmstream-cli, and
    dspenc built with `cc -O2 -o tools/bin/dspenc tools/src/dspenc.c -lm`).

stage.py also puts the bin folders on the PATH it hands each packer, so a
packer that runs plain "ffmpeg" finds the bundled one.
"""
import os
import shutil
import sys

TOOLS = os.path.dirname(os.path.abspath(__file__))


def exe_name(name):
    return name + ".exe" if os.name == "nt" else name


def is_lfs_pointer(path):
    """True for a Git LFS pointer stub (a ~130-byte text file) standing in for
    a program. GitHub's "Download ZIP" ships the repo's bin/ffmpeg/ffmpeg.exe
    that way, and running it fails with WinError 216."""
    try:
        if os.path.getsize(path) > 1024:
            return False
        with open(path, "rb") as f:
            return f.read(40).startswith(b"version https://git-lfs")
    except OSError:
        return False


def _usable(d):
    """A bin folder whose only program is an LFS pointer is left out, so the
    PATH's own ffmpeg is the one a packer runs."""
    progs = [n for n in os.listdir(d) if n.lower().endswith(".exe") or
             (os.name != "nt" and os.access(os.path.join(d, n), os.X_OK)
              and os.path.isfile(os.path.join(d, n)))]
    return not progs or not all(is_lfs_pointer(os.path.join(d, n)) for n in progs)


def bin_dirs():
    dirs = []
    if getattr(sys, "frozen", False):
        app = os.path.dirname(os.path.abspath(sys.executable))
        dirs += [os.path.join(app, "bin"), os.path.join(app, "bin", "vgmstream"),
                 os.path.join(app, "bin", "ffmpeg"), os.path.join(app, "bin", "tex3ds")]
    dirs += [os.path.join(TOOLS, "bin"), os.path.join(TOOLS, "bin", "vgmstream")]
    # Run from a built app's source/ folder: the app's own bin/ above it.
    app = os.path.dirname(TOOLS)
    if os.path.isdir(os.path.join(app, "_internal")):
        dirs += [os.path.join(app, "bin"), os.path.join(app, "bin", "vgmstream"),
                 os.path.join(app, "bin", "ffmpeg"), os.path.join(app, "bin", "tex3ds")]
    return [d for d in dirs if os.path.isdir(d) and _usable(d)]


def find(name):
    """The program's path, or its bare name (so the error names it) when it
    is nowhere."""
    for d in bin_dirs():
        p = os.path.join(d, exe_name(name))
        if os.path.isfile(p) and not is_lfs_pointer(p):
            return p
    w = shutil.which(name)
    return w if w and not is_lfs_pointer(w) else name


def ffmpeg_problem():
    """None when a real ffmpeg is reachable, else what to tell the user."""
    if find("ffmpeg") != "ffmpeg":
        return None
    return ("ffmpeg was not found. The bin/ffmpeg/ffmpeg.exe in a GitHub "
            "\"Download ZIP\" is only a Git LFS placeholder; download the "
            "release zip, clone with Git LFS installed, or install ffmpeg "
            "(with libx264) and put it on PATH.")


def have(name):
    return find(name) != name or shutil.which(name) is not None


def path_with_bins(path=None):
    """PATH with the bin folders in front, for a child process."""
    path = os.environ.get("PATH", "") if path is None else path
    return os.pathsep.join(bin_dirs() + ([path] if path else []))


# ------------------------------------------------------------------ tex3ds
# devkitPro's tex3ds encodes the tracks' ETC1 textures (etc1_tex.py). The
# Windows apps carry it in bin/tex3ds; elsewhere it comes with devkitPro's
# 3dstools package.
DEVKITPRO_INSTALLER_API = "https://api.github.com/repos/devkitPro/installer/releases/latest"
DEVKITPRO_GUIDE = "https://devkitpro.org/wiki/Getting_Started"


class MissingTex3ds(SystemExit):
    """tex3ds is nowhere: the apps catch this one and offer to install it."""


def find_tex3ds():
    """tex3ds's path, or None. Looked for: $TEX3DS, the bin folders (the
    apps' bin/tex3ds), devkitPro's tools/bin ($DEVKITPRO, C:/devkitPro,
    /opt/devkitpro), then the PATH."""
    cands = [os.environ.get("TEX3DS", "")]
    cands += [os.path.join(d, exe_name("tex3ds")) for d in bin_dirs()]
    # DEVKITPRO is often an MSYS path (/opt/devkitpro) that Windows cannot
    # open; the usual install folders follow it.
    for root in (os.environ.get("DEVKITPRO"), r"C:\devkitPro", "/opt/devkitpro"):
        if root:
            cands.append(os.path.join(root, "tools", "bin", exe_name("tex3ds")))
    cands.append(shutil.which("tex3ds") or "")
    for p in cands:
        if p and os.path.isfile(p) and not is_lfs_pointer(p):
            return p
    return None


def tex3ds_problem():
    """None when tex3ds is reachable, else what to tell the user."""
    if find_tex3ds():
        return None
    if os.name == "nt":
        how = ("The NFSU2-3DS Tools release zip has it in bin/tex3ds; otherwise install "
               "devkitPro (%s) with \"3DS Development\" ticked, into C:\\devkitPro." % DEVKITPRO_GUIDE)
    else:
        how = ("Install devkitPro's pacman (%s), then: sudo dkp-pacman -S 3dstools "
               "(or set TEX3DS to a tex3ds you built)." % DEVKITPRO_GUIDE)
    return "tex3ds (devkitPro's 3DS texture tool) was not found; the tracks' textures need it. " + how


def _pacman():
    for p in (shutil.which("dkp-pacman"), "/opt/devkitpro/pacman/bin/pacman"):
        if p and os.path.isfile(p):
            return p
    return None


def can_install_tex3ds():
    return os.name == "nt" or _pacman() is not None


def _run_elevated_and_wait(exe):
    """Starts exe through the shell (so its UAC prompt shows) and waits."""
    import ctypes
    from ctypes import wintypes

    class SEI(ctypes.Structure):
        _fields_ = [("cbSize", wintypes.DWORD), ("fMask", ctypes.c_ulong), ("hwnd", wintypes.HWND),
                    ("lpVerb", wintypes.LPCWSTR), ("lpFile", wintypes.LPCWSTR),
                    ("lpParameters", wintypes.LPCWSTR), ("lpDirectory", wintypes.LPCWSTR),
                    ("nShow", ctypes.c_int), ("hInstApp", wintypes.HINSTANCE),
                    ("lpIDList", ctypes.c_void_p), ("lpClass", wintypes.LPCWSTR),
                    ("hkeyClass", wintypes.HKEY), ("dwHotKey", wintypes.DWORD),
                    ("hIcon", wintypes.HANDLE), ("hProcess", wintypes.HANDLE)]
    sei = SEI()
    sei.cbSize = ctypes.sizeof(SEI)
    sei.fMask = 0x00000040          # SEE_MASK_NOCLOSEPROCESS
    sei.lpVerb = "open"
    sei.lpFile = exe
    sei.lpDirectory = os.path.dirname(exe)
    sei.nShow = 1                   # SW_SHOWNORMAL
    if not ctypes.windll.shell32.ShellExecuteExW(ctypes.byref(sei)):
        raise OSError("could not start %s (error %d)" % (exe, ctypes.GetLastError()))
    if sei.hProcess:
        ctypes.windll.kernel32.WaitForSingleObject(sei.hProcess, 0xFFFFFFFF)
        ctypes.windll.kernel32.CloseHandle(sei.hProcess)


def install_tex3ds(log=print):
    """Installs tex3ds where this system allows it. Windows: downloads the
    official devkitPro installer from its GitHub releases and runs it (the
    user ticks "3DS Development"). Linux / macOS with devkitPro's pacman:
    sudo dkp-pacman -S 3dstools. Returns tex3ds's path, or None."""
    if os.name == "nt":
        import json
        import tempfile
        import urllib.request
        hdr = {"User-Agent": "NFSU2-3DS-Tools", "Accept": "application/vnd.github+json"}
        log("tex3ds: looking up the latest devkitPro installer (%s)" % DEVKITPRO_INSTALLER_API)
        with urllib.request.urlopen(urllib.request.Request(DEVKITPRO_INSTALLER_API, headers=hdr),
                                    timeout=30) as r:
            rel = json.load(r)
        asset = next((a for a in rel.get("assets", []) if a.get("name", "").lower().endswith(".exe")
                      and "devkitpro" in a.get("name", "").lower()), None)
        if not asset:
            raise RuntimeError("no installer in devkitPro's latest release; get it from " + DEVKITPRO_GUIDE)
        exe = os.path.join(tempfile.mkdtemp(prefix="nfsu2_dkp_"), asset["name"])
        log("tex3ds: downloading %s (%.1f MB)" % (asset["browser_download_url"], asset.get("size", 0) / 1e6))
        with urllib.request.urlopen(urllib.request.Request(asset["browser_download_url"],
                                                           headers={"User-Agent": hdr["User-Agent"]}),
                                    timeout=120) as r, open(exe, "wb") as fh:
            shutil.copyfileobj(r, fh)
        log("tex3ds: running the devkitPro installer -- tick \"3DS Development\" and keep the "
            "folder C:\\devkitPro; this waits until it closes")
        _run_elevated_and_wait(exe)
    else:
        pm = _pacman()
        if not pm:
            log("tex3ds: " + tex3ds_problem())
            return None
        cmd = ["sudo", pm, "-S", "--needed", "3dstools"]
        log("tex3ds: running " + " ".join(cmd))
        import subprocess
        subprocess.call(cmd)
    p = find_tex3ds()
    log("tex3ds: %s" % (p or "still not found -- " + tex3ds_problem()))
    return p


def prompt_install_tex3ds(err, ask=input, log=print):
    """The command line's question after MissingTex3ds: True when tex3ds is
    there afterwards (the caller then runs the job again)."""
    log("")
    log(str(err))
    if not can_install_tex3ds():
        return False
    try:
        a = ask("Install it now%s? [y/N] " % (" (downloads and runs devkitPro's installer)"
                                               if os.name == "nt" else " (sudo dkp-pacman -S 3dstools)"))
    except EOFError:
        return False
    if a.strip().lower() not in ("y", "yes"):
        return False
    try:
        return install_tex3ds(log) is not None
    except Exception as e:
        log("tex3ds: install failed: %s" % e)
        return False
