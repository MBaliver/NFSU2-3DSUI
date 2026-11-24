// PORT: the development command file -- see devcmd.h.
#include "devcmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#define DEVCMD_FILE     "sdmc:/nfsu2_cmd.txt"
#define DEVCMD_DONE     "sdmc:/nfsu2_cmd_done.txt"
#define DEVCMD_SHOT_DIR "sdmc:/nfsu2_shots"
#define DEVCMD_POLL_MS  250
#define DEVCMD_MAX_OPS  256

typedef enum {
	OP_KEYS,    // keys held for `frames`
	OP_TOUCH,   // touch from (x1,y1) to (x2,y2) over `frames`
	OP_STICK,   // Circle Pad at (x1,y1) for `frames`
	OP_CSTICK,  // C-Stick at (x1,y1) for `frames`
	OP_WAIT,    // `ms` with nothing pressed
	OP_SHOT,
	OP_LOG,
} OpKind;

typedef struct {
	OpKind kind;
	u32 keys;
	int x1, y1, x2, y2;
	int frames;
	u32 ms;
	char text[64];
} Op;

static bool s_on;
static void (*s_log)(const char*);
static Op s_ops[DEVCMD_MAX_OPS];
static int s_opCount, s_opIdx;
static int s_opFrame;         // frames into the current op
static u64 s_opStartMs;       // OP_WAIT's start
static int s_gapFrames;       // released frames between two inputs
static u64 s_nextPollMs, s_scriptStartMs;
static char s_errors[512];

// This scan's injection.
static bool s_inj;
static u32 s_held, s_heldWas, s_down, s_up;
static bool s_touch;
static touchPosition s_touchPos;
static bool s_stick, s_cstick;
static circlePosition s_stickPos, s_cstickPos;

static void logf_(const char* fmt, const char* a) {
	if (!s_log)
		return;
	char b[160];
	snprintf(b, sizeof(b), fmt, a);
	s_log(b);
}

void devcmd_init(bool enabled, void (*log)(const char*)) {
	s_on = enabled;
	s_log = log;
	if (s_on) {
		remove(DEVCMD_FILE);   // a stale script from a past run
		mkdir(DEVCMD_SHOT_DIR, 0777);
		logf_("devcmd: on, polling %s", DEVCMD_FILE);
	}
}

bool devcmd_enabled(void) { return s_on; }
bool devcmd_active(void) { return s_inj; }

static u32 keyByName(const char* n) {
	static const struct { const char* n; u32 k; } kKeys[] = {
		{ "A", KEY_A }, { "B", KEY_B }, { "X", KEY_X }, { "Y", KEY_Y },
		{ "L", KEY_L }, { "R", KEY_R }, { "ZL", KEY_ZL }, { "ZR", KEY_ZR },
		{ "START", KEY_START }, { "SELECT", KEY_SELECT },
		{ "UP", KEY_DUP }, { "DOWN", KEY_DDOWN }, { "LEFT", KEY_DLEFT }, { "RIGHT", KEY_DRIGHT },
	};
	for (size_t i = 0; i < sizeof(kKeys) / sizeof(kKeys[0]); i++)
		if (strcasecmp(n, kKeys[i].n) == 0)
			return kKeys[i].k;
	return 0;
}

static u32 parseKeys(char* s) {
	u32 k = 0;
	for (char* t = strtok(s, "+"); t; t = strtok(NULL, "+")) {
		const u32 one = keyByName(t);
		if (!one)
			return 0;
		k |= one;
	}
	return k;
}

static void addError(int line, const char* what) {
	const size_t n = strlen(s_errors);
	snprintf(s_errors + n, sizeof(s_errors) - n, "line %d: %s\n", line, what);
}

static bool parseLine(char* line, int lineNo, Op* op) {
	char* hash = strchr(line, '#');
	if (hash)
		*hash = '\0';
	char cmd[16] = "", a[64] = "";
	int n[5] = { 0 };
	if (sscanf(line, "%15s", cmd) != 1)
		return false;   // blank
	memset(op, 0, sizeof(*op));
	const char* rest = line + strspn(line, " \t");
	rest += strlen(cmd);
	rest += strspn(rest, " \t");
	if (strcasecmp(cmd, "press") == 0 || strcasecmp(cmd, "hold") == 0) {
		const int got = sscanf(rest, "%63s %d", a, &n[0]);
		op->kind = OP_KEYS;
		op->keys = got >= 1 ? parseKeys(a) : 0;
		op->frames = got >= 2 ? n[0] : 6;
		if (!op->keys || op->frames <= 0) {
			addError(lineNo, "bad keys or frames");
			return false;
		}
	} else if (strcasecmp(cmd, "tap") == 0) {
		const int got = sscanf(rest, "%d %d %d", &n[0], &n[1], &n[2]);
		if (got < 2) {
			addError(lineNo, "tap x y [frames]");
			return false;
		}
		op->kind = OP_TOUCH;
		op->x1 = op->x2 = n[0];
		op->y1 = op->y2 = n[1];
		op->frames = got >= 3 ? n[2] : 6;
	} else if (strcasecmp(cmd, "drag") == 0) {
		const int got = sscanf(rest, "%d %d %d %d %d", &n[0], &n[1], &n[2], &n[3], &n[4]);
		if (got < 4) {
			addError(lineNo, "drag x1 y1 x2 y2 [frames]");
			return false;
		}
		op->kind = OP_TOUCH;
		op->x1 = n[0]; op->y1 = n[1]; op->x2 = n[2]; op->y2 = n[3];
		op->frames = got >= 5 ? n[4] : 20;
	} else if (strcasecmp(cmd, "stick") == 0 || strcasecmp(cmd, "cstick") == 0) {
		if (sscanf(rest, "%d %d %d", &n[0], &n[1], &n[2]) < 3) {
			addError(lineNo, "stick dx dy frames");
			return false;
		}
		op->kind = cmd[0] == 'c' || cmd[0] == 'C' ? OP_CSTICK : OP_STICK;
		op->x1 = n[0]; op->y1 = n[1]; op->frames = n[2];
	} else if (strcasecmp(cmd, "wait") == 0) {
		if (sscanf(rest, "%d", &n[0]) < 1 || n[0] < 0) {
			addError(lineNo, "wait ms");
			return false;
		}
		op->kind = OP_WAIT;
		op->ms = (u32)n[0];
	} else if (strcasecmp(cmd, "shot") == 0 || strcasecmp(cmd, "log") == 0) {
		op->kind = cmd[0] == 's' || cmd[0] == 'S' ? OP_SHOT : OP_LOG;
		snprintf(op->text, sizeof(op->text), "%s", rest);
		// trailing whitespace / CR
		for (int i = (int)strlen(op->text) - 1; i >= 0 && (u8)op->text[i] <= ' '; i--)
			op->text[i] = '\0';
		if (op->kind == OP_SHOT) {
			if (!op->text[0])
				snprintf(op->text, sizeof(op->text), "shot");
			for (char* c = op->text; *c; c++)
				if (*c == '/' || *c == '\\' || *c == ' ' || *c == ':')
					*c = '_';
		}
	} else {
		addError(lineNo, "unknown command");
		return false;
	}
	return true;
}

static void loadScript(void) {
	FILE* f = fopen(DEVCMD_FILE, "r");
	if (!f)
		return;
	s_opCount = s_opIdx = 0;
	s_opFrame = 0;
	s_gapFrames = 0;
	s_errors[0] = '\0';
	char line[160];
	int lineNo = 0;
	while (fgets(line, sizeof(line), f) && s_opCount < DEVCMD_MAX_OPS) {
		lineNo++;
		if (parseLine(line, lineNo, &s_ops[s_opCount]))
			s_opCount++;
	}
	fclose(f);
	remove(DEVCMD_FILE);
	remove(DEVCMD_DONE);
	s_scriptStartMs = osGetTime();
	char b[64];
	snprintf(b, sizeof(b), "%d", s_opCount);
	logf_("devcmd: script with %s commands", b);
}

static void writeDone(void) {
	FILE* f = fopen(DEVCMD_DONE, "w");
	if (!f)
		return;
	fprintf(f, "done %d %llu\n%s", s_opCount,
		(unsigned long long)(osGetTime() - s_scriptStartMs), s_errors);
	fclose(f);
	s_opCount = s_opIdx = 0;
}

// The screen as its framebuffer holds it, written as a 24-bit BMP. The
// framebuffer is the panel's portrait layout: `fw` (240) across, `fh` (400 or
// 320) down, column 0 the screen's bottom row.
static bool shotScreen(gfxScreen_t screen, const char* path) {
	u16 fw = 0, fh = 0;
	const u8* fb = gfxGetFramebuffer(screen, GFX_LEFT, &fw, &fh);
	if (!fb || !fw || !fh)
		return false;
	const GSPGPU_FramebufferFormat fmt = gfxGetScreenFormat(screen);
	const int bpp = fmt == GSP_RGBA8_OES ? 4 : fmt == GSP_BGR8_OES ? 3 : 2;
	const int W = fh, H = fw;   // landscape
	const int rowBytes = (W * 3 + 3) & ~3;
	u8* row = (u8*)calloc(1, rowBytes);
	FILE* f = fopen(path, "wb");
	if (!row || !f) {
		free(row);
		if (f)
			fclose(f);
		return false;
	}
	const u32 dataSize = (u32)rowBytes * (u32)H;
	u8 hdr[54] = { 'B', 'M' };
	#define PUT32(o, v) do { hdr[o] = (u8)(v); hdr[o + 1] = (u8)((v) >> 8); \
		hdr[o + 2] = (u8)((v) >> 16); hdr[o + 3] = (u8)((v) >> 24); } while (0)
	PUT32(2, 54 + dataSize);
	PUT32(10, 54);
	PUT32(14, 40);
	PUT32(18, (u32)W);
	PUT32(22, (u32)H);   // positive: bottom-up rows
	hdr[26] = 1;
	hdr[28] = 24;
	PUT32(34, dataSize);
	#undef PUT32
	fwrite(hdr, 1, sizeof(hdr), f);
	// BMP row r (bottom-up) is screen row H-1-r, which is framebuffer column r.
	for (int r = 0; r < H; r++) {
		for (int x = 0; x < W; x++) {
			const u8* p = fb + ((size_t)x * fw + (size_t)r) * bpp;
			u8 R, G, B;
			if (fmt == GSP_RGBA8_OES) {
				B = p[1]; G = p[2]; R = p[3];
			} else if (fmt == GSP_BGR8_OES) {
				B = p[0]; G = p[1]; R = p[2];
			} else {
				const u16 v = (u16)(p[0] | (p[1] << 8));
				if (fmt == GSP_RGB565_OES) {
					R = (u8)((v >> 11) << 3); G = (u8)(((v >> 5) & 63) << 2); B = (u8)((v & 31) << 3);
				} else if (fmt == GSP_RGB5_A1_OES) {
					R = (u8)((v >> 11) << 3); G = (u8)(((v >> 6) & 31) << 3); B = (u8)(((v >> 1) & 31) << 3);
				} else {   // RGBA4
					R = (u8)((v >> 12) << 4); G = (u8)(((v >> 8) & 15) << 4); B = (u8)(((v >> 4) & 15) << 4);
				}
			}
			row[x * 3] = B;
			row[x * 3 + 1] = G;
			row[x * 3 + 2] = R;
		}
		fwrite(row, 1, rowBytes, f);
	}
	fclose(f);
	free(row);
	return true;
}

static void takeShot(const char* name) {
	char p[160];
	snprintf(p, sizeof(p), DEVCMD_SHOT_DIR "/%s_top.bmp", name);
	const bool a = shotScreen(GFX_TOP, p);
	snprintf(p, sizeof(p), DEVCMD_SHOT_DIR "/%s_bottom.bmp", name);
	const bool b = shotScreen(GFX_BOTTOM, p);
	logf_(a && b ? "devcmd: shot %s" : "devcmd: shot %s FAILED", name);
}

// One frame of the script: runs instant ops, sets this frame's injection.
static void step(void) {
	s_held = 0;
	s_touch = s_stick = s_cstick = false;
	if (s_gapFrames > 0) {
		s_gapFrames--;
		return;
	}
	while (s_opIdx < s_opCount) {
		Op* op = &s_ops[s_opIdx];
		if (op->kind == OP_SHOT) {
			takeShot(op->text);
			s_opIdx++;
			continue;
		}
		if (op->kind == OP_LOG) {
			logf_("devcmd: %s", op->text);
			s_opIdx++;
			continue;
		}
		if (op->kind == OP_WAIT) {
			if (s_opFrame == 0)
				s_opStartMs = osGetTime();
			s_opFrame++;
			if (osGetTime() - s_opStartMs >= op->ms) {
				s_opIdx++;
				s_opFrame = 0;
				continue;
			}
			return;
		}
		// A held input: this frame's part of it.
		const float t = op->frames > 1 ? (float)s_opFrame / (float)(op->frames - 1) : 1.0f;
		switch (op->kind) {
		case OP_KEYS:
			s_held = op->keys;
			break;
		case OP_TOUCH:
			s_touch = true;
			s_held = KEY_TOUCH;
			s_touchPos.px = (u16)(op->x1 + (op->x2 - op->x1) * t + 0.5f);
			s_touchPos.py = (u16)(op->y1 + (op->y2 - op->y1) * t + 0.5f);
			break;
		case OP_STICK:
			s_stick = true;
			s_stickPos.dx = (s16)op->x1;
			s_stickPos.dy = (s16)op->y1;
			break;
		case OP_CSTICK:
			s_cstick = true;
			s_cstickPos.dx = (s16)op->x1;
			s_cstickPos.dy = (s16)op->y1;
			break;
		default:
			break;
		}
		if (++s_opFrame >= op->frames) {
			s_opIdx++;
			s_opFrame = 0;
			s_gapFrames = 2;   // let go before the next input
		}
		return;
	}
}

void devcmd_hidScanInput(void) {
	hidScanInput();
	if (!s_on)
		return;
	const u64 now = osGetTime();
	if (s_opCount == 0 && now >= s_nextPollMs) {
		s_nextPollMs = now + DEVCMD_POLL_MS;
		struct stat st;
		if (stat(DEVCMD_FILE, &st) == 0)
			loadScript();
	}
	if (s_opCount > 0) {
		step();
		if (s_opIdx >= s_opCount && s_gapFrames == 0)
			writeDone();
	} else {
		s_held = 0;
		s_touch = s_stick = s_cstick = false;
	}
	s_down = s_held & ~s_heldWas;
	s_up = s_heldWas & ~s_held;
	s_heldWas = s_held;
	s_inj = s_held || s_touch || s_stick || s_cstick || s_up;
}

u32 devcmd_hidKeysDown(void) { return hidKeysDown() | s_down; }
u32 devcmd_hidKeysHeld(void) { return hidKeysHeld() | s_held; }
u32 devcmd_hidKeysUp(void) { return hidKeysUp() | s_up; }

void devcmd_hidTouchRead(touchPosition* pos) {
	if (s_touch)
		*pos = s_touchPos;
	else
		hidTouchRead(pos);
}

void devcmd_hidCircleRead(circlePosition* pos) {
	if (s_stick)
		*pos = s_stickPos;
	else
		hidCircleRead(pos);
}

void devcmd_hidCstickRead(circlePosition* pos) {
	if (s_cstick)
		*pos = s_cstickPos;
	else
		hidCstickRead(pos);
}
