# Lower-screen UI rework

Port-side work on the touch screen and on text layout, done on top of the
`497a9bf`..`2f57219` tree. None of it is retail behaviour: the PC game has no
second screen, so everything here is a port decision, marked `PORT` in the
source the same way the rest of the port marks its own choices.

The goal was a lower screen that reads like a 3DS game rather than a 640x480
PC layout squeezed into 320x240: readable text, real touch targets, and the
touch screen used for actions, not only for display.

Contents:

1. [Build and deploy on Linux](#1-build-and-deploy-on-linux)
2. [Text measurement fix (citro2d 1.7)](#2-text-measurement-fix-citro2d-17)
3. [Text that ran off the screen](#3-text-that-ran-off-the-screen)
4. [The lower-screen HUD kit](#4-the-lower-screen-hud-kit)
5. [Free roam](#5-free-roam)
6. [Races](#6-races)
7. [Minimap changes](#7-minimap-changes)
8. [GPS route on the map](#8-gps-route-on-the-map)
9. [Reskinned menu screens](#9-reskinned-menu-screens)
10. [Drag Shift: ZL/ZR option](#10-drag-shift-zlzr-option)
11. [Screen inventory and what is left](#11-screen-inventory-and-what-is-left)
12. [Testing status](#12-testing-status)

---

## 1. Build and deploy on Linux

`tools/build_cia.sh` builds the `.3dsx` and `.cia` without the Windows
`build_cia.ps1`. The Makefile cannot run from a path with a space in it
(`3DS Files`), so the script bind-mounts the project onto `build_3ds/` inside
an unprivileged mount namespace (`unshare -rm`) and runs `make cia` there.
It expects devkitPro under `~/.local/share/devkitpro/opt/devkitpro` (override
with `DEVKITPRO` / `DEVKITARM`) and uses `tools/3ds-bin/makerom` and
`tools/3ds-bin/bannertool`.

```
bash tools/build_cia.sh
```

Deploying to a console runs over Wi-Fi with **3dsDL**, an HTTP server app on
the 3DS. The console must allow the PC once: `GET /api/list` raises the prompt
on the 3DS (a large `PUT` does not), and `GET /api/status` reports `access` as
`none`, `pending`, `allowed` or `denied`. `denied` sticks until 3dsDL is
reopened.

```
H=3ds.local
curl -s "http://$H/api/list?path=%2F" -H "X-Requested-With: 3dsDL"      # ask
curl -s "http://$H/api/status" -H "X-Requested-With: 3dsDL"            # wait for "allowed"
curl -f -X PUT "http://$H/api/file?path=%2Fcias%2FNFSU2-3DS.cia&replace=1" \
     -H "X-Requested-With: 3dsDL" --data-binary "@3DS Files/NFSU2-3DS.cia"
curl -s -X POST "http://$H/api/install" -H "X-Requested-With: 3dsDL" \
     --data-urlencode "paths=/cias/NFSU2-3DS.cia"
```

Everything in this document lives in the executable. The game data on the SD
card (`/nfsu2`) is unchanged, so only the `.cia` has to be reinstalled.

## 2. Text measurement fix (citro2d 1.7)

**Symptom:** text all over the menus was misaligned. Wrapping, fitting and
anything right-aligned or centred by hand came out wrong, and the game's own
fonts were sized too large.

**Cause:** `textDims()` (`c2dtext.h`) was written for citro2d 1.6.0, where
`C2D_TextGetDimensions` does not apply a custom font's `textScale` to the
width, so `textDims` applied it itself. citro2d 1.7.0 multiplies the width by
the font's `textScale` at parse time (`C2D_TextFontParseLine`, the multiply at
+0x17C in `libcitro2d.a`'s `text.o`). With the port's correction on top, every
custom-font string measured about 60% of its drawn width. The same measurement
feeds `loadRetailFonts()`'s calibration, so the retail font sizes were off as
well.

**Fix:** `textWidthNeedsFontScale()` finds out at runtime, once per font,
which behaviour the linked library has. It parses `"M"`, compares the reported
width with the glyph's own advance (`C2D_FontGetCharWidthInfo`) and applies
the correction only on 1.6-style libraries. The port now measures correctly
against either version.

## 3. Text that ran off the screen

- **Top-screen HUD** (`hud.c`, `drawTextIn`): strings such as the
  `TRAFFIC HIT` popup next to the speedometer are narrowed to at most 80% of
  their width, then slid back inside the screen. The clip width is 400 on the
  top screen and 320 for the race stats column (`s_hudClipW`).
- **Menus** (`main.c`, `menuTextKeepClear`, called from `drawMenu`): a string
  is first moved off any `$JOY_EVENT$` button badge it overlaps on its own
  line (the "Go to Mailboxes" label sat under its B), then narrowed (down to
  `MENU_TEXT_EDGE_SQUEEZE`, 75%) and slid inside the panel. A string that is
  entirely off the panel is left alone, because that is a layout parking it.
  `menuPadGlyphRect()` repeats `drawMenu`'s badge sums for the overlap test.
- **SMS reader wrap** (`smsReaderLayout`, `smsWrapPx`): `ctsWrapText()`
  measures with the PC font's advances, but the lines are drawn with
  `g_uiFont` at the `MENU_MIN_TEXT_PX` floor, which is wider. The body is now
  rewrapped in drawn pixels against the room the line really has: the
  authored box or the panel, whichever is smaller. The body is
  centre-justified, so the room is twice the nearer side.

The reskinned reader (section 9) replaces this path on the lower screen, but
the fix stays for any other layout that uses it.

## 4. The lower-screen HUD kit (U2-UI)

A small drawing kit in `main.c`, just before `drawWorldBottom`, shared by
every reworked screen. The design was settled on an interactive mockup
(320x240 at 2x) before any of it was implemented.

The first version used invented colours (a bluish grey, a solid green
button, a white focus ring). Revision 2 rebuilt it from the game's own menu
pieces, read out of `GLOBAL/GLOBALB.BUN` (`tools/extract_global_textures.py`)
and the `GenericDialog*.fng` screens (`tools/fng_extract.py`,
`tools/fng_layout.py`). The comment above `UI_PANEL` lists the sources.

| Retail piece | What it is | Kit function |
| --- | --- | --- |
| `U2_Menu_Box` | black at alpha 153, 3 px corners (`U2_Menu_RoundedCorner`) | `uiChip()`, `UI_PANEL` |
| `U2_Menu_Box_White` tinted grey | list rows | `uiRowBar()`, `UI_GREY` |
| `U2_Menu_Text_Corner` | half-disc end of a title bar | `uiTab()` (tabs hung from a screen edge) |
| `U2_Icon_Highlight` | crisp rim, soft inner light, tinted lime | `uiHighlight()` |
| `UI_PC_Generic_Button` | black pill with a light rim | `uiPill()`, `uiButton()` |
| `GenericDialog` ring | (197,218,165) round the panel | `uiDialogRing()`, `UI_FRAME` |

- **Geometry:** `uiRoundRect()` fills a rounded rectangle from strips and
  quarter-disc fans that do not overlap. `uiBand()` draws the band between two
  offsets of a rounded outline with per-edge colours, so one function makes
  rims (`uiRing()`) and soft light that fades to clear. A tab is a pill whose
  other end runs off the screen.
- **Selection:** one indicator everywhere, the highlight: lime rim, a faint
  lit body and light just inside the rim. There is no outer glow: the retail
  texture has none, and on the 320x240 screen one would only blur.
- **Colours:** `UI_ACCENT` lime (202,244,138) for labels, the selection and
  lit button text; white values; `UI_DIM` neutral grey. Nothing is blue-tinted.
- **Text:** `uiText()` centres a string's capitals on a y coordinate, and
  `uiTextBase()` sits it on a baseline. Both work from the font's own cell
  baseline (`tglp->baselinePos` at the drawn scale) and a cap height of 24/34
  of the line, so strings of different sizes line up. `maxW` truncates with
  "..". `uiLabel()` is the 12 px caps label.
- **Other pieces:** `uiArrow()`, `uiWrap()` (word wrap in drawn pixels),
  `uiButton()` (pill with the real 3DS button art from `drawPadGlyph`; the lit
  one wears the highlight), `uiBackChip()` (header band with the back pill),
  `uiActionBar()` (footer band, 30 px pills from the right, the primary lit),
  `uiBand2()` (a black header/footer band) and `uiScreenBackdrop()` (neutral
  black gradient).

Each screen lays out its touch targets while it draws, and the next tap is
tested against them. Free roam uses `g_uiHit`, the World Map `g_wmHit`, the
mailbox `g_smsHit` and dialogs `g_uiDlgHit`.

## 5. Free roam

`drawWorldBottom()` → `drawWorldChips()`. The map is full-bleed (320x240) and
everything else floats over it.

- **Top (status):** the bank chip (`BANK $12,450`; a tap opens Career Status)
  and, when there are unread messages, an SMS chip with an envelope and the
  count (a tap opens the mailbox). A world challenge's clock replaces the bank
  chip while it runs, and keeps retail's `Animate` / `Animate_RED` alpha.
  The taps go through `g_touchOpenUi`, which the world loop answers exactly
  where the pad's press for that screen would be answered.
- **Right edge:** + / − zoom buttons (`minimap_set_view_zoom`, 0.5x to 2.5x),
  and a `RECENTER` chip while the map is panned.
- **Bottom (action):** the destination strip. With the GPS on, it shows the
  target's name, the straight-line distance and a compass arrow pointing the
  way the map shows, with a ✕ to stop the GPS. With the GPS off, it shows the
  nearest race and offers `SET GPS`. Distances follow the profile's units
  (`unitsMetric()`), so switching MPH / KM/H in the options switches them too.
- **Event feed:** up to three short lines above the strip, 4 s each:
  `+$500 Bank Reward` (this replaces retail's `Cash_Animate` popup, which had
  no bank value left to rise into), `New SMS message`, `GPS: Garage` and
  `GPS off`.
- **Marker card:** tapping a marker opens a card with SHOP / EVENT, its name
  and a detail line (the purse and "Sponsor race" for events), plus `SET GPS`.
  It starts the game's own GPS (`gpsZoneTarget` + `gps_start`, the same calls
  as the World Map's Activate GPS). Marker names come from the element type
  (`worldMarkerText`), because the shop records only carry internal names such
  as `UC_BODYSHOP`.
- **GPS colour:** while the GPS tracks a marker, everything that belongs to
  the GPS takes that marker's colour (purple for the Garage, and so on): the
  strip's stripe, label and compass, the feed line, the route and the map's
  GPS rings. Text and the compass use a lightened tint so that dark colours
  stay readable. While tracking, this overrides the profile's GPS Color option
  for the rings.

## 6. Races

`drawRacePanel()`, with the map on the right (`RACE_PANEL_W` = 122 px panel).

- **Position:** the place drawn large, its ordinal suffix hung from the
  numeral's cap line and `/N` on its baseline.
- **Lap and time** side by side, then a lap progress bar (from
  `RacingCar.lapExact`) and the best lap.
- **Standings** come from `hud_race_standings()` (new in `hud.c`), which
  returns rows in race order with name and gap together instead of retail's
  alternating names / gaps. The player's row is highlighted. A car ahead
  shows its gap in red and a car behind in green. A drift shows the board's
  points under `SCORES`.
- **Finish line:** `drawRaceFinish()` draws a chequered flag at
  `Race.finishLine` (a circuit's is its start line) and pins it to the map's
  edge while it is off screen.

## 7. Minimap changes

`minimap.c` / `minimap.h`:

- `minimap_set_square()`: fills the given rect instead of retail's inscribed
  disc. There is no mask and no ring; icons pin to the rect's edges
  (`mmPlaceRect`) and the north indicator rides the edge. The scale stays the
  round panel's (one window across the disc's diameter), so the extra area
  shows more of the city rather than zooming in. The player arrow is drawn
  1.5x larger in this mode.
- **Touch panning:** `minimap_pan_px()` turns a drag in panel pixels into a
  world offset, undoing the map's rotation, and keeps the view on the art.
  `minimap_pan_reset()`, `minimap_pan_ease()` and `minimap_panned()` go with
  it. While panned, the car icon is drawn where it is and pinned to the edge.
  The view eases back after `WBOT_PAN_HOLD_MS` (4 s) without touch.
- `minimap_set_view_zoom()` / `minimap_view_zoom()`: a zoom on top of
  retail's aperture.
- `minimap_world_to_panel()`: where a world point landed in the last draw.
  The HUD uses it for marker taps, the compass and the finish flag.
- `MinimapGps` gained `route`, `routeN` and `routeRgba` (section 8).

## 8. GPS route on the map

Retail's map shows only the GPS rings at the target and guides with the 3D
arrow on the top screen. The port also draws the road path.

- `gps_route_polyline()` (`gps.c`) replays the search's result (`ctlPath`:
  the start point plus the signpost decisions) the way
  `nodeClosestRoutePoint` walks it. Output restarts where the car is found on
  the path, so only the road ahead is returned, with one point every ≥ 6 m
  and the goal last. It returns nothing unless the GPS is following a path on
  the free-roam network.
- `drawWorldBottom` refreshes it every 250 ms (512 points at most) and
  `minimap_draw` draws it under every icon: a dark casing, then the line in
  the tracked target's colour, with round joints, all opaque so that overlaps
  blend nothing.

## 9. Reskinned menu screens

These screens keep all of their retail logic: input handlers, messages,
saves and sounds. Only the drawing is replaced, through hooks at the top of
`drawMenu()`, and taps are translated into the keys the screen already
answers.

- **Generic dialogs** (`drawUiDialog`): every `GenericDialog_*` raised
  through `g_feDialog`, in GenericDialog's shape: a wide black panel with the
  pale (197,218,165) ring. The screen behind is dimmed, the blurb is wrapped at
  16 px, and BuildFromConfig's buttons (Button1 / Button2 for Yes / No,
  Button3 alone for OK) are drawn as 36 px pills with A on the current one
  and B on No. A tap activates a button directly instead of needing a second
  tap. The blurb and labels are stored as plain text at load
  (`MenuState.dlgBlurb` / `dlgLabel`). The help dialog keeps its own layout
  because of its icon column.
- **World Map** (`drawUiWorldMap`, `uiWmTouch`): the in-game
  `UI_InGame_WorldMap`. The map is full screen over `GameMap`'s own window,
  zoom and region, with larger markers in per-type colours (`uiWmeColour`)
  and the GPS target ringed in its colour. On top: Back and the region
  switcher (`gameMapScrollZoom`). Zoom has + / − buttons, and L / R still
  zoom as before. `FILTERS n/m` opens a drawer of on/off rows; this is
  `gameMapToggleFilter`, i.e. retail's Show / Hide on World Map. The marker
  card's `SET GPS` / `GPS OFF` go through `gameMapAccept`, so retail's
  confirmation dialog still asks (now in the new dialog style). Up/down on
  the D-pad walks the drawer, left/right walks the markers, and drag pans.
  AI cars are not drawn on this map yet.
- **Mailbox** (`drawUiMailbox`): categories with arrows and position dots,
  30 px rows with the envelope in its `SMSMessageCategories` colour, unread
  rows marked, and `READ` / `DELETE`.
- **SMS reader** (`drawUiSmsReader`): a header with the sender's initial,
  `FROM …` and the subject. The body is wrapped to the panel and scrolled by
  drag or the D-pad in pixels (`g_smsReaderTop`), then `DRIVE` / `INBOX`.
  Both screens feed `smsBoxInput()`, so deleting still saves the profile,
  going back still sorts the read messages, and the prologue still gets its
  dialog-dismissed notification.

- **Icon hubs as a grid** (`drawUiIconGrid`, `uiGridLayout`, `uiGridNav`):
  LEFT / RIGHT keep the strip's order; UP / DOWN go to the nearest tile
  above / below. `UI_Main` has a hierarchy: Career and Quick Race as two
  large tiles, LAN / Customize / Profile Manager under them, Options as a
  gear in the header's left corner (still item 6 of the same list). Grids
  with more than two rows scroll. The body shop's categories
  (`UI_GenericParts_Browser`, plain and carbon modes, not while browsing
  parts) use the grid too, with the bank in the footer.
- **Garage Car Select** (`drawUiCarSelect`, `UI_CareerCarSelect`): the
  slots as numbered tabs across the top (a dot on the one being driven), the
  slot's card between two full-height arrows with the logo, badge art and
  name (or "open for sponsor car" / "locked"), `Select Car` and the bank.
  Taps go through `FSK_CAR`. The same skin (`uiCarSelScreen`) covers the
  Car Lot (the header says Buy / Trade In / pick your car to trade, A is
  Buy / Trade) and the Quick Race / LAN / Customize car selects: the view
  filter between L and R (tappable halves), the list position and a run bar,
  the row's kind (Stock / My Car / Career Car / Sponsor Car) and X as
  Customize, or Delete on a tuned car in Customize.
- **Body shop parts list** (`drawUiBodyParts`, while browsing, plain and
  carbon parts): a row per part with FITTED / OWNED / its price (the price
  only where retail prints it: a career, not from the crib), UP / DOWN and
  taps (`FSK_PART`), A as Buy or Install, the bank in the footer. The
  specialties (ICE), vinyl, decal, rim and HUD lists keep retail's layout.
- **World Map filters**: a list in a side sheet from the right (a row per
  type, a tick when shown, scrolls past ~6 rows) instead of wrapped pills
  that cut the names.
- **Marker taps**: on the World Map, a tap on a marker of another type now
  switches the cursor's row and rebuilds its list before moving the cursor
  (`gamemap_rebuild_locator`); before, the cursor stayed and the card showed
  the previous marker. In free roam, with the GPS engaged, the tap only
  considers the markers the map draws (the GPS's own), not hidden ones near
  the finger.

## 10. Drag Shift: ZL/ZR option

Options → Gameplay → **Drag Shift: ZL/ZR** (On / Off), per profile, stored in
`OptGameplay.dragTriggerShift`, a padding byte that retail leaves 0. Saves
made before the option existed read Off.

With the option On, in a drag race only, ZR shifts up and ZL shifts down, on
top of the control scheme's shift buttons. Whatever the scheme had on ZL / ZR
(camera, reset, look back, nitrous) stands down for the race. See `zShift` in
the driving input.

## 11. Screen inventory and what is left

The port draws about 90 `.fng` screens on the lower panel: every frontend
screen from `UI_Main` on (`g_feNavBottom`), every in-world overlay except
pause, post-race and sponsor (`worldUiOnBottom`), and the split screens
(Quick Race track and car select, the photo shoot). They fall into six
templates. Reworking a template changes every screen in its group.

| Template | Screens | Status |
|---|---|---|
| Dialogs | `GenericDialog_*`, `IG_GenericDialog_SMALL`, `HelpDialog_*`, `UI_InGameDialog`, `UI_Engage*Dialog`, `UI_Deleteprofile` | Done |
| Maps | `UI_InGame_WorldMap`, `UI_CareerWorldMap` | Done (garage map: `drawUiCribMap`) |
| Lists | `UI_SMS_Mailbox` (+ reader), parts / rims / vinyl / decal / ICE / neon / spinner lists, performance packages and parts (`drawUiPerfPackage`), performance brands (`drawUiPerfBrand`), `MC_List` (`drawUiMcList`), LAN servers / name / lobby / room (`drawUiLan`), `EA_Trax_Jukebox` (`drawUiTrax`), paint swatches (`drawUiPaint`) | Done (HUD list keeps its picker) |
| Status pages | `UI_Status_*`, reputation | Done (dyno results: `drawUiDynoResults`) |
| Icon menus | every icon menu on the lower screen (`uiGridScreen` no longer limits it to the hubs) | Done |
| Settings and selection | option rows, LAN rows, tuning sliders, car select, track select (`drawUiTrackSelect`) | Done, the dyno's torque / boost graph (`drawUiTuneGraph`), NOS graph (`drawUiTuneNos`) and run chart (`drawUiDynoChart`) included; their touch reads the skin's geometry (`g_uiTg`, `g_uiNos`) |

The skins only draw on the lower panel (`skinOk` in `drawMenu`): a split
screen's top half (the stats plate over the car) keeps its own widgets, and
the old overlays (`drawCarSelect`, `drawBodyShopParts`, `drawLanLegend`, the
performance parts list) stand down while a skin is live.

Not in scope: pause, post-race, sponsor, loading, splash and the top-screen
HUD, which stay on the top screen.

## 12. Testing status

- Seen on hardware (photos): earlier iterations only, namely the first
  square map with a grey bank bar, the first split race screen, and the text
  overflow in the SMS reader, the footers and the HUD that started this work.
- Built and installed with 3dsDL, but not yet seen on a console: the
  citro2d measurement fix, the current free-roam chips and destination strip,
  the current race panel, the GPS route and target colours, the finish flag,
  the Drag Shift option, and the reskinned dialogs, World Map, mailbox and
  reader. Feedback from a play session is still pending.
- Known gaps: no AI cars on the reskinned World Map; the help dialog and
  engage dialogs keep the old layout; the GPS distance is a straight line,
  not along the route.
