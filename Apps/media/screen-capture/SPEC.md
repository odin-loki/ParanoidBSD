<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Screen Capture

By Odin Loch

| | |
|---|---|
| ID | `ME05` (from CATALOGUE.md) |
| Bundle | `org.pbsd.screen-capture` |
| Category | media |
| Priority | P1 |
| Backing repos | FUSE (rasteriser screenshot for tests only; no screen capture code). No app exists. |
| Status | Planned. Nothing built. |

## 1. Purpose

Screen Capture takes screenshots and screen recordings, lets the user mark them up, and pulls text and colours off the screen. Everyone who writes documentation, reports bugs or shares what they see uses it. It replaces Spectacle, and it is the reference client of the screen portal: the only way any app gets pixels.

## 2. Scope

In scope:

- Screenshots: region, window, screen, all screens, scrolling, delayed, with or without pointer and decorations.
- Annotation editor with irreversible blur and redaction.
- Screen recording with system and microphone audio, webcam overlay, key and click display.
- Output as PNG, JPEG, WebP, AVIF, JPEG XL, GIF, WebM, MP4.
- OCR copy-text, colour pick, pixel ruler, pin to screen, history, share hand-off.
- Automatic redaction of notifications and password fields, done by the compositor and requested here.
- Global shortcuts.

Out of scope:

| Item | Owner |
|---|---|
| The source picker, the capture prompt, the capture indicator | SH04 Portals, SH02 Notifications |
| Producing frames, hiding protected surfaces | compositor behind `svc.portal.screen` (SH04, SH05) |
| Global shortcut registration and conflict handling | SH01 Desktop Shell, SY01 Settings |
| Raster editing beyond annotation | CR01 Image Editor |
| Trimming and editing recordings beyond start/end cut | CR02 Video Editor |
| Remote screen sharing in calls | CO05 Messenger, VM04 Remote Desktop (they use the same portal) |
| System-wide colour picker for other apps | SH04 (colour picker portal); this app offers the same tool for its own use |
| Magnification | AX02 Magnifier |
| Session recording of terminals | SY03 Terminal |
| Encoders and frame transport | Media pipeline, ME03 section 6.5 |

## 3. Users and top tasks

Personas:

- **Writer**: captures a region, adds an arrow and a number, pastes into a document.
- **Bug reporter**: records 20 s of a window with clicks shown, shares a WebM.
- **Support person**: must hide names and tokens before sending; the hiding must be real.
- **Developer or designer**: picks colours, measures pixels, pins a reference image above other windows.
- **Reader**: copies text out of an image or a video frame.

| # | Task | Target steps |
|---|---|---|
| T1 | Capture a region to the clipboard | 2 (`Super+Shift+S`, drag) |
| T2 | Capture the focused window | 1 (`Alt+Print`) |
| T3 | Capture the whole screen to a file | 1 (`Print`) |
| T4 | Add an arrow and text, then copy | 4 |
| T5 | Redact a line so it cannot be recovered | 2 (`R`, drag) |
| T6 | Record a window with system sound | 3 (`Super+Shift+R`, pick, Start) |
| T7 | Stop a recording | 1 (`Super+Shift+R` again or the indicator) |
| T8 | Copy text from the screen | 2 (`Super+Shift+T`, drag) |
| T9 | Pick a colour | 2 (`Super+Shift+C`, click) |
| T10 | Pin a capture on top | 1 (`P` in the overlay) |
| T11 | Capture a long scrolling page | 3 |
| T12 | Find last week's screenshot | 2 (open history, search) |
| T13 | Take a delayed capture of an open menu | 2 (set 3 s delay, capture) |

## 4. Features

### 4.1 Screenshot capture

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME05-F001 | Portal only | Every pixel comes from `svc.portal.screen`. The app holds no compositor privilege and cannot read the screen any other way. | M | 1 |
| ME05-F002 | Region | Frozen-frame overlay provided by the portal; drag a rectangle; handles to adjust; size label; arrow keys nudge 1 px, `Shift` 10 px; magnifier loupe at the pointer; `Enter` confirms. | M | 1 |
| ME05-F003 | Window | Hover highlights windows (the portal knows their bounds); click captures the window content, with or without decorations and shadow; transparent corners kept. | M | 1 |
| ME05-F004 | Focused window | One shortcut, no picker, if the user has granted "focused window without asking" (remembered grant). | M | 1 |
| ME05-F005 | Screen | One output; picker when there are several. | M | 1 |
| ME05-F006 | All screens | One image of the whole layout, or one file per output. | S | 1 |
| ME05-F007 | Last region again | Repeat the previous rectangle. | S | 1 |
| ME05-F008 | Fixed size and aspect | Region locked to a size (for example 1280×720) or a ratio. | S | 2 |
| ME05-F009 | Element snap | Region snaps to window and sub-surface edges reported by the portal; `Ctrl` disables snap. | C | 2 |
| ME05-F010 | Delay | 0–60 s countdown shown by the shell; lets menus and tooltips be opened first. | M | 1 |
| ME05-F011 | Pointer | Include or leave out the pointer; the portal delivers it as a separate layer so the choice can change afterwards in the editor. | M | 1 |
| ME05-F012 | Decorations and shadow | Include or leave out; background behind transparent windows: transparent, wallpaper, solid colour. | S | 1 |
| ME05-F013 | Scrolling capture | The user starts, scrolls the window, and stops; frames are stitched by overlap matching (own code) into one tall image. Detects fixed headers and footers. Optional auto-scroll through the a11y scroll action when the target allows it. | S | 2 |
| ME05-F014 | Scale | Capture at native pixels; mixed-scale layouts are captured per output and composed at the highest scale. | M | 1 |
| ME05-F015 | HDR and wide gamut | HDR outputs are tone mapped to SDR sRGB PNG by default; optional 10-bit AVIF/JPEG XL. Colour profile embedded. | S | 3 |
| ME05-F016 | After-capture action | Per mode: open editor, copy, save, copy and save, pin, show a toast with actions. | M | 1 |
| ME05-F017 | Quick actions toast | Thumbnail toast for 6 s: Edit, Copy, Save as, Share, Pin, Delete. Drag the thumbnail into another app. | M | 1 |
| ME05-F018 | Capture from command line | `screen-capture --region\|--window\|--screen [--delay=n] [--out=<handle>]`; still shows the portal prompt unless a remembered grant covers it. | S | 1 |
| ME05-F019 | Capture intent | Other apps ask for a screenshot through intent `pick` (image); the user does the capture and may annotate; the caller gets one image handle. | S | 2 |

### 4.2 Automatic redaction and protected content

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME05-F020 | Notification redaction | The capture request sets `hide_notifications`; the compositor leaves notification surfaces out of frames (blank, not blurred). Default on for recordings and delayed captures. | M | 1 |
| ME05-F021 | Password field redaction | Surfaces or regions marked `secret` by FUSE `lineEdit` password mode and by the keyring and auth prompts are filled with a flat pattern by the compositor. The app never receives those pixels. Cannot be turned off for auth prompts. | M | 1 |
| ME05-F022 | Protected windows | Windows whose app set "no capture" (SE04 Passwords, SE05 Vault, private browser windows by default) arrive as a labelled grey rectangle. | M | 1 |
| ME05-F023 | Do-not-disturb during recording | Optionally turns on do-not-disturb through `svc.notify` while recording. | S | 2 |
| ME05-F024 | Redaction report | The capture metadata from the portal lists what was hidden ("2 notifications, 1 password field"); shown in the toast and the editor status bar. | S | 1 |
| ME05-F025 | Local secret scan | Optional: after capture, OCR and pattern rules (e-mail, IBAN, card numbers with Luhn check, private-key headers, tokens by prefix) suggest redaction boxes. Suggestions only; local; needs `ai.local` for OCR. | C | 3 |
| ME05-F026 | Lock screen and login | Never capturable; the portal refuses. | M | 1 |

### 4.3 Annotation editor

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME05-F027 | Object model | Annotations are vector objects over the base image; selectable, movable, restylable until export. Saved in a project file (`.pbshot`). | M | 1 |
| ME05-F028 | Arrow | `A`. Straight, elbow, curved; head styles; width; colour; shadow or outline for contrast. | M | 1 |
| ME05-F029 | Line, rectangle, ellipse | `L`, `U`, `O`. Stroke, fill, rounded corners, dash; `Shift` constrains. | M | 1 |
| ME05-F030 | Freehand pen and highlighter | `F`, `H`. Smoothing; highlighter uses multiply blend. | M | 1 |
| ME05-F031 | Text | `T`. Font, size, colour, background box, outline; multi-line; Markdown-free plain text. | M | 1 |
| ME05-F032 | Numbered steps | `N`. Auto-incrementing badges 1, 2, 3 (or A, B, C); renumber on delete; optional attached label. | M | 1 |
| ME05-F033 | Callout | Speech-bubble box with a tail. | S | 2 |
| ME05-F034 | Redact | `R`. Solid box. On export the pixels under it are replaced in the base image; nothing of the original survives in the output file. | M | 1 |
| ME05-F035 | Blur and pixelate | `B`. On export the area is replaced by a mosaic or blur computed only from a 16×16 downsample of the region (block size at least 1/8 of the region's short side), so fine detail such as letters cannot be estimated back. A note says "Use Redact for text and numbers". | M | 1 |
| ME05-F036 | Irreversibility guarantee | Export flattens to pixels; redacted areas are overwritten before encoding; the output file has no layers, no undo data, no thumbnail of the original, no metadata of the original. Copy to clipboard gives the same flattened pixels. | M | 1 |
| ME05-F037 | Project files keep originals | A `.pbshot` project keeps the original so edits stay possible. The save dialog says so; "Share" never offers the project file by default. | M | 1 |
| ME05-F038 | Spotlight | Dim everything outside chosen rectangles. | S | 2 |
| ME05-F039 | Magnify inset | A zoomed circle or box of a region with a leader line. | C | 2 |
| ME05-F040 | Crop | `C`. Free or ratio; non-destructive in the project, real in the export. | M | 1 |
| ME05-F041 | Resize and canvas | Scale output; add margin, background colour, rounded corners, drop shadow, simple window frame. | S | 2 |
| ME05-F042 | Pointer layer | Show, hide, move or enlarge the captured pointer. | S | 2 |
| ME05-F043 | Stickers and stamps | Tick, cross, warning, keyboard-key caps; user image stamps. | C | 3 |
| ME05-F044 | Style presets | Colour, width and font presets; last used per tool; palette from the design tokens with contrast outline. | S | 1 |
| ME05-F045 | Select, align, order | Multi-select, align, distribute, bring forward, send back, group, duplicate, lock. | S | 2 |
| ME05-F046 | Undo and redo | Unlimited within the session; stored in the project. | M | 1 |
| ME05-F047 | Ruler and guides | Rulers in px, snap to guides and to object edges. | C | 2 |
| ME05-F048 | Merge images | Paste or drop another image as an object; join captures side by side or stacked. | C | 3 |
| ME05-F049 | Open an image | The editor opens any PNG, JPEG, WebP, AVIF, JPEG XL to annotate (decoded in the image codec helper). | S | 1 |
| ME05-F050 | Alt text | A description field stored in the PNG `iTXt` / XMP `dc:description`; optional local draft through `svc.model`. | C | 3 |

### 4.4 Screen recording

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME05-F051 | Sources | Region, window (follows the window when it moves; other windows on top are not shown), one screen, all screens. Chosen in the portal picker. | M | 2 |
| ME05-F052 | Start and stop | `Super+Shift+R` toggles; 3 s countdown (setting); stop from the shell indicator, the control strip or the shortcut. | M | 2 |
| ME05-F053 | Pause and resume | `Super+Shift+P`. | S | 2 |
| ME05-F054 | Control strip | A small floating strip (excluded from capture by the compositor): time, size, pause, stop, mic mute, webcam toggle, draw. | M | 2 |
| ME05-F055 | Indicator | The shell shows the screen indicator for the whole recording; the recorded area has a thin outline that is not in the output. | M | 2 |
| ME05-F056 | System audio | A monitor tap from `svc.media`, granted in the same portal dialog ("also record sound"): whole system, or only the recorded window's app. | M | 2 |
| ME05-F057 | Microphone | Through `svc.portal.mic` with its own prompt and indicator; level meter; noise suppression (shared with ME06); separate track or mixed. | M | 2 |
| ME05-F058 | Separate audio tracks | System and microphone as two tracks in Matroska/WebM for later mixing. | S | 2 |
| ME05-F059 | Webcam overlay | Through `svc.portal.camera`; corner or free position; circle or rectangle; size; mirrored; can be toggled during recording. Composited by this app. | S | 2 |
| ME05-F060 | Pointer | Show or hide; highlight ring; click ripples (left, right, middle in different shapes); enlarged pointer. | M | 2 |
| ME05-F061 | Key display | Shows shortcuts and typed keys as caps in a corner. Key events come from the portal as an opt-in stream tied to the capture (ME05-GAP-2); keys typed into `secret` fields are never delivered. Modes: shortcuts only (default), all keys. | S | 2 |
| ME05-F062 | Live drawing | Draw temporary arrows and marks during recording; they fade after 3 s. | C | 3 |
| ME05-F063 | Frame rate and resolution | 15, 24, 30, 60 fps; native or scaled (100%, 75%, 50%, 1080p, 720p). Variable frame rate: only changed frames are encoded. | M | 2 |
| ME05-F064 | Formats | WebM (VP9 or AV1 + Opus) default; Matroska; MP4 (AV1 + Opus); H.264 only with a restricted-pack encoder if shipped (ME03 section 6.5.3). Lossless option (FFV1 or VP9 lossless in Matroska). | M | 2 |
| ME05-F065 | GIF and animated images | Convert a recording or a selection to GIF (palette per frame, dithering, frame-rate and width limits), animated WebP or AVIF. Size estimate before export. | M | 2 |
| ME05-F066 | Trim | Set start and end, cut without re-encoding at keyframes or frame-exact with re-encode. More editing goes to CR02. | M | 2 |
| ME05-F067 | Crash-safe file | Matroska clusters flushed every 2 s; recovery on next start. | M | 2 |
| ME05-F068 | Limits | Maximum length, maximum size, low-disk stop at 500 MB free, with a notification. | M | 2 |
| ME05-F069 | Replay buffer | Optional: keep the last 30–300 s in memory and save on a shortcut. Needs a standing grant and shows the indicator the whole time. Off by default. | C | 3 |
| ME05-F070 | Hardware encode | Through the pipeline broker when available. | C | 3 |
| ME05-F071 | Performance guard | If encoding falls behind, lowers quality before frame rate, and reports drops at the end. | S | 2 |
| ME05-F072 | Chapters from markers | A shortcut drops a marker; saved as Matroska chapters. | C | 3 |
| ME05-F073 | Screenshot during recording | Normal screenshot shortcuts keep working. | S | 2 |

### 4.5 Tools: text, colour, ruler, pin

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME05-F074 | OCR copy-text | `Super+Shift+T`: drag a region; text is recognised through `svc.model` and copied; a small panel shows the text for correction. Layout modes: lines, paragraph, table (tab-separated). | M | 2 |
| ME05-F075 | OCR in the editor | Select recognised text directly on the image; `Ctrl+C`. | S | 2 |
| ME05-F076 | OCR languages | Chosen list; auto-detect among installed language models. | S | 2 |
| ME05-F077 | Code detection | A QR code or barcode inside a capture is decoded and shown as a safe card (same card as ME04 section 4.6; nothing opens by itself). | C | 3 |
| ME05-F078 | Colour pick | `Super+Shift+C`: loupe with a pixel grid; click copies the colour. Formats: `#RRGGBB`, `rgb()`, `hsl()`, `oklch()`, float; the value is stated in sRGB and in the display's space. | M | 2 |
| ME05-F079 | Palette history | Last 32 picked colours; named palettes; export as GPL or CSS variables. | S | 2 |
| ME05-F080 | Contrast check | Pick two colours; WCAG contrast ratio shown. | C | 2 |
| ME05-F081 | Pixel ruler | `Super+Shift+M`: drag to measure width, height, diagonal and angle in physical pixels and scaled units; edge-snap mode measures gaps between detected edges. | S | 2 |
| ME05-F082 | Pin to screen | A captured region floats as a borderless always-on-top window (SH05): move, scale with wheel, opacity with `Ctrl`+wheel, click-through toggle, close with `Esc`. Pins are excluded from later captures by setting. | M | 2 |
| ME05-F083 | Pin text or colour | Pin OCR text or a colour swatch as a small note. | C | 3 |
| ME05-F084 | Frozen-frame tools | Colour pick, ruler and OCR work on one frozen frame from the portal, so they need a single capture grant, not a live stream. | M | 2 |

### 4.6 Save, history, share

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME05-F085 | Formats | PNG (default), JPEG, WebP, AVIF, JPEG XL; quality settings; PNG optimised (zlib level, palette when under 256 colours). | M | 1 |
| ME05-F086 | Save location | `~/Pictures/Screenshots` and `~/Videos/Screen recordings`, granted once; "Save as" through the file portal. | M | 1 |
| ME05-F087 | File names | Template `{kind} {YYYY}-{MM}-{DD} {HH}.{mm}.{ss} {app}`; `{app}` is the window's app name only if "include window title or app" is on (off by default). | M | 1 |
| ME05-F088 | Metadata | Only: creation time, software name, colour profile, pixel density. No host name, user name or window title unless enabled. | M | 1 |
| ME05-F089 | Clipboard | Image as PNG plus a file handle; for recordings a file handle. Sensitive flag set when the redaction report was not empty or on request. | M | 1 |
| ME05-F090 | History | Grid of recent captures and recordings with search by date, kind, app name (if recorded) and OCR text (if indexed). Stored as normal files plus an index; retention by days or count. | M | 2 |
| ME05-F091 | History privacy | History can be off; "private capture" skips history, thumbnails and `svc.search`; "Clear history" removes index and, by choice, files. | M | 2 |
| ME05-F092 | Share | Share sheet (SH04) with the flattened export; never the project file unless picked on purpose. | M | 1 |
| ME05-F093 | Send to | Open in CR01, CR02, ME01, attach in CO01, insert in OF02/OF04 by intent. | S | 2 |
| ME05-F094 | Drag out | Drag the toast or history thumbnail into any app. | M | 1 |
| ME05-F095 | Print | Print through the print portal. | C | 3 |
| ME05-F096 | No upload service | The app has no network capability and no upload feature. | M | 1 |
| ME05-F097 | Auto-copy path or image | After-save options: copy image, copy file handle, copy nothing. | S | 1 |

### 4.7 Shortcuts, tray, settings, recovery

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME05-F098 | Global shortcuts | Registered with the shell (ME05-GAP-3): `Print` screen, `Alt+Print` window, `Shift+Print` region, `Super+Shift+S` region to clipboard, `Super+Shift+R` record, `Super+Shift+T` text, `Super+Shift+C` colour, `Super+Shift+M` ruler. All rebindable in SY01. | M | 1 |
| ME05-F099 | Launch on shortcut | The shell starts the app for a shortcut; the app does not need to run in the background for screenshots. | M | 1 |
| ME05-F100 | Background only while needed | `bg.run` is used while recording, while pins exist or while the replay buffer is on. No `bg.autostart`. | M | 2 |
| ME05-F101 | Capture window | A small window with mode buttons, options and the last captures, for pointer-driven use. | M | 1 |
| ME05-F102 | Remembered grants | "Focused window without asking" and "this screen without asking" can be remembered by the portal per app; shown and revocable in AD05. Recording always asks. | M | 1 |
| ME05-F103 | Profiles | Named capture presets (mode, delay, pointer, after-action, format) each bindable to a shortcut. | C | 3 |
| ME05-F104 | Recovery | Unsaved editor sessions and unfinished recordings are restored after a crash. | M | 1 |
| ME05-F105 | Audit | Every capture start and stop is logged by the portal in `svc.audit` with mode and target app; viewable in AD04. | M | 1 |
| ME05-F106 | Bus interface | `org.pbsd.screencapture.Capture1`: request capture (always user-mediated), open editor with a handle. | S | 2 |
| ME05-F107 | Accessibility capture | Captures can include the a11y tree text of the captured window as alt text when the user asks (from `svc.a11y`, same grant). | C | 3 |

## 5. GUI

### 5.1 Window map

```
Screen Capture
├─ Capture window (small launcher)
├─ Shell-drawn through svc.portal.screen (SH04):
│  ├─ Region overlay (frozen frame, selection, loupe, toolbar)
│  ├─ Window and screen picker
│  ├─ Recording consent dialog (source, sound, remember)
│  └─ Countdown and capture indicator
├─ Quick actions toast (svc.notify with thumbnail and actions)
├─ Editor window (one per capture)
│  ├─ Tool rail (left) · Canvas · Properties (right, F4) · Status bar
│  └─ Dialogs: Export · Resize and canvas · Save project
├─ Recording control strip (floating, excluded from capture)
├─ Recording review window (player, trim, export, GIF)
├─ Tools: OCR panel · Colour loupe result · Ruler overlay · Pins
├─ History window
├─ Preferences: General · Screenshots · Recording · Audio · Editor · Tools ·
│               Privacy and redaction · History · Shortcuts
└─ Shell-drawn: file portal, share sheet, mic and camera prompts
```

### 5.2 Main window

The editor is the main working window. Shown at 1100×700.

```
+------------------------------------------------------------------------------------------+
| [=] Screen Capture — Region 2026-11-12 14.03.22*       [Copy] [Save] [Share] [▁][▢][✕]  |(1)
+----+--------------------------------------------------------------------+----------------+
| ↖  |                                                                    | Arrow      (4) |
| A→ |      +----------------------------------------------------+        | Colour [■ red] |
| ▭  |      |  captured image                                    |        | Width  ─●── 4  |
| ◯  |      |        ①──────────►  [Save] button                 |        | Head   [▶ v]   |
| ✎  |      |                                                    |        | Style  [solid v]|
| ▒H |      |   ████████████████  (redacted)                     |        | [x] Outline    |
| T  |      |                                                    |        |----------------|
| ①  |      |        ②  "Click here second"                      |        | Objects        |
| ██R|      |                                              (3)   |        |  ① step        |
| ░B |      +----------------------------------------------------+        |  → arrow       |
| ⌗C |                                                                    |  █ redact      |
| 🔍 |                                                                    |  ② step + text |
|(2) |                                                                    |                |
+----+--------------------------------------------------------------------+----------------+
| 1280×720 · 100% · hidden by system: 1 notification · pointer layer: shown    [OCR text] |(5)
+------------------------------------------------------------------------------------------+
```

1. Header: title with `*` when unsaved; Copy (primary), Save, Share.
2. Tool rail (`toolButton` column): select, arrow, rectangle, ellipse, pen, highlighter, text, number, redact, blur, crop, magnify. Each shows its key in the tooltip.
3. Canvas: needs the 2D canvas widget (G-12). Phase 1 fallback: `viewportWidget` painting a composed image with hit-testing in ForgeScript.
4. Properties (`propertyGrid`) for the active tool or selection; object list (`listView`) with lock and visibility.
5. Status bar: size, zoom, the redaction report from the portal, pointer layer toggle, OCR button.

States: empty (editor opened with no image: "Take a capture or open an image"), loading (decode of a large image), error (unsupported file), read-only (image opened from a read-only place: export only), unsaved (`*`), recovered ("Restored after a crash" bar).

### 5.3 Other views and dialogs

**Capture window** (420×360)

```
+------------------------------------------------+
| [=] Screen Capture                  [▁] [✕]   |
| [ Region ] [ Window ] [ Screen ] [ All ]       |
| [ Scrolling ] [ Record ● ]                     |
| Delay [0 s v]  [x] Pointer  [ ] Decorations    |
| After capture [Open editor v]                  |
| Tools: [Text] [Colour] [Ruler] [History]       |
| Recent  +----+ +----+ +----+ +----+            |
|         |    | |    | | ▶  | |    |            |
|         +----+ +----+ +----+ +----+            |
| Hidden automatically: notifications, passwords |
+------------------------------------------------+
```

**Region overlay** (drawn by the shell on a frozen frame; this app supplies options only)

```
+------------------------------------------------------------------------------------------+
|  (dimmed frozen screen)                                                                  |
|            ┌──────────────────────────────────────┐ 640 × 360                            |
|            │                                      │                                      |
|            │        selected region               │     ┌─────────┐ loupe 8×            |
|            │                                      │     │ ▦▦▦▦▦▦▦ │ #1F5FD6             |
|            └──────────────────────────────────────┘     └─────────┘ x 812 y 440         |
|            [Copy Enter] [Save Ctrl+S] [Edit E] [Pin P] [Text T] [Record R] [✕ Esc]       |
+------------------------------------------------------------------------------------------+
```

The overlay belongs to the portal so that no app sees the screen before the user confirms a region. States: multi-monitor (overlay on every output), nothing selected (hint text), too small (under 4×4 px: ignored).

**Recording consent** (drawn by the shell; fields requested by this app)

```
+--------------------------------------------------------------+
| Screen Capture wants to record                               |
| What   (•) Window: Text Editor — notes.md                    |
|        ( ) Screen 1 (2560×1440)   ( ) A region               |
| Sound  [x] System sound: only this window's app              |
|        [ ] Microphone (asks separately)                      |
| Also   [x] Show clicks   [ ] Show keys (shortcuts only)      |
| Always hidden: notifications, password fields, locked apps.  |
|                               [Cancel]  [Start recording]    |
+--------------------------------------------------------------+
```

**Recording control strip** (floating, 360×44)

```
+----------------------------------------------------------+
| ● 00:42  18 MB   [⏸] [■ Stop]  [mic ▮▮▮▯] [cam] [✎] [⋮] |
+----------------------------------------------------------+
```

**Recording review** (900×600)

```
+----------------------------------------------------------------------------------+
| [=] Recording 2026-11-12 14.10.05           [Save] [Export GIF…] [Share] [✕]    |
+----------------------------------------------------------------------------------+
|                                                                                  |
|                              VIDEO PREVIEW                                       |
|                                                                                  |
+----------------------------------------------------------------------------------+
| [▶] 00:07 [──[████████████████████████]────────] 00:42   start 00:03  end 00:31  |
| Tracks: [x] video  [x] system sound  [x] microphone       [Open in Video Editor] |
| WebM VP9+Opus · 1280×720 · 30 fps (variable) · 18 MB → trimmed about 12 MB       |
+----------------------------------------------------------------------------------+
```

`videoWidget` plus a trim `slider` with two handles. States: recovering, exporting (progress), encoder error.

**Export GIF** (dialog, 520×360)

```
+------------------------------------------------------+
| Export animation                                     |
| Format (•) GIF  ( ) Animated WebP  ( ) Animated AVIF |
| Width [640] px   Frame rate [12 v]   Loop [x]        |
| Colours [256 v]  Dither [ordered v]                  |
| Range 00:03 – 00:31 (28 s, 336 frames)               |
| Estimated size 6.8 MB  ⚠ large for a GIF; try WebP   |
|                          [Cancel]  [Export GIF]      |
+------------------------------------------------------+
```

**OCR panel** (popover, 420×260)

```
+----------------------------------------------+
| Text from screen (on this device)   [English]|
| +------------------------------------------+ |
| | error: could not open file "config.toml" | |
| | at line 42                               | |
| +------------------------------------------+ |
| Copied. [Copy again] [As table] [Pin] [✕]    |
+----------------------------------------------+
```

**History window** (900×600): toolbar with search `lineEdit`, filter chips (Screenshots, Recordings, Pinned), `gridView` of thumbnails grouped by day, inspector with file facts and actions; empty state "No captures yet. Press Print."

**Export image** (dialog): format, quality, scale, background and frame options, a line "Redacted areas are overwritten. This file cannot be un-redacted.", `[Export]`.

### 5.4 Menus

Editor window:

```
File     New capture ▸ (Region Shift+Print · Window Alt+Print · Screen Print · Scrolling · Record) ·
         Open image… Ctrl+O · Save Ctrl+S · Save as… Ctrl+Shift+S · Save project… ·
         Export… Ctrl+E · Share… · Print… Ctrl+P · History Ctrl+Y · Close Ctrl+W · Quit Ctrl+Q
Edit     Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut Ctrl+X · Copy image Ctrl+C · Copy selection ·
         Paste Ctrl+V · Duplicate Ctrl+D · Delete · Select all Ctrl+A · Preferences Ctrl+,
Tools    Select S · Arrow A · Line L · Rectangle U · Ellipse O · Pen F · Highlighter H · Text T ·
         Number N · Callout K · Redact R · Blur B · Spotlight · Magnify inset · Crop C ·
         Pick colour I · Measure M
Image    Resize… · Canvas and frame… · Pointer layer ▸ · Rotate ▸ · Flip ▸ · Suggest redactions… ·
         Recognise text Ctrl+T · Alt text…
Arrange  Bring forward Ctrl+] · Send back Ctrl+[ · Align ▸ · Distribute ▸ · Group Ctrl+G ·
         Ungroup Ctrl+Shift+G · Lock
View     Zoom in Ctrl++ · Zoom out Ctrl+- · Fit Ctrl+0 · 100% Ctrl+1 · Rulers · Guides ·
         Properties F4 · Pin this capture P · Full screen F11
Help     Help F1 · Shortcuts · About
```

### 5.5 Keyboard and pointer

Global shortcuts are in section 4.7. In the region overlay: arrows nudge, `Shift`+arrows resize, `Space`+drag moves the selection, `Tab` cycles window/region/screen mode, `Ctrl+A` whole screen, number keys `1`–`9` pick an output, `Esc` cancels. In the editor: single-letter tool keys from 5.4, `Shift` constrains, `Alt` draws from the centre, `[`/`]` width, `Enter` commits text, arrow keys move the selection 1 px (`Shift` 10 px).

| Context | Menu items |
|---|---|
| Canvas object | Edit · Duplicate · Bring forward · Send back · Lock · Copy style · Paste style · Delete |
| Canvas background | Paste · Recognise text · Pick colour here · Add guide · Crop to selection |
| History item | Open in editor · Copy · Pin · Share… · Show in Files · Rename F2 · Move to trash |
| Pin | Copy · Save as… · Opacity ▸ · Click-through · Scale ▸ · Close |
| Control strip | Pause · Stop · Discard recording · Mute microphone · Hide strip (shortcut still works) |

Drag and drop: images into the editor (new object or new document); thumbnails out to any app (flattened PNG and a file handle); a colour swatch out as text. Wheel: zoom at pointer in the editor and loupe; scale on pins.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Capture window with mode buttons; history empty text. |
| Loading | "Preparing capture…" appears only if the portal takes over 300 ms. |
| Error | Portal refused (policy, lock screen): toast with the reason. Encoder failure: recording saved up to the failure. |
| Denied | The user cancelled or denied: nothing is captured, no error dialog, a quiet toast "Capture cancelled". |
| Offline | Not relevant; the app has no network use. |
| Locked | On session lock, recording pauses and the stream closes; it does not resume by itself. |
| Read-only | Save folder not writable: capture kept in the editor, bar offers "Choose folder". |
| Narrow (<640) | Editor properties become a bottom sheet; tool rail scrolls; history shows two columns. |
| Protected content | Grey labelled rectangles in the image; status bar says what was hidden. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Capture window | `toolButton`, `comboBox`, `checkBox`, `gridView` |
| Editor canvas | 2D canvas (G-12); fallback `viewportWidget` |
| Tool rail | `toolButton` |
| Properties | `propertyGrid`, `colorPicker`, `slider`, `fontPicker`, `listView` |
| Review player | `videoWidget` (G-11), `slider` |
| History | `gridView`, `lineEdit`, `modelListView` over `SqliteModel` |
| OCR panel | `codeEdit` (plain text), `tableView` (table mode) |
| Pins | borderless `AppWindow` with `image` |
| Control strip | `toolButton`, `progressBar` (level), `label` |

Missing: G-12, G-11. `ME05-GAP-1`: the `svc.portal.screen` interface (frozen-frame region overlay, window and output streams, separate pointer layer, redaction flags and report, capture-excluded surfaces, per-app audio tap) is not defined in any document read. `ME05-GAP-2`: opt-in input-event stream (keys, clicks) bound to a capture session. `ME05-GAP-3`: global shortcut registration service in the shell (not listed in PLATFORM.md section 8). `ME05-GAP-4`: a surface flag for `secret` regions in FUSE widgets and the compositor protocol. `ME05-GAP-5`: always-on-top, click-through window type for pins (SH05).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Modes, after-capture actions, editor document model, history, settings |
| UI | FUSE UI | app host | Section 5 except shell-drawn parts |
| Screen portal | C++23 | system (`svc.portal.screen`, compositor side) | Pickers, overlay, consent, frames, redaction, indicator |
| Media session client | C++23 (platform, ME03 section 6.5) | app host | Receives frame streams and audio rings; record graph; webcam composite |
| `flatten-helper` | C++23 | helper, no capabilities | Renders annotations onto pixels, overwrites redacted areas, crops, scales |
| `image-codec-helper` | C++23 plus vendored codecs | helper, no capabilities | Encode and decode stills (ME01 section 6.3) |
| `encode-helper` | C++23 plus vendored encoders | helper, no capabilities | Video encode, mux, GIF and animated image encode; writes to a passed descriptor |
| `stitch-helper` | C++23 | helper, no capabilities | Scrolling capture overlap matching |
| OCR | — | `svc.model` | Text recognition |
| Shortcut registration | — | shell (ME05-GAP-3) | Starts the app with an action |

```
 shell shortcut ─► app host (ForgeScript + FUSE UI) ◄── frames (shm), pointer layer, report ──┐
                        │      │         │                                                    │
                        │      │         └─ request {mode, flags} ─► svc.portal.screen ───────┘
                        │      │                                     (compositor: picker,
            base image  │      │ frames + PCM rings                   overlay, redaction,
            + objects   │      ▼                                      indicator)
                 ┌──────▼─────┐  ┌──────────────┐   svc.media (system sound tap) ─┐
                 │ flatten-   │  │ encode-helper│◄── svc.portal.mic ──────────────┤
                 │ helper     │  │ (video, GIF) │◄── svc.portal.camera (overlay) ─┘
                 └──────┬─────┘  └──────┬───────┘
                        ▼ pixels        ▼ file fd
                 image-codec-helper ─► file fd / clipboard
```

### 6.2 Data model

Stores: `~/.local/share/org.pbsd.screen-capture/history.sqlite`; editor autosave in `~/.local/share/org.pbsd.screen-capture/sessions/`; thumbnails in `~/.cache/org.pbsd.screen-capture/thumbs/`.

| Entity | Fields |
|---|---|
| `capture` | `id INTEGER PK`, `kind TEXT (region\|window\|screen\|all\|scroll\|recording\|ocr\|colour)`, `grant_ref TEXT`, `file_name TEXT`, `created_at INTEGER`, `width`, `height`, `duration_ms`, `format TEXT`, `app_name TEXT NULL`, `redaction_json TEXT`, `ocr_text TEXT NULL`, `pinned INTEGER`, `project_ref TEXT NULL`, `private INTEGER` |
| `colour` | `id`, `srgb_hex TEXT`, `display_json TEXT`, `picked_at`, `palette_id` |
| `palette` | `id`, `name` |
| `preset` | `id`, `name`, `mode`, `delay_s`, `pointer INTEGER`, `after_action TEXT`, `format TEXT`, `shortcut TEXT` |
| `style_preset` | `tool TEXT`, `props_json TEXT` |

Project file `.pbshot` (documented, open): one JSON header followed by PNG blobs.

```
magic "PBSHOT1\n"
header JSON: { "version":1, "size":[w,h], "scale":2.0, "base":{"offset":n,"len":n},
               "pointer":{"offset":n,"len":n,"pos":[x,y],"visible":true},
               "objects":[ {"id":1,"type":"arrow","points":[[x,y],[x,y]],"stroke":"#E5534B","width":4},
                           {"id":2,"type":"redact","rect":[x,y,w,h]},
                           {"id":3,"type":"blur","rect":[x,y,w,h],"mode":"mosaic"},
                           {"id":4,"type":"step","n":1,"pos":[x,y],"label":"…"},
                           {"id":5,"type":"text","pos":[x,y],"text":"…","font":"…","size":15} ],
               "crop":[x,y,w,h], "canvas":{"margin":0,"bg":null}, "redaction_report":{…} }
blobs: base PNG, pointer PNG, stamp PNGs
```

Recording journal: `<file>.part.json` as in ME04 section 6.2.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| PNG, JPEG, WebP, AVIF, JPEG XL | yes | yes | image codec helper (ME01 section 6.3) | image-codec-helper |
| GIF | yes | yes | own encoder: median-cut or k-means palette, ordered and error-diffusion dither, frame differencing, LZW | encode-helper |
| Animated WebP, animated AVIF | no | yes | libwebp (BSD-3-Clause), AVIF muxer + SVT-AV1 | encode-helper |
| WebM, Matroska, fragmented MP4 | own files (review, trim, recover) | yes | pipeline muxers and demuxers (ME03 section 6.5.2) | encode-helper / demux-helper |
| VP9, AV1, FFV1, Opus, FLAC | review | yes | libvpx, SVT-AV1, own FFV1, libopus, libFLAC (ME03 section 6.5.3) | encode-helper / decode-helper |
| `.pbshot` project | yes | yes | own code; strict length checks | flatten-helper (blobs), app host receives the validated header |
| Portal frame stream | yes | — | shared-memory frame pool defined by ME05-GAP-1 | app host (trusted system source) |
| GPL palette, CSS variables | — | yes | ForgeScript | app host |
| Barcodes in captures | yes | no | decoders from ME04 (frame-analysis helper) | frame-analysis-helper |

### 6.4 Integration

- Intents handled: `pick` (screenshot for another app), `edit` / `open` for images (annotate). Emits: `share`, `edit` (CR01, CR02), `open` (ME01, ME03), `compose` (CO01), `print`.
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.screen`, `svc.portal.mic`, `svc.portal.camera`, `svc.intent`, `svc.search` (history, optional), `svc.clipboard`, `svc.media`, `svc.power` (inhibit sleep while recording), `svc.model`, `svc.undo-trash`, `svc.audit`, `svc.a11y` (scroll action, alt text; optional).
- Offers: `org.pbsd.screencapture.Capture1`.
- Other apps: SH04 (portal, picker, overlay), SH05 (pins, window bounds), SH02 (indicator, do-not-disturb), SH01 (shortcuts), SY01 (shortcut settings), AD05 (remembered grants), AD04 (audit view), ME04 (webcam path, barcode cards), ME06 (noise suppression code), ME01 (image codec helper), ME03 (pipeline), CR01, CR02, AX02 (shares the loupe idea, not code), AI02 (OCR model), CO05 and VM04 (other portal clients).
- Backing repos, Verified: FUSE can render its UI to a PNG through the CPU rasteriser for tests (`FUSE:docs/ui/README.md`, `fuse_ui_sample out.png`, designer `--screenshot`). That is a render of one app's own tree, not screen capture. The plan in this repo's P5-02 uses KWin, which has its own capture path; it is being replaced (PLATFORM.md V20). Everything here is Proposed.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | History index, autosave |
| `fs.portal` | Save folders, open image |
| `dev.screen` | The purpose of the app; every capture goes through the portal; recording always asks |
| `dev.mic` | Narration; separate prompt |
| `dev.camera` | Webcam overlay; separate prompt |
| `dev.gpu.compute` | Scaling, compositing, blur |
| `ai.local`, `svc.model` | OCR, optional redaction suggestions |
| `proc.helper:flatten-helper`, `image-codec-helper`, `encode-helper`, `stitch-helper`, `demux-helper`, `decode-helper`, `frame-analysis-helper` | Section 6.1 |
| `bg.run` | Recording, pins, replay buffer |
| `net.none` | No network use at all |
| `svc.*` listed in 6.4 | As described |

Not requested: any `net.*` beyond `net.none`, `dev.location`, `proc.exec`, `bg.autostart`, `fs.path:*`.

| Asset | Adversary | Attack surface | Mitigation |
|---|---|---|---|
| Everything on screen | This app if compromised; any app asking for the same capability | Screen frames | Portal-only pixels; user picks the source in a shell-drawn dialog; indicator; recording never has a silent grant; audit; AD05 revocation |
| Secrets on screen (passwords, notifications, vault windows) | Recipient of a capture; this app | Frames | Compositor-side redaction: those pixels never reach the app; report shows what was hidden |
| Text under redaction boxes | Recipient with image tools | Layers, blur inversion, thumbnails, metadata, clipboard formats | Flatten in a helper, overwrite before encode, coarse mosaic from a 16×16 downsample, no embedded original, same pixels on clipboard; proof target T1 on the overwrite |
| Typed keys | This app; recipients | Key display stream | Opt-in per recording; shortcuts-only default; secret fields excluded by the portal; never stored outside the video |
| Other people's audio | — | System sound tap | Stated in the consent dialog; per-app tap by default; indicator |
| Capture history | Other local users | Files, index, OCR text, `svc.search` | History off or private capture; retention; no window titles by default |
| The app host | Crafted image opened for annotation, crafted `.pbshot` | Decoders, project parser | Zero-capability helpers; strict project format |
| The user's attention | App that triggers captures through the bus | Bus interface, `pick` intent | Every request is user-mediated by the portal |

Stored: captures as files the user owns, a history index, optional OCR text, colours, presets. Leaves the machine: nothing. The manifest declares `net.none`.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `shot.after_region` | enum `editor\|copy\|save\|copy_save\|toast` | `copy` | Action after a region capture |
| `shot.after_window` / `shot.after_screen` | enum | `toast` / `save` | Action after other modes |
| `shot.format` | enum `png\|jpeg\|webp\|avif\|jxl` | `png` | Image format |
| `shot.quality` | int | 90 | Lossy quality |
| `shot.pointer` | bool | false | Include the pointer |
| `shot.decorations` | bool | true | Include decorations |
| `shot.shadow` | bool | false | Include window shadow |
| `shot.delay_s` | int | 0 | Default delay |
| `shot.folder` | grant | `~/Pictures/Screenshots` | Screenshot folder |
| `names.template` | string | `{kind} {YYYY}-{MM}-{DD} {HH}.{mm}.{ss}` | File names |
| `names.include_app` | bool | false | Put the app name in names and history |
| `rec.container` | enum `webm\|mkv\|mp4` | `webm` | Container |
| `rec.codec` | enum `vp9\|av1\|lossless` | `vp9` | Video codec |
| `rec.fps` | int | 30 | Frame rate |
| `rec.scale` | enum `native\|75\|50\|1080p\|720p` | `native` | Output scale |
| `rec.countdown_s` | int | 3 | Countdown |
| `rec.system_audio` | enum `off\|app\|all` | `app` | System sound source |
| `rec.mic` | bool | false | Record the microphone |
| `rec.separate_tracks` | bool | true | One track per audio source |
| `rec.show_clicks` | bool | true | Click ripples |
| `rec.show_keys` | enum `off\|shortcuts\|all` | `off` | Key display |
| `rec.webcam` | bool | false | Webcam overlay |
| `rec.dnd` | bool | true | Do-not-disturb while recording |
| `rec.max_minutes` | int | 0 (none) | Length limit |
| `rec.folder` | grant | `~/Videos/Screen recordings` | Recording folder |
| `redact.hide_notifications` | bool | true | Ask the compositor to leave notifications out |
| `redact.suggest` | bool | false | Suggest redactions with OCR |
| `editor.blur_mode` | enum `mosaic\|blur` | `mosaic` | Obscure style |
| `tools.ocr_languages` | list | system language | OCR languages |
| `tools.colour_format` | enum `hex\|rgb\|hsl\|oklch` | `hex` | Copied colour format |
| `pins.exclude_from_capture` | bool | true | Leave pins out of captures |
| `history.enabled` | bool | true | Keep history |
| `history.retention_days` | int | 90 | Index retention (files are kept) |
| `history.index_text` | bool | false | OCR-index captures for search |
| `replay.seconds` | int | 0 (off) | Replay buffer length |

## 9. Accessibility and languages

- Every capture mode has a keyboard path: the region overlay can be driven fully with arrows and `Enter`; window mode can cycle windows with `Tab`.
- The overlay announces size and position on demand; the countdown is spoken.
- Editor objects are in the a11y tree with type and text ("arrow from 120, 80 to 300, 140"; "redaction 200 by 24"); they can be created with the keyboard at the canvas centre and moved with arrows.
- Alt text field for exported images; optional local draft description.
- Annotation default colours come with an outline so they meet 3:1 against any background.
- Recording indicators and countdown do not rely on colour or motion.
- OCR supports the languages for which models are installed; right-to-left text is copied in logical order.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Shortcut to frozen overlay visible | 150 ms (shell and portal; app start is not on this path) |
| Region confirmed to image on clipboard | 100 ms for 4K PNG at fast compression |
| App cold start to capture window | 150 ms |
| Editor open with a 4K image | 250 ms |
| Recording overhead | 1440p30 VP9 software under 2 cores; under 5% extra compositor frame time |
| Dropped frames while recording 1080p60 | under 1% with hardware encode, under 5% software on 4 cores; always reported |
| GIF export | 10 s of 640 px at 12 fps in under 3 s |
| OCR of a 1000×300 region | under 500 ms on CPU |
| Scrolling capture stitch | 20 frames of 1440 px width in under 2 s |
| Memory | app host under 150 MB; replay buffer bounded by setting (compressed) |
| Idle | Not running unless a window, pin or recording exists |

## 11. Testing and proof

Nothing has run.

- Unit: file naming, after-capture routing, object model operations, renumbering, project read and write, history retention.
- Golden: `.fuseui` renders; flatten output for each object type against reference images; GIF frames against references.
- Scripted UI with a synthetic portal source: each capture mode, cancel, deny, delayed capture, editor round trip, record-pause-stop, trim, GIF export.
- Redaction tests: exported files and clipboard data searched for any original pixel under redact boxes (byte-exact and correlation tests); a recovery attempt on mosaic output must fail a similarity threshold; no extra chunks or metadata in the output.
- Portal contract tests (with SH04): secret surfaces never appear in frames; notifications absent when flagged; lock screen refused.
- Fuzz: `.pbshot` parser, stitch matcher, GIF encoder input paths, recording recovery.
- Differential: GIF output decoded by an independent decoder equals the quantised frames; OCR positions.
- Soak: 2 h recording with variable frame rate, memory flat; kill test for crash-safe files.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Redaction overwrite in `flatten-helper` | Every output pixel inside a redact rectangle equals the fill colour and depends on no input pixel | T1 | Proof of the overwrite loop and of ordering (overwrite before any encode path) | — |
| Mosaic and blur | Output inside the region is a function of the 16×16 downsample only | T1 | Proof over the data-flow of the function | — |
| `.pbshot` parser | No runtime errors; bounded memory | T2 | PRISM; fuzz | — |
| Flatten renderer (shapes, text) | No out-of-bounds writes | T2 | PRISM; goldens | Text shaping is in the FUSE text stack (T3) |
| GIF encoder | No runtime errors; valid stream | T2 | PRISM; differential decode | — |
| Stitch matcher | Bounded time and memory | T2 | PRISM; fuzz | Quality is tested, not proved |
| Export metadata writer | Only allow-listed fields are written | T1 | Shared allow-list proof with ME01 stripper | — |
| Portal use | The app has no other pixel source | T3 | Manifest and sandbox tests (no compositor socket rights) | Depends on G-06, ME05-GAP-1 |
| Vendored encoders | Contained | T4 | Helper sandbox | Third-party C |
| App logic | Language-level memory safety | T3 | Interpreter; unit tests | — |

## 12. Build plan

### Phase 1: screenshots and editor

| ID | Work item | Depends on |
|---|---|---|
| ME05-W01 | Screen portal interface for stills: region overlay, window, output, pointer layer, redaction flags and report (define with SH04) | ME05-GAP-1, ME05-GAP-4, SH04, SH05, G-09 |
| ME05-W02 | Global shortcuts that start the app with an action | ME05-GAP-3, SH01 |
| ME05-W03 | Capture window, after-capture actions, quick actions toast | G-01, G-02, `svc.notify` |
| ME05-W04 | Save, copy, file names, metadata allow-list | ME01 image codec helper, `svc.clipboard` |
| ME05-W05 | Editor: object model, arrow, shapes, pen, text, numbers, crop, undo | G-12 (fallback `viewportWidget`) |
| ME05-W06 | `flatten-helper` with redact and mosaic; export | FORGE-WP-27, FORGE-WP-28 |
| ME05-W07 | Project format and autosave recovery | — |
| ME05-W08 | Sandbox test: no pixel path except the portal | G-06, G-18 |

### Phase 2: recording and tools

| ID | Work item | Depends on |
|---|---|---|
| ME05-W09 | Portal streams for recording, consent dialog fields, capture-excluded surfaces | ME05-GAP-1, SH04 |
| ME05-W10 | Record graph, control strip, crash-safe files, limits | ME03 `encode-helper`, G-11 |
| ME05-W11 | System sound tap and microphone tracks | `svc.media`, FUSE-R-088, ME06 noise suppression |
| ME05-W12 | Webcam overlay | ME04 camera portal work (ME04-GAP-1) |
| ME05-W13 | Click display and key display | ME05-GAP-2 |
| ME05-W14 | Review window, trim, GIF and animated export | — |
| ME05-W15 | OCR copy-text and editor text selection | G-15, FORGE-WP-29, FORGE-WP-31, AI02 |
| ME05-W16 | Colour pick, palettes, ruler | — |
| ME05-W17 | Pins | ME05-GAP-5, SH05 |
| ME05-W18 | Scrolling capture (`stitch-helper`) | — |
| ME05-W19 | History window, search, retention | `svc.search` |
| ME05-W20 | `pick` intent and bus interface | G-08 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| ME05-W21 | Redaction suggestions, barcode cards | ME04 decoders, G-15 |
| ME05-W22 | Replay buffer | standing-grant policy from SH04/AD05 |
| ME05-W23 | Live drawing, chapters from markers, presets | — |
| ME05-W24 | HDR capture output, hardware encode | ME03-GAP-1, ME03 hardware broker |
| ME05-W25 | Stamps, merge images, alt text draft, a11y text capture | `svc.a11y` |

## 13. Open questions and risks

1. **The screen portal is undefined** (ME05-GAP-1). This spec states what it must do. Until SH04 and the compositor design exist, nothing here can be built.
2. **Compositor-side redaction** needs a `secret` surface flag in FUSE widgets and the compositor protocol (ME05-GAP-4). Apps not built on FUSE (VM03 Compatibility, VMs) cannot mark fields; their windows need a per-app "protected" policy instead.
3. **Blur is not redaction.** Even a coarse mosaic leaks coarse structure (length of a line, layout). The UI steers users to solid redaction for text.
4. **Key display** is a keylogger by nature. It must stay a portal feature tied to a visible recording; if that cannot be made safe it should be cut.
5. **Replay buffer** needs a standing screen grant, which conflicts with "recording always asks". Off by default; may be cut.
6. **Scrolling capture** quality depends on content (animations, sticky elements); auto-scroll through a11y actions needs target app support.
7. **Global shortcuts** have no service in PLATFORM.md section 8 (ME05-GAP-3).
8. **GIF** is requested but poor for long clips; the app warns and suggests WebP or WebM.
9. **H.264/MP4** for wide compatibility needs a restricted-pack encoder that the base plan does not ship (ME03 section 6.5.3).
