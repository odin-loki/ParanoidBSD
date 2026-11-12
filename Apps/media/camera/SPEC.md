<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Camera

By Odin Loch

| | |
|---|---|
| ID | `ME04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.camera` |
| Category | media |
| Priority | P2 |
| Backing repos | FUSE (`videoWidget` stub; no capture code found). No camera app exists. |
| Status | Planned. Nothing built. |

## 1. Purpose

Camera takes photos and videos with the cameras attached to the machine, scans documents to PDF, and reads QR codes and barcodes safely. It also offers a virtual camera with background blur for calls in other apps. It replaces Kamoso and similar webcam tools.

## 2. Scope

In scope:

- Photo, video, burst, timer, HDR capture; resolution and format controls; manual controls where the device has them.
- Several cameras, switching and picture-in-picture of two cameras.
- Document scan with perspective fix, clean-up and OCR to PDF.
- QR and barcode scanning with a safe preview of what the code contains.
- Effects, grid and level overlays.
- Privacy: indicator, portals, no background capture, location tagging off by default.
- Virtual camera output with background blur or replacement through local models.

Out of scope:

| Item | Owner |
|---|---|
| Photo library, editing, metadata editing | ME01 Photos |
| Video editing | CR02 Video Editor |
| Flatbed and sheet-fed scanners | UT06 Printers and Scanners |
| Screen recording | ME05 Screen Capture |
| Audio-only recording | ME06 Sound Recorder |
| Video calls | CO05 Messenger (it may consume the virtual camera) |
| Camera device permissions, drivers, firmware | SH04 Portals, AD05 Permissions, VM05 Devices |
| Opening links from codes | IN01 Web Browser (after the preview here) |
| Encoders, decoders, frame transport | Media pipeline, ME03 section 6.5 |
| Camera on/off kill switch | SH02 Quick Settings, SY01 |

## 3. Users and top tasks

Personas:

- **Laptop user**: takes a quick photo or a short clip with the built-in camera.
- **Student or office worker**: scans paper to a searchable PDF with a webcam or document camera.
- **Caller**: wants a blurred background in a call without the call app seeing the room.
- **Careful user**: scans a QR code and wants to see where it leads before anything opens.
- **Maker**: uses an external UVC camera with manual focus and exposure.

| # | Task | Target steps |
|---|---|---|
| T1 | Take a photo | 1 (`Space`) after launch and grant |
| T2 | Record a video | 2 (switch mode `V`, `Space`) |
| T3 | Switch camera | 1 key (`C`) |
| T4 | Timer photo | 2 (`T` cycles 3/5/10 s, `Space`) |
| T5 | Scan a 3-page document to PDF with OCR | 5 (mode, capture ×3, Save) |
| T6 | Read a QR code and copy its text | 2 (show code, Copy) |
| T7 | Lock exposure and focus | 1 (click-hold on the preview) |
| T8 | Open the last capture | 1 (click the thumbnail) |
| T9 | Turn on background blur for a call | 2 (Virtual camera, Blur) |
| T10 | Stop all capture at once | 1 (`Esc` or close; indicator goes off) |
| T11 | Change resolution | 2 |
| T12 | Burst | 1 (hold `Space`) |

## 4. Features

### 4.1 Devices and preview

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F001 | Camera access by portal | The app asks `svc.portal.camera`; the shell prompt names the device. The app receives a frame stream handle, never the device node. | M | 1 |
| ME04-F002 | No access until needed | The camera opens when the window is shown and the user has not paused; it closes on minimise, on lock, when the window is hidden for 3 s, and on `Esc`. | M | 1 |
| ME04-F003 | Device list | Built-in, USB (UVC), document cameras, capture cards, network cameras handed over as streams (4.9). Names from the portal. | M | 1 |
| ME04-F004 | Switch camera | `C` cycles; `comboBox` in the toolbar; remembers last used. | M | 1 |
| ME04-F005 | Hot-plug | A camera that appears or disappears updates the list; a lost camera shows "Camera disconnected" and stops recording safely. | M | 1 |
| ME04-F006 | Live preview | Preview through the media pipeline surface (ME03 section 6.5) at the display rate, mirrored for front cameras (setting). | M | 1 |
| ME04-F007 | Preview fit | Fit or fill; shaded bars show what the chosen aspect will keep. | S | 1 |
| ME04-F008 | Format negotiation | Prefers uncompressed (YUYV, NV12) for low latency; MJPEG or H.264 from the device is decoded in the decode helper. | M | 1 |
| ME04-F009 | Device information | Inspector: name, bus, formats, resolutions, frame rates, controls, firmware string. | S | 2 |
| ME04-F010 | Two cameras at once | Second camera as an inset (corner, size) or side by side, when the bus can carry both. | C | 3 |
| ME04-F011 | Busy device | "The camera is in use by Messenger" with the owning app from the portal. | M | 1 |
| ME04-F012 | Hardware switch and shutter state | Shows "Camera is switched off" when the kill switch or lens cover is reported by the platform. | S | 2 |

### 4.2 Photo

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F013 | Capture | `Space`, `Enter`, shutter button, volume keys on tablets. Full-resolution still (device still pin where offered, else the best video frame). | M | 1 |
| ME04-F014 | Formats | JPEG (default), PNG, WebP, AVIF, JPEG XL; HEIF only with the restricted pack. Quality setting. | M | 1 |
| ME04-F015 | RAW | DNG when the device gives Bayer data. | C | 3 |
| ME04-F016 | Resolution and aspect | List from the device; aspect 4:3, 16:9, 1:1, 3:2 by crop. | M | 1 |
| ME04-F017 | Timer | Off, 3, 5, 10 s; countdown on screen with optional tick sound and screen flash. | M | 1 |
| ME04-F018 | Burst | Hold the shutter: up to 30 frames at the device rate; saved as a stack for ME01. | S | 2 |
| ME04-F019 | Interval capture | One frame every n seconds for m frames or until stopped; optional assembly into a time-lapse video. Window must stay visible. | C | 3 |
| ME04-F020 | HDR | Bracket 3 exposures when the device has exposure control, align, merge, tone map; otherwise single-frame local tone mapping. Marked "HDR" in metadata. | S | 2 |
| ME04-F021 | Low-light stack | Average 4–8 aligned frames to cut noise. | C | 3 |
| ME04-F022 | Screen flash | White full-screen flash at capture for front cameras; device torch where it exists. | S | 2 |
| ME04-F023 | Mirror | Preview mirrored, saved image not mirrored by default (setting). | M | 1 |
| ME04-F024 | Shutter sound | On by default at system volume rules; can be turned off. | S | 1 |
| ME04-F025 | Review strip | Thumbnail of the last capture; click opens the review view; `Delete` trashes it with Undo. | M | 1 |
| ME04-F026 | Capture feedback | 80 ms frame flash on the preview; none with reduced motion (a tick icon instead). | M | 1 |

### 4.3 Video

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F027 | Record | `Space` starts and stops; red timer and file size shown; pause and resume `P`. | M | 1 |
| ME04-F028 | Containers and codecs | WebM (VP9 or AV1 + Opus) and Matroska by default; MP4 (AV1 + Opus). H.264 output only with the restricted pack encoder if the owner ships one. | M | 1 |
| ME04-F029 | Resolution and frame rate | From the device; 24, 25, 30, 50, 60 fps where offered. | M | 1 |
| ME04-F030 | Quality | Presets (small, balanced, high) mapped to constant-quality values; bit-rate shown. | S | 1 |
| ME04-F031 | Audio | Microphone through `svc.portal.mic` with its own prompt and indicator; device choice; level meter; mute; "no audio" mode needs no mic grant at all. | M | 1 |
| ME04-F032 | Audio monitoring | Optional headphone monitoring. | C | 2 |
| ME04-F033 | Photo during video | `S` saves a still from the stream. | S | 2 |
| ME04-F034 | Crash-safe recording | Matroska with clusters flushed every 2 s; a recording survives a crash or power cut up to the last cluster. MP4 is written fragmented. | M | 1 |
| ME04-F035 | Disk space guard | Stops cleanly when free space falls under 500 MB; shows remaining time. | M | 1 |
| ME04-F036 | Maximum length | Optional limit by time or size. | C | 2 |
| ME04-F037 | Slow motion and time-lapse | High-frame-rate capture played slow; time-lapse at 1 frame per n seconds. | C | 3 |
| ME04-F038 | Stabilisation | Software stabilisation as a post step (not live). | C | 3 |
| ME04-F039 | Hardware encode | Through the pipeline's broker when available; software otherwise. | C | 3 |
| ME04-F040 | Recording continues only while visible | Recording pauses with a notice if the window is hidden for 10 s, unless "allow recording while minimised" is set (still no `bg.run`: closing the window stops it). | M | 1 |

### 4.4 Manual controls

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F041 | Control discovery | Only controls the device reports are shown. Others are absent, not greyed. | M | 2 |
| ME04-F042 | Exposure | Auto, compensation ±3 EV, manual exposure time and gain/ISO, anti-flicker 50/60 Hz. | M | 2 |
| ME04-F043 | Focus | Auto, continuous, manual slider, click-to-focus where region control exists, focus peaking overlay. | M | 2 |
| ME04-F044 | White balance | Auto, presets (daylight, cloudy, tungsten, fluorescent), Kelvin slider, click a grey point. | M | 2 |
| ME04-F045 | Lock | Click-hold locks exposure and focus (AE/AF lock badge). | S | 2 |
| ME04-F046 | Zoom, pan, tilt | Optical or digital zoom slider; pan/tilt for PTZ cameras; `+`/`-` keys. | S | 2 |
| ME04-F047 | Image controls | Brightness, contrast, saturation, sharpness, gamma, backlight compensation. | S | 2 |
| ME04-F048 | Histogram and zebra | Live histogram; clipping stripes. | C | 2 |
| ME04-F049 | Profiles per camera | Save and recall control sets per device id; "reset to device defaults". | S | 2 |
| ME04-F050 | Controls stay local | Controls are set through the portal stream handle; they reset when the app releases the camera unless "keep device settings" is on. | S | 2 |

### 4.5 Document scan

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F051 | Document mode | `D`. Detects the page outline live (edge detection, own code) and draws it; captures when steady (auto) or on `Space`. | M | 2 |
| ME04-F052 | Perspective fix | Four-corner warp to a flat rectangle; corners adjustable by drag with a magnifier. | M | 2 |
| ME04-F053 | Clean-up | Modes: colour, grey, black and white (adaptive threshold), whiteboard. Shadow removal, de-skew, auto-crop, contrast. | M | 2 |
| ME04-F054 | Multi-page | Page strip; reorder, rotate, retake, delete; batch capture. | M | 2 |
| ME04-F055 | Paper size | Detect A4/Letter/A5 by ratio; set output size and DPI (150, 200, 300). | S | 2 |
| ME04-F056 | OCR | Text recognition through `svc.model` (`ai.local`); language choice; produces an invisible text layer. | M | 2 |
| ME04-F057 | Output | PDF (PDF/A-2b where possible) with text layer, or images (PNG, JPEG), or plain text / Markdown. Compression: JBIG2-free (uses CCITT G4 or Flate for bitonal), JPEG or JPEG XL-in-PDF off by default. | M | 2 |
| ME04-F058 | Review text | Side panel with recognised text; copy; low-confidence words underlined. | S | 2 |
| ME04-F059 | Scan intent | Handles intent `scan` from other apps (OF02 Notes, OF03 Document Viewer, CO01 Mail): returns a PDF handle. | S | 2 |
| ME04-F060 | Import an image as a page | Add an existing image and run the same fix and OCR. | S | 2 |
| ME04-F061 | Book mode | Split a two-page spread; curved-page flattening. | C | 3 |
| ME04-F062 | No identity-document extraction | Machine-readable zones and identity cards get no special handling; nothing is extracted from them. | M | 2 |
| ME04-F063 | Redaction hand-off | "Redact in Document Viewer" sends the PDF to OF03. | C | 3 |

### 4.6 QR codes and barcodes

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F064 | Scan mode | `Q`. Also passive detection in photo mode (setting, on). Decoder runs in the frame-analysis helper. | M | 2 |
| ME04-F065 | Symbologies | QR, Micro QR, Data Matrix, Aztec, PDF417, EAN-13/8, UPC-A/E, Code 128, Code 39, Code 93, ITF, Codabar. Own C++23 decoders. | M | 2 |
| ME04-F066 | Nothing opens by itself | A decoded code only shows a card. No action runs without a click. | M | 2 |
| ME04-F067 | Safe link preview | For URLs: scheme, registrable domain in bold, full URL with punycode shown decoded and raw, mixed-script warning, redirect-shortener warning (local list), `http` warning. No request is made to preview. | M | 2 |
| ME04-F068 | Typed payloads | Wi-Fi (`WIFI:`), contact (vCard, MeCard), calendar event, e-mail, SMS, geo, phone, OTP (`otpauth://`), plain text. Each shows parsed fields. | M | 2 |
| ME04-F069 | Actions | Copy; Open in Browser (IN01, optionally in a disposable container); Add network (SY01, shows SSID and security, password hidden until held); Add contact (CO03); Add event (CO02); Add to Passwords (SE04, for `otpauth`, with a warning that the secret is on screen); Show on map (IN04). | M | 2 |
| ME04-F070 | Sensitive payloads | OTP secrets and Wi-Fi passwords are masked, never written to history, and cleared from the clipboard by the sensitivity flag. | M | 2 |
| ME04-F071 | History | Optional local history of scanned codes (off by default); clear all. | C | 2 |
| ME04-F072 | Scan from image | Decode codes in a file or a pasted image. | S | 2 |
| ME04-F073 | Multiple codes | Outlines every code in view; click one to pick. | S | 2 |
| ME04-F074 | Generate a code | Make a QR code from text for sharing (own encoder); save as PNG or SVG. | C | 3 |

### 4.7 Overlays and effects

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F075 | Grid | Thirds, 4×4, golden ratio, centre cross. `G` cycles. | M | 1 |
| ME04-F076 | Level | Horizon line from a device accelerometer when the platform exposes one; otherwise from detected lines in the image. | C | 2 |
| ME04-F077 | Aspect and safe-area guides | Crop marks for 16:9, 1:1, 9:16. | C | 2 |
| ME04-F078 | Colour filters | Mono, sepia, warm, cool, high contrast, negative. Applied to output and preview; original not kept unless "also save unfiltered". | C | 2 |
| ME04-F079 | Face-aware exposure | Optional face detection to guide exposure and focus region. Local model; no identity. | C | 3 |
| ME04-F080 | Flip and rotate | 90° steps for cameras mounted sideways; saved per device. | S | 1 |

### 4.8 Virtual camera

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F081 | Virtual camera output | Publishes a processed stream as a camera source in `svc.portal.camera` (ME04-GAP-2). Other apps pick it in the normal camera prompt as "Camera (processed)". | S | 3 |
| ME04-F082 | Background blur | Person segmentation by a local model through `svc.model`; blur strength 1–10; edge feathering. | S | 3 |
| ME04-F083 | Background replace | Image or solid colour behind the person. | C | 3 |
| ME04-F084 | Framing | Auto-centre crop that follows the person; fixed crop; mirror. | C | 3 |
| ME04-F085 | Light correction | Auto exposure lift and white balance on the output stream. | C | 3 |
| ME04-F086 | Still or paused output | Send a chosen still image or "camera off" card instead of live video. | S | 3 |
| ME04-F087 | Fail closed | If segmentation falls behind or the model stops, the output switches to full blur or the "camera off" card, never to the raw frame. | M | 3 |
| ME04-F088 | Consumer list | Shows which apps are reading the virtual camera; one click cuts a consumer off. | M | 3 |
| ME04-F089 | Runs only with a visible control | The virtual camera runs while a small control window or a shell indicator entry is present; closing it stops the stream. Needs `bg.run` while a consumer is attached. | M | 3 |
| ME04-F090 | Performance guard | Target 30 fps at 720p; drops resolution before dropping frames; shows model and frame time. | S | 3 |

### 4.9 Storage, privacy, integration

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME04-F091 | Save location | Default `~/Pictures/Camera` and `~/Videos/Camera`, granted once through the portal; scans to `~/Documents/Scans`. | M | 1 |
| ME04-F092 | File names | `{type}_{YYYYMMDD}_{HHMMSS}_{seq}`; template setting. | M | 1 |
| ME04-F093 | Metadata written | Date, orientation, camera model as reported, exposure values where known. Software field says "Camera". No host name, no user name. | M | 1 |
| ME04-F094 | Location tagging | Off by default. When on, asks `dev.location` with its own prompt; precision choice (exact, town); a badge on the shutter shows that location will be embedded. | M | 2 |
| ME04-F095 | Minimal-metadata mode | Writes only orientation and colour profile. | S | 1 |
| ME04-F096 | Indicators | The shell shows camera, microphone and location indicators (SH02). The app also shows a red dot beside the active device name. | M | 1 |
| ME04-F097 | No background capture | The manifest does not request `bg.autostart`; `bg.run` is used only for the virtual camera with a consumer attached. No capture starts without a visible window or indicator. | M | 1 |
| ME04-F098 | Audit | Every start and stop of camera, microphone and location use is written to `svc.audit` by the portal. | M | 1 |
| ME04-F099 | Network cameras | An RTSP or HTTP camera URL can be opened as a source through the pipeline's net-fetch helper (`net.user`, per-host prompt). Off until the user adds one. | C | 3 |
| ME04-F100 | Share and open | Share sheet for the last capture (metadata stripped by default); "Open in Photos" (ME01), "Open in Video" (ME03), "Edit" (CR01, CR02). | M | 1 |
| ME04-F101 | Capture intent | Handles intent `pick` for image or video capture from other apps (for example a contact photo for CO03): returns one handle after the user confirms the shot. | S | 2 |
| ME04-F102 | Private capture | A session toggle: saves only where the user chooses each time, no review thumbnails kept, no history. | S | 2 |
| ME04-F103 | Encrypted destination | Save straight into a Vault container (SE05) or a hidden album (ME01) chosen as the destination. | C | 3 |
| ME04-F104 | Command line | `camera [--mode=photo\|video\|scan\|qr] [--device=<portal name>]`. No headless capture option exists. | S | 1 |
| ME04-F105 | Recovery | Unfinished recordings are finalised on next start; unsaved scan sessions are restored. | M | 1 |
| ME04-F106 | Settings export | Per-camera profiles export as JSON. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Camera
├─ Main window (single instance)
│  ├─ Header bar
│  ├─ Mode bar: Photo · Video · Document · Codes
│  ├─ Preview surface with overlays (grid, level, page outline, code outlines)
│  ├─ Controls drawer (right, F4): Exposure · Focus · White balance · Image · Device
│  ├─ Capture bar: review thumbnail · shutter · camera switch · timer · flash
│  └─ Inline bars: no camera, permission denied, camera busy, low disk, disconnected
├─ Review view (last captures)
├─ Document session view (pages, corners, clean-up, OCR text, save)
├─ Code result card (popover over the preview)
├─ Virtual camera control window (small)
├─ Dialogs: Save scan · Resolution and format · Add network camera · Corner adjust
├─ Preferences: General · Photo · Video · Audio · Document · Codes · Virtual camera ·
│               Privacy · Shortcuts
└─ Shell-drawn: camera, microphone and location prompts; file portal; share sheet
```

### 5.2 Main window

Shown at 960×640, medium breakpoint, photo mode, controls drawer open.

```
+------------------------------------------------------------------------------------------+
| [=] Camera                    [● Integrated camera v]        [cam][mic]  [▁] [▢] [✕]    |(1)
+------------------------------------------------------------------------------------------+
|   [ Photo ]   Video    Document    Codes                         1920×1080 · JPEG  [⚙] |(2)
+----------------------------------------------------------------------+-------------------+
|        ┆                    ┆                    ┆                   | Exposure      (5) |
|        ┆                    ┆                    ┆                   |  (•) Auto ( ) Man |
|  ┄┄┄┄┄┄┼┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┼┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┼┄┄┄┄┄┄┄┄┄┄┄┄       |  Comp ──●── +0.3  |
|        ┆                    ┆                    ┆                   | Focus             |
|        ┆        LIVE PREVIEW (3)                 ┆                   |  (•) Auto ( ) Man |
|        ┆                    ┆        [AE/AF lock]┆                   | White balance     |
|  ┄┄┄┄┄┄┼┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┼┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┼┄┄┄┄┄┄┄┄┄┄┄┄       |  [Auto v] 5200 K  |
|        ┆                    ┆                    ┆                   | Zoom ●──── 1.0×   |
|        ┆                    ┆                    ┆      ▁▃▆█▅▂ (4)   | [Reset] [Save…]   |
+----------------------------------------------------------------------+-------------------+
|  +-----+                        ╭─────╮                                                  |
|  |last |  [Timer off v] [Flash] │  ◉  │ [Switch C] [Grid G] [Mirror]     location: off  |(6)
|  +-----+                        ╰─────╯                                                  |
+------------------------------------------------------------------------------------------+
```

1. Header: device `comboBox` with a red dot while the camera is live; `[cam]` and `[mic]` badges mirror the shell indicators. No `[net]` badge unless a network camera is open.
2. Mode bar (`tabBar`), current resolution and format (click opens the format dialog), settings.
3. Preview (`videoWidget` after G-11) with overlays: grid lines, lock badge, level line, page or code outlines.
4. Histogram (`chart`), optional.
5. Controls drawer, 240 px (`propertyGrid` with `slider`, `radioButton`, `comboBox`); only controls the device reports.
6. Capture bar: review thumbnail (`image`), timer `comboBox`, flash `toolButton`, shutter (primary button), switch, grid, mirror; the location state is always written in words.

States: empty (no camera: "No camera found. Connect one or check Devices."), permission needed ("Camera access is off for this app. [Ask again] [Open Permissions]"), busy, disconnected, loading (spinner under 300 ms typical), error (stream failed, with Retry), recording (shutter becomes a stop square; timer and size shown; mode bar disabled), paused by visibility, low disk.

### 5.3 Other views and dialogs

**Video mode, recording**

```
+------------------------------------------------------------------------------------------+
| [=] Camera                    [● USB camera v]               [cam][mic]  [▁] [▢] [✕]    |
+------------------------------------------------------------------------------------------+
|   Photo   [ Video ]   Document   Codes                 1280×720 · 30 fps · WebM VP9+Opus |
+------------------------------------------------------------------------------------------+
|  ● REC 00:01:23   48 MB   about 2 h 10 min left                                          |
|                                                                                          |
|                                LIVE PREVIEW                                              |
|                                                                                          |
|  mic [▮▮▮▮▮▮▯▯▯▯] −12 dB  [Mute]                                                         |
+------------------------------------------------------------------------------------------+
|  +-----+                 [Pause P]   ╭─────╮   [Still S]                                 |
|  |last |                             │  ■  │                                             |
|  +-----+                             ╰─────╯                                             |
+------------------------------------------------------------------------------------------+
```

**Document session** (1100×700)

```
+------------------------------------------------------------------------------------------+
| [←] Scan — 3 pages                                  [Add page] [Import image…] [Save…]   |
+-----------+-----------------------------------------------------+------------------------+
| +-------+ |                                                     | Clean-up               |
| | 1     | |        ●━━━━━━━━━━━━━━━━━━━━━━━━━●                   |  (•) Black and white   |
| +-------+ |        ┃                         ┃                  |  ( ) Grey ( ) Colour   |
| +-------+ |        ┃     page 2 (flattened   ┃                  |  ( ) Whiteboard        |
| |>2     | |        ┃     preview, corners    ┃                  |  [x] Remove shadows    |
| +-------+ |        ┃     adjustable)         ┃                  |  [x] De-skew           |
| +-------+ |        ┃                         ┃                  | Size [A4 v] DPI [300 v]|
| | 3     | |        ●━━━━━━━━━━━━━━━━━━━━━━━━━●                   | OCR  [x] English v     |
| +-------+ |   [Rotate] [Retake] [Corners…] [Delete]             | Text (page 2)          |
| drag to   |                                                     |  Invoice 1042 …        |
| reorder   |                                                     |  [Copy text]           |
+-----------+-----------------------------------------------------+------------------------+
| OCR on this device · model "ocr-latin" verified · 2 of 3 pages recognised                |
+------------------------------------------------------------------------------------------+
```

Page strip `listView`; page canvas `image` with corner handles (G-12 improves it); right panel `propertyGrid` plus `markdownView` for text. States: no OCR model (checkbox disabled, link to AI02), OCR running (per-page progress), low-confidence warning.

**Save scan** (dialog, 560×340)

```
+--------------------------------------------------------------+
| Save scan                                                    |
| Format (•) PDF with text  ( ) PDF images only  ( ) PNG files |
|        ( ) Text (.txt)    ( ) Markdown (.md)                 |
| [x] PDF/A-2b      Compression [balanced v]                   |
| Title [Invoice 1042________]  (no author is written)         |
| Save to [Documents/Scans/2026-11-12 Invoice 1042.pdf] […]    |
| 3 pages · about 410 kB            [Cancel]  [Save PDF]       |
+--------------------------------------------------------------+
```

**Code result card** (popover, 420×300)

```
+--------------------------------------------------+
| QR code · web link                               |
| https://  example-bank.com  /login?ref=qr        |
|           ^ registrable domain                   |
| ⚠ Uses letters from two scripts: "examp1e" check |
| Full: https://example-bank.com/login?ref=qr      |
| Nothing has been opened or fetched.              |
| [Copy]  [Open in Browser ▾]  [Dismiss]           |
|          └ Open in a disposable container        |
+--------------------------------------------------+
```

Variants: Wi-Fi (SSID, security, password masked, `[Add network…]`), contact, event, OTP (secret masked, `[Add to Passwords…]`), text (`[Copy]`).

**Resolution and format** (dialog, 480×360)

```
+----------------------------------------------+
| Integrated camera                            |
| Photo  [1920×1080 v]  Format [JPEG v] Q [90] |
| Video  [1280×720 v]   Rate [30 fps v]        |
|        Container [WebM v] Codec [VP9 v]      |
|        Quality [balanced v]  Audio [Opus v]  |
| Device stream  [MJPEG v] (decoded in helper) |
|                         [Cancel]  [Apply]    |
+----------------------------------------------+
```

**Virtual camera control** (window, 360×300)

```
+----------------------------------------------+
| Virtual camera            [● live]   [▁] [✕] |
| +------------------------------------------+ |
| |        processed preview                 | |
| +------------------------------------------+ |
| Source [Integrated camera v]                 |
| Background (•) Blur ──●── 6  ( ) Image ( ) Off|
| [x] Auto-frame   [ ] Send still image        |
| Used by: Messenger            [Cut off]      |
| Model "segment-person" · 21 ms/frame         |
| [Stop virtual camera]                        |
+----------------------------------------------+
```

**Review view**: large `image` or `videoWidget` of the capture, filmstrip of this session, buttons `[Open in Photos] [Share] [Edit] [Delete]`, and a metadata line stating exactly what was embedded ("date, orientation; no location").

### 5.4 Menus

```
File     Open captures folder · Open last capture Ctrl+O · Import image as page… ·
         Scan code from image… · Add network camera… · Share last… · Close Ctrl+W · Quit Ctrl+Q
Camera   Device ▸ · Switch camera C · Resolution and format… Ctrl+R · Mirror preview M ·
         Rotate ▸ · Pause camera Esc · Device information
Capture  Take photo / Start-stop Space · Pause recording P · Still during video S ·
         Timer ▸ T · Burst (hold Space) · HDR H · Flash F · Interval capture… · Private capture
Mode     Photo 1 · Video 2 · Document D · Codes Q
View     Controls F4 · Grid ▸ G · Level L · Histogram Ctrl+H · Zebra Z · Focus peaking K ·
         Fit/Fill · Full screen F11
Tools    Virtual camera… · Per-camera profiles ▸ · Location tagging ▸ (Off / Town / Exact) ·
         Preferences Ctrl+,
Help     Help F1 · Shortcuts · About
```

### 5.5 Keyboard and pointer

Keys are in 5.4. `+`/`-` zoom, arrow keys pan/tilt on PTZ cameras, `Delete` trashes the last capture, `Ctrl+C` copies the last capture or the decoded code text.

| Context | Menu items |
|---|---|
| Preview | Take photo · Lock exposure and focus · Focus here · Set white point here · Grid ▸ · Mirror · Copy frame |
| Review thumbnail | Open · Open in Photos · Share… · Show in Files · Move to trash |
| Scan page | Retake · Rotate left / right · Adjust corners… · Duplicate · Delete |
| Code outline | Show details · Copy raw data |

Pointer: click = focus point (if supported), click-hold = AE/AF lock, wheel = zoom, drag on the exposure sun icon = compensation. Drag and drop: an image file onto the window in document mode adds a page; in codes mode decodes it. Drag out: the review thumbnail as a file handle. Touch: pinch zoom, swipe between modes.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No camera: message, `[Open Devices]` (VM05). Document and Codes modes still work with "Import image". |
| Loading | Preview placeholder with the device name; cancel with `Esc`. |
| Error | Stream error with cause; Retry; "Use another camera". |
| Offline | Not relevant (no network), except network cameras: "Cannot reach camera". |
| Locked | When the session locks, capture stops and the camera is released; on unlock the preview resumes only if the window is visible. |
| Read-only | Save folder not writable: bar with `[Choose another folder]`; captures held in memory up to 64 MB. |
| Narrow (<640) | Controls drawer becomes a bottom sheet; capture bar keeps shutter, switch, review; mode bar scrolls. |
| Permission denied | Bar with `[Ask again]` and `[Open Permissions]` (AD05). No preview is faked. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Preview, review video | `videoWidget` (G-11) |
| Mode bar | `tabBar` |
| Device and option pickers | `comboBox`, `toolButton`, `toggleSwitch` |
| Controls drawer | `propertyGrid`, `slider`, `radioButton`, `spinBox` |
| Histogram, level meter | `chart`, `progressBar` |
| Page strip, session filmstrip | `listView`, `image` |
| Text panel | `markdownView`, `codeEdit` (plain text) |
| Dialogs, cards | `dialog`, `label`, `link` (never auto-followed), `checkBox` |

Missing: G-11 (camera frames into the surface). `ME04-GAP-1`: camera control interface in `svc.portal.camera` (enumerate formats and controls, set controls, still capture) — the portal is listed in PLATFORM.md section 8 with no interface defined. `ME04-GAP-2`: the camera portal accepting an app-provided virtual source. `ME04-GAP-3`: an orientation sensor service for the level. G-12 for corner editing.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Modes, capture state machine, scan session, code cards, settings |
| UI | FUSE UI | app host | Section 5 |
| Media session client | C++23 (platform, ME03 section 6.5) | app host | Preview clock, frame pool from the portal, record graph |
| Camera portal | C++23 | system service (`svc.portal.camera`) | Owns the device; prompts; indicator; hands out frame streams |
| `decode-helper` | C++23 plus vendored codecs | helper, no capabilities | MJPEG and H.264 from devices |
| `encode-helper` | C++23 plus vendored encoders | helper, no capabilities | Still and video encode, mux; writes to a passed descriptor |
| `frame-analysis-helper` | C++23 | helper, no capabilities | Page-edge detection, perspective warp, clean-up, barcode decode, HDR merge |
| `pdf-writer` | C++23 | helper, one writable descriptor | Builds the PDF with image and text layers |
| Segmentation and OCR | — | `svc.model` | Local models; frames passed by shared memory |
| Virtual camera publisher | C++23 binding | app host | Sends processed frames to the camera portal as a source |

```
 device ─► svc.portal.camera ──frames (shm)──► app host (ForgeScript + FUSE UI + session client)
              ▲ virtual source                      │            │             │
              └─────────────────────────────────────┘      frames│       frames│
                                                    ┌───────────▼───┐   ┌─────▼────────┐
 svc.portal.mic ──PCM ring──► app host ──► encode-  │ frame-analysis│   │ svc.model    │
                                           helper ─►│ helper        │   │ (OCR, segment)│
                                           file fd  └───────┬───────┘   └──────────────┘
                                                            ▼ pages
                                                      pdf-writer ─► file fd
```

### 6.2 Data model

Store: `~/.local/share/org.pbsd.camera/state.sqlite`; scan sessions in `~/.local/share/org.pbsd.camera/scans/<session>/` (page images until saved, then removed); no media library (captures are plain files).

| Entity | Fields |
|---|---|
| `device_profile` | `device_key TEXT PK` (portal-stable id), `label TEXT`, `rotation INTEGER`, `mirror INTEGER`, `photo_w`, `photo_h`, `video_w`, `video_h`, `fps_num`, `fps_den`, `stream_format TEXT`, `controls_json TEXT` |
| `capture` (session review only; pruned after 30 days or never kept in private capture) | `id`, `kind TEXT (photo\|video\|scan\|burst)`, `grant_ref TEXT`, `file_name TEXT`, `created_at`, `width`, `height`, `duration_ms`, `has_location INTEGER` |
| `scan_session` | `id`, `created_at`, `paper TEXT`, `dpi INTEGER`, `mode TEXT`, `ocr_lang TEXT`, `state TEXT` |
| `scan_page` | `session_id`, `pos`, `raw_ref TEXT`, `corners_json TEXT` (4 points, normalised), `rotation`, `text TEXT`, `confidence REAL` |
| `code_history` (off by default) | `id`, `symbology TEXT`, `kind TEXT`, `text TEXT` (never for sensitive kinds), `at` |
| `virtual_cam` | `source_device_key`, `background TEXT (blur\|image\|off)`, `blur INTEGER`, `image_ref`, `auto_frame INTEGER` |

Recording journal: `<file>.part.json` beside an unfinished recording with container, tracks and last flushed cluster offset; removed on clean stop.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| UVC payloads (YUYV, NV12, MJPEG, H.264) | yes | — | portal delivers frames; MJPEG/H.264 by pipeline codecs | camera portal (framing), decode helper |
| JPEG, PNG, WebP, AVIF, JPEG XL | import for pages | yes | image codec helper (ME01 section 6.3) | image codec helper / encode helper |
| DNG | — | yes | own writer | encode helper |
| WebM, Matroska, fragmented MP4 | recover own files | yes | pipeline muxers (ME03 section 6.5.2) | encode helper |
| VP9, AV1, Opus, FLAC | — | yes | libvpx, SVT-AV1, libopus, libFLAC (ME03 section 6.5.3) | encode helper |
| PDF 1.7 / PDF/A-2b (write only) | no | yes | own writer: images, invisible text, XMP, output intent | pdf-writer |
| QR, Data Matrix, Aztec, PDF417, 1D barcodes | yes | QR | own C++23 (ZXing-C++, Apache-2.0, as differential reference) | frame-analysis helper |
| URL, vCard, MeCard, iCalendar, `WIFI:`, `otpauth:` payloads | yes | no | own parsers, strict length limits | frame-analysis helper returns typed records |
| EXIF, XMP | — | yes | metadata writer shared with ME01 | encode helper |
| RTSP, HTTP MJPEG (network cameras) | yes | — | ME03 net-fetch helper | net-fetch helper |

### 6.4 Integration

- Intents handled: `scan` (returns PDF), `pick` (capture a photo or video for another app). Emits: `open` (ME01, ME03, IN01), `share`, `edit` (CR01, CR02, OF03), `compose` (CO01 attach scan).
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.camera`, `svc.portal.mic`, `svc.portal.location`, `svc.intent`, `svc.clipboard`, `svc.media`, `svc.power` (inhibit sleep while recording), `svc.model`, `svc.undo-trash`, `svc.audit`.
- Offers: a virtual camera source to `svc.portal.camera` (ME04-GAP-2).
- Other apps: ME01 (captures appear by folder watch; stacks), ME03 (pipeline), OF03 (PDF), UT06 (shares the clean-up and OCR path for flatbeds: Proposed common helper), IN01, SY01 (Wi-Fi add), CO02, CO03, SE04 (OTP), SE05, AI02 (models), CO05 (virtual camera consumer), AD05, VM05.
- Backing repos, Verified: FUSE has no camera capture code under the paths read (`FUSE:Source/FUSE/UI/Video/` holds only the Theora source; no capture call was found in `FUSE:Source/FUSE/Audio/src/`). A repo named `CameraDetector` is on the profile but is listed as not planned and was not read (CATALOGUE.md). Everything here is Proposed.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Profiles, scan sessions |
| `fs.portal` | Save folders, import image |
| `dev.camera` | The purpose of the app; prompt on first use; indicator while live |
| `dev.mic` | Video with sound; separate prompt; not asked in photo, document or code modes |
| `dev.location` | Only if the user turns on location tagging; separate prompt |
| `dev.gpu.compute` | Preview processing, blur |
| `ai.local`, `svc.model` | OCR, segmentation |
| `proc.helper:decode-helper`, `encode-helper`, `frame-analysis-helper`, `pdf-writer` | Section 6.1 |
| `bg.run` | Virtual camera while a consumer is attached and an indicator is shown |
| `net.user` | Only for network cameras the user adds; never used otherwise |
| `svc.*` listed in 6.4 | As described |

Not requested: `dev.screen`, `proc.exec`, `bg.autostart`, `net.host:*`, `fs.path:*`.

| Asset | Adversary | Attack surface | Mitigation |
|---|---|---|---|
| The user's image and voice | Any app, including this one if compromised | Camera and mic streams | Portal owns devices; shell indicator cannot be hidden by the app; capture tied to a visible window; audit log; one-click revoke (AD05) |
| The room behind the user | Consumer of the virtual camera | Raw frames leaking past blur | Fail closed to full blur or a card; consumers never get the raw device through this app |
| Location | Recipients of photos | EXIF GPS | Off by default; badge when on; share strips by default |
| The session | Hostile QR or barcode | Payload parsers; auto-actions | Parsers in a zero-capability helper; nothing opens without a click; typed cards; masked secrets |
| The user's judgement | Phishing code | Look-alike domains, shorteners | Domain emphasis, mixed-script and punycode display, shortener warning, disposable container option |
| The session | Hostile USB camera | Malformed UVC and MJPEG streams | Portal frames the stream; decode in the helper; device firmware never reaches the app host |
| Scanned documents | Other local users | Scan session files, OCR text | Session files in app data, removed on save; private capture; Vault destination |
| Model input | — | Frames sent to `svc.model` | Local only; shared memory; the model service has no network |

Stored: per-camera profiles, unfinished scan sessions, optional code history. Leaves the machine: nothing, unless the user adds a network camera (requests to that host) or shares a file.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `general.start_mode` | enum `last\|photo\|video\|document\|codes` | `last` | Mode at start |
| `general.mirror_preview` | bool | true | Mirror front cameras |
| `general.save_mirrored` | bool | false | Save as previewed |
| `general.shutter_sound` | bool | true | Capture sound |
| `general.release_when_hidden_s` | int | 3 | Release the camera after the window is hidden |
| `photo.format` | enum `jpeg\|png\|webp\|avif\|jxl` | `jpeg` | Still format |
| `photo.quality` | int | 90 | Lossy quality |
| `photo.folder` | grant | `~/Pictures/Camera` | Photo folder |
| `photo.detect_codes` | bool | true | Passive code detection |
| `video.container` | enum `webm\|mkv\|mp4` | `webm` | Container |
| `video.codec` | enum `vp9\|av1` | `vp9` | Video codec |
| `video.quality` | enum `small\|balanced\|high` | `balanced` | Quality preset |
| `video.folder` | grant | `~/Videos/Camera` | Video folder |
| `video.record_when_minimised` | bool | false | Keep recording while minimised |
| `audio.record` | bool | true | Record sound with video |
| `audio.device` | string | `default` | Microphone |
| `doc.mode` | enum `bw\|grey\|colour\|whiteboard` | `bw` | Clean-up mode |
| `doc.dpi` | int | 300 | Output resolution |
| `doc.auto_capture` | bool | true | Capture when steady |
| `doc.ocr` | bool | true | Recognise text when a model is installed |
| `doc.ocr_languages` | list | system language | OCR languages |
| `doc.pdfa` | bool | true | Write PDF/A-2b |
| `doc.folder` | grant | `~/Documents/Scans` | Scan folder |
| `codes.history` | bool | false | Keep scanned codes |
| `codes.open_links_disposable` | bool | true | Default "open" uses a disposable browser container |
| `privacy.location` | enum `off\|town\|exact` | `off` | Location tagging |
| `privacy.minimal_metadata` | bool | false | Only orientation and profile |
| `privacy.review_history_days` | int | 30 | Keep the review list |
| `virtual.background` | enum `blur\|image\|off` | `blur` | Virtual camera background |
| `virtual.fail_mode` | enum `full_blur\|card` | `full_blur` | Output when processing fails |
| `names.template` | string | `{type}_{YYYYMMDD}_{HHMMSS}_{seq}` | File names |

## 9. Accessibility and languages

- The shutter, mode and device controls are fully keyboard driven; `Space` always captures in the current mode.
- Timer countdown is spoken ("three, two, one") and shown; capture success is announced with the file name.
- Document mode speaks guidance ("move closer", "page found", "hold still") from the edge detector, for blind and low-vision users.
- Code cards are plain text in the a11y tree; warnings are announced before actions.
- No information is given by colour alone: recording shows "REC" and a square, location shows words.
- Screen flash and capture flash are suppressed with reduced motion and photosensitivity settings.
- OCR languages are independent of the UI language; right-to-left text is kept in logical order in the PDF text layer (G-03).

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Launch to live preview (permission already granted) | 500 ms |
| Preview latency, glass to glass | under 80 ms at 30 fps for uncompressed streams |
| Shutter to file written (1080p JPEG) | under 150 ms; shot-to-shot 250 ms |
| Recording | 1080p30 VP9 software on 4 cores without dropped frames; drops counted and shown |
| Page detection | 15 Hz on CPU |
| Code detection | under 100 ms from in-focus frame to card |
| OCR | A4 at 300 dpi under 2 s on CPU (budget from FORGE-WP-31) |
| Virtual camera | 720p30 with segmentation on GPU; fallback 480p |
| Memory | app host under 150 MB; frame pool 6 frames |
| Camera released after hide | within 3 s, verified by the portal indicator |
| Idle (paused camera) | 0 frames, 0 wake-ups |

## 11. Testing and proof

Nothing has run.

- Unit: capture state machine, file naming, recording journal recovery, URL analysis (domain extraction, mixed-script detection), payload parsers.
- Golden: `.fuseui` renders for each mode and state; image goldens for perspective warp and clean-up on a page corpus.
- Scripted UI with a synthetic camera source from the portal test harness: photo, video, scan to PDF, code card, permission denied, hot-unplug during recording.
- Conformance: barcode decoders against ISO test symbols and a damaged-symbol corpus; PDF output through a PDF/A validator (veraPDF as an external reference).
- Differential: barcode results against ZXing-C++; OCR text layer positions against rendered text.
- Fuzz: barcode bitstream decoders, payload parsers, MJPEG path, recovery of truncated recordings.
- Privacy tests: after hide or lock the portal reports the stream closed within the budget; files written with location off contain no GPS bytes; virtual camera fail-closed test by killing the model service mid-stream.
- Power-cut test: kill during recording, then verify the file plays to the last cluster.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Barcode decoders (QR, Data Matrix, 1D) | No runtime errors; bounded output | T2 | PRISM; fuzz | Reed–Solomon proof of correctness would be T1 work |
| Payload parsers (URL, vCard, Wi-Fi, otpauth) | No runtime errors; typed output only | T2 | PRISM; fuzz | — |
| URL safety analysis | Registrable-domain extraction matches the public-suffix data | T2 | PRISM; table tests | Depends on a data file |
| Capture visibility rule | No frame is requested while the window is hidden beyond the grace time | T1 | Small state machine; model check | Portal must enforce the same rule independently |
| Virtual camera fail-closed switch | Raw frame never reaches the output when the mask is missing or stale | T1 | Proof over the compositor function (mask age check) | — |
| Perspective warp and clean-up | No out-of-bounds; bounded memory | T2 | PRISM; goldens | — |
| PDF writer | Well-formed output; no runtime errors | T2 | PRISM; validator | PDF/A semantics are checked by tests only |
| Recording journal and recovery | Recovered file is valid up to the last flushed cluster | T3 | Kill tests | File-system behaviour |
| Vendored encoders | Contained | T4 | Helper sandbox | Third-party C |

## 12. Build plan

### Phase 1: photo and video

| ID | Work item | Depends on |
|---|---|---|
| ME04-W01 | Camera portal interface: enumerate, open stream, formats (define with SH04) | G-09, ME04-GAP-1, SH04 |
| ME04-W02 | Preview through the media pipeline surface | G-11, ME03 phase 1, FUSE-R-089 |
| ME04-W03 | Photo capture, formats, timer, review, trash | ME01 image codec helper |
| ME04-W04 | Video record graph with crash-safe Matroska/WebM, mic through `svc.portal.mic` | ME03 `encode-helper`, FUSE-R-088 |
| ME04-W05 | Visibility rule, release on hide and lock, indicators | SH02, SH03 |
| ME04-W06 | Main window, modes, states | G-01, G-02 |
| ME04-W07 | Camera stack on HardenedBSD verified (UVC driver, formats) | G-18, VM05 |

### Phase 2: controls, documents, codes

| ID | Work item | Depends on |
|---|---|---|
| ME04-W08 | Manual controls and per-camera profiles | ME04-GAP-1 |
| ME04-W09 | `frame-analysis-helper`: page detection, warp, clean-up | — |
| ME04-W10 | Document session UI, `pdf-writer`, PDF/A | G-12 (better corner editing) |
| ME04-W11 | OCR through `svc.model` | G-15, FORGE-WP-29, FORGE-WP-31, AI02 |
| ME04-W12 | Barcode decoders, payload parsers, code cards, actions | IN01, SY01, CO02, CO03, SE04 |
| ME04-W13 | Burst, HDR bracket and merge | — |
| ME04-W14 | Location tagging | `svc.portal.location` |
| ME04-W15 | `scan` and `pick` intents | G-08, SH04 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| ME04-W16 | Virtual camera publisher, blur, replace, fail-closed, consumer list | ME04-GAP-2, G-15, `dev.gpu.compute` |
| ME04-W17 | Two cameras, interval capture, time-lapse, slow motion | — |
| ME04-W18 | DNG output, low-light stack, stabilisation | — |
| ME04-W19 | Network cameras | ME03 `net-fetch-helper` |
| ME04-W20 | Book mode, QR generator, Vault destination | SE05 |
| ME04-W21 | Level from sensors | ME04-GAP-3 |

## 13. Open questions and risks

1. **Camera stack on HardenedBSD** is Unverified. FreeBSD USB cameras commonly go through a user-space V4L2 compatibility layer; whether that fits a capability-mode portal, and which devices work, must be tested (G-18).
2. **The camera portal has no defined interface** (ME04-GAP-1). This app is its first demanding client and should drive its design with SH04.
3. **Virtual camera** needs the portal to accept app-provided sources (ME04-GAP-2) and `bg.run`. If refused, the alternative is to put blur inside the portal itself as a system feature.
4. **OCR quality** depends on which models AI02 ships; without a model, scanning still works but produces image-only PDFs.
5. **Indicator trust**: the app's own red dot is a convenience; only the shell indicator is trustworthy. The spec must never suggest otherwise.
6. **Recording while minimised** weakens "no capture without a visible window". It is off by default; the owner may want it removed.
7. **Hardware encode** is absent at first; 4K recording in software is not promised.
8. **Public-suffix data** for the link preview must ship offline and be updated with the system (SY05).
9. **Phase and priority conflict**: the catalogue lists "document scan, QR codes" in the one-line description; they are phase 2 here because they depend on helpers and models.
