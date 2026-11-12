<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Video Editor

By Odin Loch

| | |
|---|---|
| ID | `CR02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.video-editor` |
| Category | create |
| Priority | P2 |
| Backing repos | NeuralVFX (generated effect elements), FUSE (UI, `timeline`, `nodeGraph`, `curveEditor`), FORGE (ForgeScript, model service, signal code) |
| Status | Planned. Nothing built. |

Labels: **Verified** = read in a repo at the cited path. **Proposed** = design in this plan. **Unverified** = could not be read.

## 1. Purpose

A non-linear video editor for cutting, compositing, grading, mixing and exporting films, tutorials and short clips, all on the local machine. It replaces Kdenlive, Shotcut and DaVinci Resolve on this OS. NeuralVFX supplies generated visual-effect elements (fire, smoke, explosions, water, weather); the edit, decode, compositing, grading and export engines are new work.

## 2. Scope

**In scope:** media bin and proxies; multi-track timeline; trimming; multicam; transitions; titles; keyframes; effects and node compositing; colour grading with scopes; audio mixing and sync; speed changes; stabilisation; tracking and rotoscoping; local neural tools; generated effect elements from NeuralVFX; captions with local transcription; render queue; project files; autosave.

**Out of scope:**

| Item | Owner |
|---|---|
| Playing finished video | ME03 Video |
| Screen and camera recording | ME05 Screen Capture, ME04 Camera (their files import here) |
| Deep audio repair, MIDI, mastering | CR03 Audio Editor (round trip by intent) |
| Still-image retouching | CR01 Image Editor |
| 3D scenes, game cinematics, training effect models | CR04 Studio, NeuralVFX's own tools |
| Model install and signing | AI02 Model Manager |
| Upload to any site | none; export to a file and share through SH04 |
| Live streaming | not planned |

## 3. Users and top tasks

Personas: **Editor** (long-form cutting, keyboard driven). **Creator** (short clips, captions, quick export). **Colourist** (grading, scopes, HDR). **Compositor** (keying, tracking, node graphs, generated elements). **Teacher** (screen recordings, cuts, captions).

| # | Task | Target steps |
|---|---|---|
| T1 | Import a folder of clips and see thumbnails | 2 |
| T2 | Mark in and out in the source viewer and insert at the playhead | 3 keys (`I`, `O`, `,`) |
| T3 | Ripple-trim a cut point | 1 drag or 2 keys |
| T4 | Add a cross-dissolve to a cut | 1 key (`Ctrl+T`) |
| T5 | Add a title and animate its position | 5 |
| T6 | Sync a camera clip with a separate audio recording | 2 |
| T7 | Balance a shot with wheels and match it to another shot | 4 |
| T8 | Make captions from speech, fix them, burn in or export SRT | 4 |
| T9 | Add a generated fire element behind a subject and set wind | 4 |
| T10 | Stabilise a shaky clip | 2 |
| T11 | Export H.264 1080p with a preset | 3 |
| T12 | Recover the project after a crash | 1 |
| T13 | Hand a project folder to a colleague with media and hashes | 2 |
| T14 | Cut a three-camera interview live with number keys | 2 + one key per cut |

## 4. Features

Priority: M must, S should, C could. Phase 1 usable core, 2 full feature, 3 advanced.

### 4.1 Project and media bin

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F001 | Project folder | A project is a folder with `project.json`, sequences, caches and optional copied media (section 6.2). | M | 1 |
| CR02-F002 | Project settings | Frame rate (23.976 to 120, exact rationals), resolution, pixel aspect, working colour space, audio rate and layout, timecode start, drop-frame. | M | 1 |
| CR02-F003 | Import | Files and folders through the portal or drag and drop; image sequences detected by numbering; import report. | M | 1 |
| CR02-F004 | Media bin | Folders (bins), list and thumbnail views, columns (name, duration, fps, size, codec, colour, audio, date, hash, licence, notes), sort, type-ahead. | M | 1 |
| CR02-F005 | Hover scrub and preview | Scrub a thumbnail by moving the pointer; double-click opens the source viewer. | M | 1 |
| CR02-F006 | Metadata and tags | Keywords, rating, colour label, scene, take, reel, free notes, licence and source fields; smart bins from queries. | S | 2 |
| CR02-F007 | Media hashes | BLAKE3 of each file recorded at import; a changed file shows a "changed" badge. | M | 1 |
| CR02-F008 | Relink | Offline media shows red; relink by folder search using name, size and hash. | M | 1 |
| CR02-F009 | Proxies | Background transcode to an intra-frame proxy at 1/2 or 1/4 size; per-clip and global proxy toggle; export always uses originals. | M | 2 |
| CR02-F010 | Optimised media and render cache | Cache decoded or rendered frames for heavy clips and effects; cache size cap; coloured bar on the ruler. | S | 2 |
| CR02-F011 | Subclips and markers on clips | Named ranges and markers stored on the source clip. | S | 2 |
| CR02-F012 | Interpret footage | Override frame rate, field order, pixel aspect, colour space, alpha mode (straight, premultiplied), rotation. | M | 2 |
| CR02-F013 | Variable frame rate handling | Detect variable-rate files; conform to constant rate on import with a note. | S | 2 |
| CR02-F014 | Collect project | Copy or move all used media into the project folder, with or without handles, and write a manifest with hashes. | S | 2 |
| CR02-F015 | Transcript search | Search spoken words across the bin once clips are transcribed (4.13). | C | 3 |
| CR02-F016 | Stills, audio, sequences as media | Images (via CR01's formats), audio files, nested sequences and generated elements all live in the bin. | M | 1 |

### 4.2 Viewers

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F017 | Source and programme viewers | Two viewers or one switched; transport, timecode, in/out, zoom, safe areas, guides. | M | 1 |
| CR02-F018 | Playback | J K L shuttle (reverse, stop, forward, up to 32×), frame step, loop, play around, play in to out. | M | 1 |
| CR02-F019 | Playback quality | Full, half, quarter resolution; drop-frame indicator; "render then play" for heavy ranges. | M | 1 |
| CR02-F020 | Full-screen and second-display output | Clean feed on another output; colour-managed. | S | 2 |
| CR02-F021 | Compare | Split, wipe or difference against a reference frame or another clip. | S | 2 |
| CR02-F022 | Overlays | Timecode burn-in, clip name, title and action safe, centre cross, aspect masks (2.39, 1:1, 9:16). | S | 2 |
| CR02-F023 | On-screen controls | Transform, crop, mask and tracker handles drawn in the viewer. | M | 2 |
| CR02-F024 | Frame grab | Save the current frame as PNG or EXR; send to CR01. | S | 1 |

### 4.3 Timeline

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F025 | Tracks | Unlimited video, audio, title, caption and adjustment tracks; headers with name, lock, mute, solo, hide, sync lock, target, height. | M | 1 |
| CR02-F026 | Edit operations | Insert, overwrite, replace, fit to fill, append, place on top; three- and four-point editing. | M | 1 |
| CR02-F027 | Selection | Click, marquee, track-forward select, select by label; multi-select with Shift and Ctrl. | M | 1 |
| CR02-F028 | Move and snap | Drag clips with snapping to edits, playhead, markers; `N` toggles snap; nudge by frames. | M | 1 |
| CR02-F029 | Blade and join | Cut at playhead on targeted or all tracks; join through-edits. | M | 1 |
| CR02-F030 | Ripple delete and close gaps | Delete with ripple; find and close gaps per track. | M | 1 |
| CR02-F031 | Linked clips and groups | Video and audio linked; group, ungroup; link offset warning when out of sync. | M | 1 |
| CR02-F032 | Markers and regions | Timeline markers with colour, name, note, duration; chapter markers export. | M | 1 |
| CR02-F033 | Nested and compound sequences | Use a sequence as a clip; open in a tab; flatten. | S | 2 |
| CR02-F034 | Multiple sequences | Tabs; duplicate sequence as a version; compare two sequences side by side. | M | 1 |
| CR02-F035 | Ruler formats | Timecode (drop and non-drop), frames, seconds, feet+frames. | M | 1 |
| CR02-F036 | Zoom and navigate | Zoom to fit, to selection, to one frame; minimap strip; page or smooth scroll on play. | M | 1 |
| CR02-F037 | Clip display | Thumbnails (first, first+last, filmstrip), waveforms, name, speed, effect badges, proxy and offline badges. | M | 1 |
| CR02-F038 | Adjustment clips | A clip whose effects apply to everything under it. | S | 2 |
| CR02-F039 | Track and clip colours | Label colours; colour by source, by codec, by resolution. | C | 2 |
| CR02-F040 | Timeline search | Find clips by name, effect, marker text; step through results. | S | 2 |
| CR02-F041 | Duplicate-frame detection | Mark ranges of source used more than once. | C | 3 |

### 4.4 Trimming

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F042 | Ripple trim | Move one side of an edit; later clips shift. Tool `B`; `Q`/`W` trim start/end to playhead with ripple. | M | 1 |
| CR02-F043 | Roll trim | Move the cut; both sides change; duration unchanged. Tool `Ctrl+drag` on a cut or `R`. | M | 1 |
| CR02-F044 | Slip | Change the source range inside a clip without moving it. Tool `Y`. | M | 2 |
| CR02-F045 | Slide | Move a clip between its neighbours; they absorb the change. Tool `U`. | M | 2 |
| CR02-F046 | Trim view | Viewer shows outgoing and incoming frames (two-up) and the frame offsets while trimming. | S | 2 |
| CR02-F047 | Keyboard trim | Select an edit point; `,` `.` trim by one frame, `Shift` by five; loop play around the cut while trimming. | M | 2 |
| CR02-F048 | Asymmetric and multi-track trim | Trim several edits at once in different directions; sync locks keep other tracks aligned. | C | 3 |
| CR02-F049 | Handles check | Trim stops at the end of media and says so; transitions warn when handles are short. | M | 1 |

### 4.5 Multicam

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F050 | Multicam clip | Build from clips synced by timecode, in point, marker or audio waveform. | S | 2 |
| CR02-F051 | Angle viewer | Grid of up to 16 angles; keys `1`–`9` cut live during playback; `Alt+n` switches video only, `Shift+n` audio only. | S | 2 |
| CR02-F052 | Refine | Roll angle cuts, change angle after the fact, flatten to plain clips. | S | 2 |

### 4.6 Transitions and titles

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F053 | Video transitions | Cross-dissolve, dip to colour, additive, wipes (linear, radial, clock, iris), push, slide, zoom, blur dissolve, luma wipe from an image. | M | 1 |
| CR02-F054 | Audio transitions | Constant power, constant gain, exponential fades and crossfades. | M | 1 |
| CR02-F055 | Transition editing | Duration, alignment (centre, start, end), easing curve, default transition and duration. | M | 1 |
| CR02-F056 | Titles | Text clips with font, size, colour, stroke, shadow, background box, alignment, safe-area snapping. Needs G-03. | M | 2 |
| CR02-F057 | Title templates | Lower third, credits roll, crawl, subtitle style, countdown; templates are `.json` plus assets; user templates. | S | 2 |
| CR02-F058 | Text animation | Per-character or per-word animate in and out (fade, slide, type-on) with stagger. | C | 3 |
| CR02-F059 | Shapes and solids | Rectangle, ellipse, line, gradient and colour solids as generators. | S | 2 |

### 4.7 Keyframes and curves

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F060 | Keyframe any parameter | A stopwatch toggle on every numeric, colour and point parameter. | M | 2 |
| CR02-F061 | Keyframe lanes | Lanes under a clip in the timeline; drag keys in time; copy and paste keys. | M | 2 |
| CR02-F062 | Curve editor | Value curves with linear, hold, bezier (auto, unified, broken, flat) tangents; box select; scale; frame all. Uses `curveEditor` after FUSE-WP-50. | M | 2 |
| CR02-F063 | Easing presets | Ease in, out, in-out, bounce, overshoot as one-click presets on selected keys. | S | 2 |
| CR02-F064 | Motion paths | Position keys show as a path in the viewer with editable handles. | S | 3 |
| CR02-F065 | Expressions | A parameter may be a short signed ForgeScript expression of time and other parameters. | C | 3 |

### 4.8 Effects and compositing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F066 | Clip transform | Position, scale, rotation, anchor, opacity, crop, flip; blend modes (same list and formulas as CR01). | M | 1 |
| CR02-F067 | Effect stack | Ordered list per clip, track and sequence; enable, reorder, copy, paste attributes, save as preset. | M | 1 |
| CR02-F068 | Built-in effects | Blurs, sharpen, glow, vignette, grain, noise, mosaic, mirror, lens distortion, chromatic aberration, drop shadow, corner pin, timecode generator. | M | 2 |
| CR02-F069 | Keying | Chroma key (screen colour, tolerance, softness, spill suppression, core and edge matte), luma key, difference key; matte view. | M | 2 |
| CR02-F070 | Masks | Rectangle, ellipse, bezier masks per effect or clip; feather, expansion, invert; animated by keys or tracker. | M | 2 |
| CR02-F071 | Node graph | A clip or a sequence range can use a node graph instead of a stack: source, merge, transform, key, mask, blur, colour, tracker, generated element, output nodes. Uses `nodeGraph` after FUSE-WP-50. | S | 3 |
| CR02-F072 | Node graph tools | Typed ports (image, matte, data), viewer on any node, bypass, groups, comments, cache per node. | S | 3 |
| CR02-F073 | Alpha handling | Straight and premultiplied sources handled explicitly; all compositing in linear light, premultiplied. | M | 1 |
| CR02-F074 | GPU execution | Effect kernels run on GPU through FUSE compute where available; every kernel has a CPU reference. | S | 2 |
| CR02-F075 | Script effects | Signed ForgeScript effects that compose built-in kernels. No native plugins. | C | 3 |
| CR02-F076 | Picture in picture and split screen presets | Layout presets with gaps and borders. | C | 2 |

### 4.9 Generated effect elements (NeuralVFX)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F077 | Element browser | The NeuralVFX effect library (234 named effects, Verified count in `NeuralVFX:README.md`) listed by category with thumbnails; drag to the timeline as a generator clip with alpha. | S | 2 |
| CR02-F078 | Element controls | Each element's controls (for example intensity, wind, turbulence), seed, hue, brightness and speed as keyframable parameters. | S | 2 |
| CR02-F079 | Scene script clips | A NeuralVFX scene script (`.nvfxs`) as a generator clip; its named inputs become parameters; rule triggers become timeline markers. | S | 3 |
| CR02-F080 | Element passes | Colour, albedo, emission, normal and thickness passes of an element as separate node outputs for relighting the element in the compositor. | C | 3 |
| CR02-F081 | Weather element | The weather director as a generator: regime, wind, rain, snow, fog as parameters. | C | 3 |
| CR02-F082 | Volume element | The 3D fire or smoke volume demo models with a camera; rendered offline, not in real time. Marked experimental. | C | 3 |
| CR02-F083 | Element render size | Elements are generated at their native size (64–256 px sprites, larger for scenes) and scaled; the inspector shows native size and warns above 2× enlargement. | S | 2 |
| CR02-F084 | Deterministic elements | Same seed, same frames. The seed is stored in the project so renders repeat. | S | 2 |
| CR02-F085 | Continue from footage | Estimate an element's start state from a few frames of footage. Experimental: upstream validated it on simulated renders only. | C | 3 |

### 4.10 Colour

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F086 | Colour management | Input transform per clip, scene-linear working space, output transform per export and per display; Rec. 709, sRGB, Display P3, Rec. 2020, PQ, HLG, common camera log curves. | M | 2 |
| CR02-F087 | Primary wheels | Lift, gamma, gain, offset; temperature, tint, contrast, pivot, saturation; numeric fields. | M | 2 |
| CR02-F088 | Curves | Custom RGB and luma, hue vs hue, hue vs saturation, hue vs luma, luma vs saturation, saturation vs saturation. | M | 2 |
| CR02-F089 | Secondaries | HSL qualifier, shape windows, tracked windows; matte view; inside/outside grade. | S | 3 |
| CR02-F090 | Scopes | Waveform (luma, RGB parade), vectorscope with skin line, histogram, CIE chromaticity; graticules for SDR and HDR. Computed on GPU or at reduced rate on CPU. | M | 2 |
| CR02-F091 | LUTs | Apply `.cube` 1D and 3D as input, creative or output LUT; export a grade as `.cube`. | M | 2 |
| CR02-F092 | Shot match | Match exposure and balance to a reference frame; deterministic statistics, no model. | S | 3 |
| CR02-F093 | Grade management | Copy grade, stills gallery, versions per clip, apply to group, bypass all. | S | 2 |
| CR02-F094 | HDR | Grade and monitor HDR when the display path supports it (FUSE-WP-11); HDR10 static metadata on export; tone-map preview to SDR. | S | 3 |
| CR02-F095 | Gamut and legal checks | Broadcast-safe and gamut warnings as an overlay; clamp effect. | S | 3 |

### 4.11 Audio

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F096 | Audio tracks | Mono, stereo, 5.1 tracks; clip gain, pan, fades by handles; waveform display. | M | 1 |
| CR02-F097 | Mixer | Channel strips with fader, pan, mute, solo, meters; submix buses; master. | M | 2 |
| CR02-F098 | Volume automation | Keyframes on clip and track volume and pan; write by dragging the fader during play. | S | 2 |
| CR02-F099 | Audio effects | EQ, compressor, limiter, gate, de-noise, de-ess, reverb, delay (same DSP set as CR03). | S | 2 |
| CR02-F100 | Sync by waveform | Align clips or build synced clips by cross-correlation of audio. | M | 2 |
| CR02-F101 | Voice-over record | Record to a track from the microphone through the portal with a countdown. | S | 2 |
| CR02-F102 | Loudness | Integrated, short-term and momentary LUFS and true peak meters; normalise to a target on export. | M | 2 |
| CR02-F103 | Ducking | Lower music under speech using a sidechain; makes editable keyframes. | S | 3 |
| CR02-F104 | Round trip to CR03 | Send a clip or the mix to the Audio Editor by intent; the result returns as a new take. | C | 3 |
| CR02-F105 | Scrubbing and pitch-kept shuttle | Audible scrub; pitch-corrected playback up to 2×. | S | 2 |

### 4.12 Speed, stabilisation, tracking

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F106 | Constant speed and reverse | Percent or target duration; ripple option; keep pitch. | M | 1 |
| CR02-F107 | Speed ramps | Keyframed speed curve on a clip with smooth ramps; freeze frame. | S | 2 |
| CR02-F108 | Frame sampling | Nearest, frame blend, optical flow (classic), or model interpolation (4.13). | S | 2 |
| CR02-F109 | Stabilisation | Analyse camera motion (translation, rotation, scale, perspective); smoothness; crop, auto-scale or edge fill; rolling-shutter correction. | S | 2 |
| CR02-F110 | Point and planar tracking | Track points and planes; apply to transform, corner pin, masks; export tracking data as JSON. | S | 3 |
| CR02-F111 | Rotoscoping | Bezier shapes with keyframes; tracker-assisted propagation; feather per vertex. | S | 3 |
| CR02-F112 | Camera lens tools | Lens distortion removal from manual values or a calibration grid. | C | 3 |

### 4.13 Local neural tools (all through `svc.model`, all offline)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F113 | Upscale | 2× and 4× super-resolution per clip; rendered to cache. | C | 3 |
| CR02-F114 | Denoise | Temporal and spatial model denoise with strength. | C | 3 |
| CR02-F115 | Frame interpolation | Model-based in-between frames for slow motion and rate conversion. | C | 3 |
| CR02-F116 | Matting | Subject matte without a green screen; outputs an alpha channel; point prompt to pick the subject. | C | 3 |
| CR02-F117 | Relighting | Estimate normals and albedo for a subject and apply a simple light. Marked experimental. | C | 3 |
| CR02-F118 | Transcription | Speech to text with word timings, language detection and speaker turns; makes caption tracks. | S | 2 |
| CR02-F119 | Model disclosure and job control | Each tool shows model name, version, hash, signer and an estimate of time and memory; jobs run in the background, can pause, and results are cached per clip. | M | 2 |
| CR02-F120 | Provenance | Clips changed by a model carry a flag in the project; an optional "AI-processed" note in export metadata. | S | 2 |

### 4.14 Captions

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F121 | Caption tracks | Several per sequence, each with a language tag; cues as clips on the track. | M | 2 |
| CR02-F122 | Caption editor | Table of cues (in, out, text, speaker); split, merge, shift, snap to speech; reading-rate warning. | M | 2 |
| CR02-F123 | Import and export | SRT, WebVTT, TTML (IMSC subset), ASS (read; styles flattened), EDL of cue times. | M | 2 |
| CR02-F124 | Style and burn-in | Font, size, box, position, safe area; burn in or keep as a text stream in the container. | S | 2 |
| CR02-F125 | Translate | Translate a caption track with a local model; result is a new track marked machine-translated. | C | 3 |

### 4.15 Render and export

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F126 | Render queue | Jobs with sequence, range, preset, destination; reorder, pause, resume, cancel; runs with the window closed (job moves to notifications). | M | 1 |
| CR02-F127 | Export presets | Named presets by purpose (archive, web 1080p, web 4K, vertical, audio only, image sequence, review proxy); user presets as JSON. | M | 1 |
| CR02-F128 | Codecs and containers | See the table in 6.3. Open formats are the defaults (AV1 or VP9 + Opus in WebM or Matroska; FFV1 + FLAC for archive). | M | 1 |
| CR02-F129 | Hardware encode | Used only when the platform video pipeline (G-11) exposes it; always optional; the job states which encoder ran. | C | 3 |
| CR02-F130 | Rate control | Constant quality, target bitrate, two-pass; GOP length; pixel format; bit depth; colour tags. | M | 2 |
| CR02-F131 | Range and parts | Whole sequence, in to out, per marker region, per clip (stems), chapters from markers. | S | 2 |
| CR02-F132 | Audio export | Mix, stems per bus, channel layout, loudness target with a report. | S | 2 |
| CR02-F133 | Interchange | Export and import OpenTimelineIO JSON, CMX3600 EDL, FCPXML subset; export a media-linked AAF is not planned. | S | 3 |
| CR02-F134 | Metadata policy | Title, author, date, licence fields are opt-in; by default no device, location or user name is written. | M | 1 |
| CR02-F135 | Verify and sign | BLAKE3 of the output shown and saved in a sidecar; optional Ed25519 signature with a keyring key. | C | 3 |
| CR02-F136 | Smart render | Copy compressed frames unchanged where the edit allows it. | C | 3 |

### 4.16 Collaboration, safety, recovery, settings

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR02-F137 | Text project files | Sequences are JSON with stable ids and sorted keys, so DV03 can diff and merge them. | M | 1 |
| CR02-F138 | Sequence compare and merge | Show added, removed, moved and trimmed clips between two versions; take either side per change. | S | 3 |
| CR02-F139 | Lock file | A project opened twice (shared folder) opens read-only the second time with the holder's host name. | S | 2 |
| CR02-F140 | Review export | A low-rate file with burnt timecode plus an empty comments JSON; import the comments back as markers. | C | 3 |
| CR02-F141 | Autosave | Journal every edit; snapshot every 2 minutes; keep the last 50 snapshots. | M | 1 |
| CR02-F142 | Crash recovery | On start, offer the journal replay; a crashed helper restarts and playback resumes at the same frame. | M | 1 |
| CR02-F143 | Unlimited undo with history list | Per sequence; survives save; jump to a state. | M | 1 |
| CR02-F144 | Sandboxed decoding | All demuxing and decoding in the media helper with no capabilities beyond the passed handles. | M | 1 |
| CR02-F145 | Untrusted project open | A project from someone else opens with scripts, expressions and script effects disabled until the user allows them; shows signer if signed. | M | 2 |
| CR02-F146 | Workspaces | Edit, Colour, Audio, Effects, Captions layouts; `Shift+1..5`. | M | 1 |
| CR02-F147 | Shortcut sets | All rebindable; ready-made sets that mirror common editors. | S | 2 |
| CR02-F148 | Sleep inhibit and power | No sleep while rendering; on battery, pause background analysis (setting). | S | 1 |
| CR02-F149 | Disk space guard | Estimate output and cache size before a job; refuse to fill the disk. | M | 1 |

## 5. GUI

### 5.1 Window map

```
Main window (workbench with docks; workspaces Edit · Colour · Audio · Effects · Captions)
├─ Header bar: menu, project — sequence, search, badges ([mic] while recording, [gpu])
├─ Docks
│   ├─ Media bin            ├─ Effects and transitions browser   ├─ Elements (NeuralVFX)
│   ├─ Source viewer        ├─ Programme viewer                  ├─ Multicam viewer
│   ├─ Timeline (tabs per sequence)                              ├─ Inspector (clip, effect, title)
│   ├─ Keyframe / curve editor   ├─ Node graph                   ├─ Colour (wheels, curves, LUT)
│   ├─ Scopes               ├─ Audio mixer    ├─ Loudness        ├─ Captions table
│   ├─ Markers list         ├─ History        └─ Jobs (render queue, proxies, analysis, AI)
├─ Dialogs
│   ├─ New project / Project settings      ├─ Import report        ├─ Relink media
│   ├─ Export (preset editor)              ├─ Collect project      ├─ Interpret footage
│   ├─ Speed / duration                    ├─ Sync clips           ├─ Make multicam clip
│   ├─ Transcribe                          ├─ Stabilise            ├─ Sequence compare
│   ├─ Preferences                         └─ Keyboard shortcuts
├─ Popovers: marker edit, transition duration, track header menu, timecode entry
└─ Full-screen output window (second display)
```

### 5.2 Main window

Shown at 1600 × 900, Edit workspace, compact density. States: empty, loading, offline media, rendering, read-only.

```
+------------------------------------------------------------------------------------------+
| [=] Video Editor — trip / Main cut*        [search Ctrl+K]           [gpu] [▁] [▢] [✕]  |(1)
+-----------------------+--------------------------+--------------------------+------------+
| Media | Effects | Elem| Source: beach_04.mov      | Programme: Main cut      | Inspector  |
| [search____] [▤][▦]   |                          |                          | Clip       |
| ▸ Day 1               |                          |                          |  beach_04  |
| ▾ Day 2               |        (viewer)          |        (viewer)          | Transform  |
|  ▣ beach_01  00:12    |                          |                          |  Pos X [0] |
|  ▣ beach_04  00:31 P  |                          |                          |  Scale 100%|
|  ▣ drone_02  01:04 !  | 00:00:04:12 [I][O] ◀ ▶   | 01:00:12:08  ◀◀ ▶ ▶▶ [½] | Opacity 100|
|  ♪ music.flac 03:20   | [insert ,][overwrite .]  | [fit v] [safe] [compare] | Speed 100% |
|         (2)           |          (3)             |          (4)             | Effects (5)|
+-----------------------+--------------------------+--------------------------+------------+
| [↖][B][R][Y][U][✂] snap[N] link | Main cut × | Teaser × | +     01:00:12:08      [−▬▬+] |(6)
|      |01:00:00   |01:00:05   |01:00:10 ▼ |01:00:15   |01:00:20   |01:00:25   |          |(7)
| C1 ▣ |                    [ captions: "We left at dawn…" ][ … ]                         |
| V2 ▣ |            [ title: Day 2 ]                                                      |
| V1 ▣ |[ beach_01      ]⟋⟍[ beach_04                 ][ drone_02 !offline ]              |(8)
| A1 ♪ |[ beach_01 ~~~~ ]  [ beach_04 ~~~~~~~~~~~~~~~ ][ drone_02 ~~~~~~~~ ]              |
| A2 ♪ |[ music.flac ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ]  |
| (9)  |▁▁▁▁▁▁▁▁▁▁▁▁▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▁▁▁▁▁▁  render cache bar                             |
+------------------------------------------------------------------------------------------+
| 5 clips selected · proxy on · cache 3.1/20 GiB · job: proxies 4/12 ▬▬▬  · 23.976 fps Rec709|(10)
+------------------------------------------------------------------------------------------+
```

1. **Header bar.** No `[net]` badge: the app has `net.none`. `[mic]` shows during voice-over.
2. **Media bin**, 300 px. `treeView` (list) or `gridView` (thumbnails). Badges: `P` proxy ready, `!` offline, `≠` hash changed.
3. **Source viewer.** `videoWidget` fed by the media helper. Transport row of `toolButton`s; timecode is a `lineEdit` that accepts typed timecode.
4. **Programme viewer.** Same widget; overlays for safe areas and on-screen controls.
5. **Inspector**, 300 px. `propertyGrid` grouped by Transform, Speed, Audio, then one group per effect. Each row has a keyframe toggle and previous/next key arrows.
6. **Timeline toolbar.** Tools: select, ripple, roll, slip, slide, blade. Sequence tabs. Master timecode. Zoom slider.
7. **Ruler** with playhead `▼`, in/out range, markers.
8. **Tracks.** `timeline` widget (upgraded, 5.7). `⟋⟍` is a transition. Clip badges show state.
9. **Track headers**, 120 px: name, lock, mute or hide, solo, target, sync lock.
10. **Status bar.** Selection, proxy state, cache use, running job, sequence format.

### 5.3 Other views and dialogs

**Colour workspace** (1600 × 900). States: no clip selected (controls disabled), scope paused on battery.

```
+------------------------------------------------------------------------------------------+
| Stills gallery | Programme viewer (with compare wipe)              | Scopes [Parade v]    |
| [▣][▣][▣]      |                                                   |  R     G     B       |
|                |                                                   | ▁▃▆█▆▃ ▁▃▆█▆ ▁▂▅█▅▂ |
|                |                                                   | [Vectorscope] [Histo]|
+----------------+-----------------------------------+---------------+----------------------+
| Input [S-Log3 v] Working [ACEScct v] Output [Rec709 v]  LUT [none v]    [bypass] [reset]  |
|   Lift (○)      Gamma (○)      Gain (○)     Offset (○)   | Curves [Custom v]              |
|   [▬▬▬▬]        [▬▬▬▬]         [▬▬▬▬]       [▬▬▬▬]       |  curveEditor                   |
|   Temp [0] Tint [0] Contrast [1.00] Pivot [0.43] Sat [1.00]| Qualifier H S L  [matte view] |
+------------------------------------------------------------------------------------------+
| mini timeline: [clip][clip][>clip<][clip]   grade version [2 of 3 v]  [copy] [paste]     |
+------------------------------------------------------------------------------------------+
```

**Export** (900 × 640). States: estimating, not enough disk (Export disabled with the numbers), encoder missing.

```
+---------------------------------- Export -----------------------------------------------+
| Presets              | Name [Main cut_v3__________]  To [~/Videos/out…]                 |
| > Web 1080p (AV1)    | Container [Matroska v]                                           |
|   Web 4K (AV1)       | Video [AV1 (software) v]  Quality [CQ 28]  Speed [6]             |
|   Archive (FFV1)     |   Size [1920×1080 v] Rate [as sequence v] Depth [10-bit v]       |
|   H.264 compatible   |   Colour [Rec.709 SDR v]  [ ] HDR10 metadata                     |
|   Vertical 9:16      | Audio [Opus v] [160 kb/s] [Stereo v]  Loudness [-14 LUFS v]      |
|   Audio only (FLAC)  | Captions ( ) none (•) text stream ( ) burn in  Track [English v] |
|   Image sequence     | Range (•) whole ( ) in–out ( ) per region                        |
|   [+ Save preset]    | Metadata [none v]   [ ] Write hash sidecar  [ ] Sign             |
|                      | Estimate: 412 MiB · about 6 min · free 212 GiB                   |
|                      |                           [Cancel] [Add to queue] [Export now]   |
+-----------------------------------------------------------------------------------------+
```

**Captions workspace** (table dock + viewer). States: transcribing (rows fill as they arrive), no model installed (Transcribe disabled with a link to AI02).

```
+----------------------------------------------+------------------------------------------+
| Track [English v] [+] [Transcribe…] [Import…]| Programme viewer with caption preview    |
| #  In          Out         Speaker  Text     |                                          |
| 1  01:00:01:02 01:00:03:10 A        We left… |                                          |
|>2  01:00:03:12 01:00:06:00 A        The road…|      "The road was empty."               |
| 3  01:00:06:05 01:00:08:20 B        Look!    | 17 cps ✓  2 lines ✓                      |
| [split] [merge] [shift…] [snap to speech]    | Style [Default v] Position [bottom v]    |
+----------------------------------------------+------------------------------------------+
```

**Node graph** (dock, 900 × 500). States: cycle refused (toast), node error (red border with message).

```
+------------------------------------------------------------------------------------------+
| [+ node Ctrl+Space] [group] [frame all F]            viewer on: [Merge 2 v]              |
|  [Source: actor]──▶[Key]──▶[Despill]──┐                                                  |
|                                       ├─▶[Merge 1]──┐                                    |
|  [Element: campfire]──▶[Transform]────┘             ├─▶[Merge 2]──▶[Colour]──▶[Output]   |
|  [Source: plate]────────────────────────────────────┘                                    |
|                                                                          [minimap]       |
+------------------------------------------------------------------------------------------+
```

**Elements browser and inspector** (dock). States: pack missing (empty state with reason), generating thumbnail.

```
+---------------- Elements -----------------+  +------------ Inspector: Element ----------+
| [search___] [Fire v]                      |  | campfire   native 128 px   model 82 KiB  |
| [▣ campfire] [▣ torch] [▣ wildfire]       |  | Seed [42____] [new]                      |
| [▣ candle ] [▣ coals] [▣ window fire]     |  | Intensity [▬▬▬▬▬▬○▬▬] 0.70  ◆            |
| 234 effects · pack hash 3f9a… ✓ verified  |  | Wind      [▬▬○▬▬▬▬▬▬] 0.20  ◆            |
| Drag to the timeline or the node graph    |  | Turbulence[▬▬▬▬○▬▬▬▬] 0.50  ◇            |
+-------------------------------------------+  | Hue [0°] Brightness [1.0] Speed [1.0]    |
                                               | Passes: [x] colour [ ] normal [ ] emission|
                                               | Scale 180% ⚠ above 2× looks soft         |
                                               +------------------------------------------+
```

**Audio mixer** (dock): one strip per track and bus with `slider` fader, pan `dial`, mute, solo, record-arm, insert slots, level meter; master strip with LUFS read-out.

**Relink media** (dialog): `tableView` of offline clips with old location, found candidate, match basis (hash, name+size); buttons "Search folder…", "Relink 12", "Skip".

### 5.4 Menus

```
File      New Project Ctrl+N · Open Ctrl+O · Recent ▸ · Save Ctrl+S · Save As Ctrl+Shift+S
          Import Media Ctrl+I · Import ▸ (Captions, Timeline OTIO/EDL/XML) · Relink…
          Collect Project… · Export… Ctrl+E · Add to Render Queue Ctrl+Shift+E
          Export Frame Ctrl+Shift+F · Project Settings… · Close Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut/Copy/Paste · Paste Insert Ctrl+Shift+V
          Paste Attributes Alt+V · Delete Del · Ripple Delete Shift+Del · Select ▸
          Find in Timeline Ctrl+F · Shortcuts… · Preferences Ctrl+,
Clip      Insert , · Overwrite . · Replace · Fit to Fill · Speed/Duration Ctrl+R · Freeze Frame
          Link Ctrl+L · Group Ctrl+G · Nest · Enable Shift+E · Make Proxy · Interpret Footage…
          Sync… · Make Multicam Clip… · Stabilise… · Open in Audio Editor · Open Frame in Image Editor
Sequence  New Ctrl+Alt+N · Settings… · Add Track ▸ · Blade Ctrl+B · Blade All Ctrl+Shift+B
          Add Transition Ctrl+T · Add Audio Transition Ctrl+Shift+T · Close Gaps
          Render In to Out Enter · Delete Render Cache · Compare With…
Mark      In I · Out O · Clear Alt+X · Mark Clip X · Add Marker M · Edit Marker Shift+M
          Next/Previous Marker Shift+Down/Up · Go to In/Out Shift+I/O · Next/Previous Edit Down/Up
Effects   Add Effect ▸ · Add Title ▸ · Add Generator ▸ · Add Element… · Remove All
          Keyframes ▸ (Add, Next, Previous, Ease) · Open Node Graph · Track…
Colour    Bypass Grade Shift+D · Copy Grade · Paste Grade · Grab Still · Apply LUT… · Shot Match
Audio     Mixer · Record Voice-over · Normalise… · Duck… · Transcribe…
View      Workspace ▸ Shift+1..5 · Viewer ▸ (Fit, 100%, Quality, Safe Areas, Compare)
          Proxy Playback · Zoom In/Out = / - · Zoom to Fit Shift+Z · Snap N · Full Screen F11
Window    (each dock) · Jobs · Reset Layout
Help      Video Editor Help F1 · Shortcuts · About
```

### 5.5 Keyboard and pointer

Transport: `Space` play/stop, `J K L` shuttle, `Left/Right` one frame, `Shift+Left/Right` five frames, `Home/End`, `Up/Down` previous/next edit. Tools: `A` select, `B` ripple, `R` roll, `Y` slip, `U` slide, `C` blade, `H` hand, `Z` zoom. Edit: `Q`/`W` ripple trim start/end to playhead, `,` insert, `.` overwrite (when no edit point is selected), `D` disable clip, `F` match frame, `Shift+F` reveal in bin, `` ` `` maximise the dock under the pointer.

| Where | Context menu |
|---|---|
| Bin item | Open in Source, Insert, Overwrite, Make Proxy, Interpret Footage…, Relink…, Reveal in Files, Transcribe, Label ▸, Properties |
| Timeline clip | Cut, Copy, Paste Attributes, Ripple Delete, Speed…, Link, Group, Nest, Enable, Replace With ▸, Match Frame, Reveal in Bin, Stabilise…, Open in Audio Editor, Label ▸ |
| Edit point | Add Transition, Ripple Trim, Roll Trim, Join Through-Edit |
| Empty track area | Paste, Close Gap, Add Track, Add Marker |
| Track header | Rename, Height ▸, Lock, Delete Track, Add Track Above/Below |
| Keyframe | Interpolation ▸, Ease ▸, Copy, Delete |
| Node | Bypass, View, Group, Cache, Delete |
| Caption row | Split, Merge with Next, Shift…, Set Speaker ▸, Delete |

Drag and drop: files from SY02 into the bin or timeline (import then place); bin item to the timeline (insert with `Ctrl`, overwrite default); effect onto a clip; element onto a track or the node graph; LUT file onto a clip; clip from the timeline to the bin (makes a subclip). Gestures: pinch zooms the timeline; two-finger scroll pans; `Alt`+wheel zooms at the pointer.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Project hub: "Create a project" with New, Open and recent projects in a `gridView`. In an empty bin: "Import media" button and a drop target. |
| Loading | Bin rows appear at once with a placeholder; thumbnails and waveforms fill in; the timeline is usable before analysis ends. |
| Offline media | Clips draw red hatched with the file name; the viewer shows "Media offline"; export asks whether to stop or render black. |
| Error | A decode error marks the clip with `!` and the helper's message; the helper restarts; other clips still play. |
| Rendering | Jobs dock and status bar show progress; editing other sequences stays allowed. |
| Offline (network) | No change; the app never uses the network. |
| Locked | Session lock stops playback and recording and blanks the viewers. |
| Read-only | Project lock held elsewhere or media folder read-only: banner, edits disabled, Save As enabled. |
| Low disk | Banner with numbers; proxies and cache stop growing; queued jobs wait. |
| Narrow (< 1100 px) | One viewer (switched); inspector becomes a sheet. Under 640 px: bin is an overlay, timeline shows two tracks with a track picker. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Workbench | `dockArea`, `dockWidget`, `tabWidget`, `splitter` |
| Viewers | `videoWidget` (a frame-source stub today, V3) — needs G-11 (FUSE-WP-53) |
| Bin, markers, jobs, captions | `treeView`, `gridView`, `tableView`, `progressBar` |
| Timeline | `timeline` — see below |
| Curves | `curveEditor` — see below |
| Node graph | `nodeGraph` — see below |
| Inspector | `propertyGrid`, `slider`, `spinBox`, `colorPicker`, `comboBox`, `checkBox` |
| Mixer | `slider`, `dial`, `toolButton`; meters need FUSE-WP-49 |
| Scopes | **Missing**: CR02-GAP-1 |
| Wheels | **Missing**: CR02-GAP-2 |

What the three editor widgets have today (Verified, `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp:277-320`):

| Widget | Has | Lacks for this app (Proposed in FUSE-WP-50) |
|---|---|---|
| `timeline` | Named tracks; clips `[start, end)` with a label and colour; a time ruler and playhead; ruler drag scrubs; clip drag moves with snap to a fixed step; wheel zoom; a view range with Ctrl+wheel zoom and Shift+wheel pan; query of clips under a time. Times are `f32` seconds. Data is stored inside the widget. | A model-backed design; exact rational time (f32 seconds cannot address frames in long sequences); trim handles; ripple, roll, slip, slide gestures; multi-select and marquee; track headers; transitions; thumbnails and waveforms in clips; keyframe lanes; markers and regions; timecode ruler formats; snapping to edits and markers; virtualisation for 10 000 clips; drag and drop from outside. |
| `curveEditor` | One float curve; keys `(t, v)`; linear, smooth and step interpolation; click to add, drag to move, Delete to remove; `curveEvaluate`. | Several curves at once; bezier tangents; box select and scale; pre and post behaviour; snapping; frame all; per-curve lock and visibility. |
| `nodeGraph` | Nodes with typed input and output pins; bezier links; drag to link with type check; one link per input; cycles refused; pan and zoom; delete; topological order query. | Marquee and multi-select; groups and comments; reroute points; widgets inside nodes; minimap; search-to-add; level of detail for large graphs; model backing. |

Gaps:

- **G-11** (FUSE-WP-52 audio, FUSE-WP-53 video surface, FUSE-WP-54 HDR and subtitles): this app needs frame-accurate seek, random access for scrubbing, several streams at once (multicam), audio-master clock, and frames delivered as GPU images or shared memory.
- **CR02-GAP-1**: scope widgets (waveform, parade, vectorscope, histogram, CIE) drawn from a small data image.
- **CR02-GAP-2**: colour wheel control (a 2D puck plus a ring slider).
- **CR02-GAP-3**: cross-process GPU image sharing between the render helper and the UI (same need as CR01-GAP-3).
- **CR02-GAP-4**: a system-wide media codec service or shared helper for encode (G-11 as written covers playback; encode is not stated).
- Text shaping for titles and captions: G-03. Audio meters and waveforms: FUSE-WP-49.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Project and sequence model, edit operations, undo journal, tools, presets, job list, intents |
| UI | FUSE UI (`.fuseui` + CSS) | app host | Workbench, docks, dialogs |
| `media-io` | C++23 (wraps codec libraries) | helper, one per open file group, no capabilities | Demux, decode, thumbnails, waveforms, proxy transcode input. Parses untrusted bytes: PLATFORM.md section 2 (a). |
| `render-engine` | C++23 | helper | Frame graph: evaluates a sequence at a time into frames and audio; compositing, effects, colour, speed, tracking, stabilisation analysis; GPU kernels. Section 2 (b). |
| `encode` | C++23 (wraps encoders and muxers) | helper, one per job | Encode and mux to the passed output handle. Section 2 (b). |
| `element-gen` | C++23 (wraps `libnvfx`) | helper, no capabilities | Loads a NeuralVFX effect pack and renders elements and scene scripts to RGBA frames. Section 2 (d). |
| `audio-engine` | C++23 | helper (shared design with CR03) | Mixing, effects, resampling, loudness; feeds the system audio pipeline |
| Model client | ForgeScript | app host | Queues frames and audio to `svc.model` |

```
 +----------------------- app host (org.pbsd.video-editor) ------------------------+
 | ForgeScript: project, sequences, edit ops, journal, jobs                        |
 | FUSE UI: docks, timeline, viewers (videoWidget) ◀── frames (GPU image / shm)    |
 +------┬---------------┬----------------┬----------------┬----------------┬--------+
        │ handles       │ graph + time   │ job            │ element, seed  │ tiles, PCM
        ▼               ▼                ▼                ▼                ▼
    media-io ──frames─▶ render-engine ──frames─▶ encode   element-gen     svc.model
    (no caps)           (dev.gpu.compute)  │      (output  (no caps,       (ai.local)
                             ▲             │       handle)  CPU only)
                             └─ audio-engine ─▶ svc.media (monitoring)
```

The app host holds no pixels except thumbnails. The render engine pulls decoded frames from `media-io` and element frames from `element-gen` over shared memory. A crash in any helper loses at most the frame in flight.

### 6.2 Data model

Project folder:

```
<name>.vproj/
  project.json            settings, bins, media records, schema version
  sequences/<uuid>.json   one file per sequence
  grades/<uuid>.json      grade versions and stills metadata
  captions/<uuid>.vtt     caption tracks (WebVTT with cue ids)
  graphs/<uuid>.json      node graphs
  presets/                project-local export and effect presets
  media/                  optional collected media
  cache/                  proxies, waveforms, thumbnails, render cache (never backed up)
  autosave/               journal and snapshots
  lock                    holder host and time
```

| Entity | Fields |
|---|---|
| `Project` | `schema: u32`, `id: uuid`, `name: string`, `fps: {num: u32, den: u32}`, `size: {w,h}`, `par: rational`, `working_space: string`, `audio: {rate: u32, layout: string}`, `tc_start: i64`, `drop_frame: bool` |
| `Media` | `id: uuid`, `kind: enum{video,audio,image,sequence_images,element,scene_script}`, `handle_token: string`, `name`, `size_bytes: u64`, `blake3: hex`, `duration: rational`, `streams: [Stream]`, `interpret: {...}`, `proxy: path?`, `tags: [string]`, `licence: string`, `notes: string`, `ai: [string]` |
| `Stream` | `index: u32`, `type`, `codec: string`, `fps`, `size`, `pix_fmt`, `colour: {primaries, transfer, matrix, range}`, `channels`, `rate` |
| `Sequence` | `id`, `name`, `fps`, `size`, `tracks: [Track]`, `markers: [Marker]`, `in: i64?`, `out: i64?` |
| `Track` | `id`, `kind: enum{video,audio,title,caption,adjust}`, `name`, `lock`, `mute`, `solo`, `sync_lock`, `height: u16`, `items: [Item]`, `effects: [Effect]` |
| `Item` (clip) | `id`, `media: uuid`, `stream: u32`, `start: i64` (sequence frames), `duration: i64`, `source_in: rational`, `speed: Curve or f64`, `link: uuid?`, `group: uuid?`, `enabled: bool`, `transform: {...}`, `blend: enum`, `effects: [Effect]`, `graph: uuid?`, `grade: uuid?`, `label: u8` |
| `Transition` | `id`, `track`, `at: i64`, `duration: i64`, `align: enum`, `type: string`, `params: json` |
| `Effect` | `id`, `type: string`, `enabled`, `params: map<string, Param>`, `masks: [Mask]` |
| `Param` | `value: json` or `keys: [{t: i64, v: json, interp: enum, in: (f32,f32), out: (f32,f32)}]` or `expr: string` |
| `Element` (media params) | `pack_blake3: hex`, `entry: string`, `seed: u64`, `controls: map<string, Param>`, `hue: f32`, `brightness: f32`, `speed: f32`, `size_px: u32` |
| `Marker` | `id`, `t: i64`, `duration: i64`, `name`, `note`, `colour: u8`, `chapter: bool` |
| `Job` | `id`, `kind: enum{export,proxy,analysis,ai}`, `sequence`, `range`, `preset: json`, `state`, `progress: f32`, `output_token`, `log` |

All sequence times are integer frames at the sequence rate; source times are rationals. No floating-point time is stored. JSON is written with sorted keys and one item per line so text diff and merge work (feature F137). `schema` rises on any change; older projects migrate in memory and a copy of the old files goes to `autosave/pre-migration/`.

### 6.3 File formats and protocols

Codec and container table (targets; what is actually available depends on G-11 and CR02-GAP-4):

| Format | Decode | Encode | Library or own code | Runs in |
|---|---|---|---|---|
| AV1 | yes | yes | dav1d (BSD) decode; SVT-AV1 (BSD) or rav1e (BSD, Rust) encode | `media-io`, `encode` |
| VP9, VP8 | yes | VP9 | libvpx (BSD) | `media-io`, `encode` |
| H.264 | yes | yes | Decode: FFmpeg libavcodec (LGPL build); encode: x264 (GPL-2+) or OpenH264 (BSD). Patent position is the user's jurisdiction's; the open presets are the defaults. | `media-io`, `encode` |
| HEVC | yes | optional | libavcodec decode; x265 (GPL-2+) encode, off by default | `media-io`, `encode` |
| ProRes, DNxHR | yes | proxy use | libavcodec | `media-io`, `encode` |
| FFV1 | yes | yes | libavcodec | `media-io`, `encode` |
| MJPEG, image sequences (PNG, EXR, TIFF, JPEG) | yes | yes | CR01's codec set | `media-io`, `encode` |
| Opus, Vorbis, FLAC, PCM | yes | yes | libopus (BSD), libvorbis (BSD), libFLAC (BSD), own PCM | `media-io`, `encode` |
| AAC, MP3, AC-3 | yes | AAC | libavcodec | `media-io`, `encode` |
| Matroska, WebM | yes | yes | Own demuxer preferred; libavformat fallback | `media-io`, `encode` |
| MP4, MOV | yes | yes | libavformat | `media-io`, `encode` |
| MXF, AVI, MPEG-TS | yes | no | libavformat | `media-io` |
| WAV, AIFF, Ogg | yes | WAV, Ogg | Own code | `media-io`, `encode` |
| SRT, WebVTT, TTML, ASS | yes | SRT, VTT, TTML | Own code | app host (bounded text) |
| `.cube` LUT | yes | yes | Own code | app host, engine |
| OpenTimelineIO JSON, CMX3600 EDL, FCPXML subset | yes | yes | Own code | app host |
| NeuralVFX `.nvfxp` pack, `.nvfx`, `.nvfxl`, `.nvfxs`, `.nvfx3` | yes | no | `libnvfx` C API | `element-gen` |
| Project JSON | yes | yes | Own code | app host |
| Hardware encode and decode | — | — | Only through the platform pipeline (G-11); Unverified on HardenedBSD | platform |

FFmpeg's libraries are LGPL-2.1+ when built without GPL parts; with x264 or x265 the encode helper becomes GPL, which is compatible with this project's AGPL-3.0-or-later. All are C outside the safe subset: they run only in helpers with no capabilities, are vendored, and build offline.

### 6.4 Integration

- **Intents handled:** `open` (project, or a media file → new project with that clip), `edit`, `new`, `share` target. **Emitted:** `share`, `open` (CR03 round trip, CR01 frame edit), `view` (ME03 plays an export).
- **MIME:** edit `application/x-pbsd-video-project+json`; import the video, audio, image and caption types of 6.3; export those plus `application/x-otio+json`.
- **Services:** `svc.portal.file`, `svc.portal.mic`, `svc.settings`, `svc.notify`, `svc.media`, `svc.power`, `svc.model`, `svc.thumbnail`, `svc.clipboard`, `svc.keyring`, `svc.backup` (excludes `cache/`), `svc.intent`, `svc.a11y`.
- **Other apps:** ME03 plays results; ME05 and ME04 supply recordings; CR03 for audio work; CR01 for stills; CR04 Studio can export image sequences and cinematics that import here; DV03 versions the project folder; AI02 installs models; SE04 holds signing keys.

**What NeuralVFX provides (Verified) and how it maps to this app:**

| NeuralVFX capability | Source | Use here |
|---|---|---|
| Small per-effect networks that draw sprites (64–256 px) from time, artist controls and a seed, on one CPU core, no GPU | `NeuralVFX:README.md` | Generator clips (feature F077, F078) |
| Runtime C API: load effect, create instance, set controls, seed, colour (hue, brightness), render RGBA at a time, bake a flipbook | `NeuralVFX:include/neuralfx/nvfx.h` | `element-gen` helper calls |
| Effect library: 234 named effects in `.nvfxl` catalogues, with a spawn-and-drive C API | `NeuralVFX:README.md`, `library/`, `include/neuralfx/nvfx_library.h` | Element browser |
| Scene scripts (`.nvfxs`): coupled effects, force fields, particles, light, bloom; C API with seek, step, inputs, rule triggers, module controls | `NeuralVFX:include/neuralfx/nvfx_scene.h`, `examples/scenes/` | Scene script clips (F079) |
| Engine layers (lit sprites): colour, albedo, emission, normal, thickness as RGBA8, RGBA16F or RGBA32F | `NeuralVFX:include/neuralfx/nvfx_scene.h`, `docs/ENGINES.md` | Element passes (F080) |
| Weather director: seven regimes with seeded gusts and rain cells | `NeuralVFX:docs/WEATHER.md` | Weather element (F081) |
| 3D volume demo (fire, smoke) with a physically based volume renderer and any camera; about 21–77 ms a step on CPU | `NeuralVFX:README.md`, `include/neuralfx/nvfx_volume.h` | Volume element, offline only (F082) |
| Effect packs (`.nvfxp`): one file with models, catalogue and a SHA-256 per member | `NeuralVFX:include/neuralfx/nvfx_pack.h` | The pack shipped in the bundle; hash shown in the browser |
| Effect manager: many effects within a CPU budget | `NeuralVFX:include/neuralfx/nvfx_manager.h` | Preview playback of many elements |
| Start points from footage (`nvfx_estimate`), validated on simulated renders only | `NeuralVFX:docs/FOOTAGE.md` | Continue from footage, experimental (F085) |
| Ingest of footage through an ffmpeg pipe with a licence register | `NeuralVFX:include/neuralfx/ingest.hpp` | Not used at run time; the idea of a licence field per clip is reused in the bin |
| Metrics: PSNR, SSIM, temporal PSNR, flicker | `NeuralVFX:README.md` (`src/core/metrics.cpp`) | Could back the viewer compare tool; Proposed |
| Determinism: same calls, same pictures on any worker count | `NeuralVFX:docs/FUSE.md` | Repeatable renders (F084) |
| Sample FUSE integration against a stand-in FUSE (no FUSE source in that repo) | `NeuralVFX:engines/fuse/fuse_engine.hpp` | Pattern only; this app uses the C API directly |
| Licence AGPL-3.0-or-later; C++23; no third-party runtime dependency | `NeuralVFX:README.md` | Fits the suite |

**What NeuralVFX does not provide (Verified by reading the README and headers):** it is not a video editing engine. It has no timeline, no media decode or encode of its own (video goes through an `ffmpeg` pipe for baselines and figures, `NeuralVFX:include/neuralfx/video_codec.hpp`), no general compositing or grading, and no footage upscaling, denoising, frame interpolation, matting or relighting. Its learned 2× upscaler applies to its own effects only and did not pass its own test (`NeuralVFX:include/neuralfx/upscale.hpp`, `README.md`). So the neural tools of 4.13 are Proposed and depend on separate models through `svc.model`, not on NeuralVFX. Build notes: upstream builds on Ubuntu with g++-14; a HardenedBSD build with clang and libc++ is Unverified; BC7 and ASTC encoders and the viewer's Dear ImGui are fetched at configure time and must be switched off for the offline build (`NeuralVFX:CMakeLists.txt`).

FORGE: `FORGE:examples/integrations/neuralvfx_stream.cpp` is a 36-line sample that feeds filtered noise through `forge::inference::streaming::session` with `forge::signal` (biquad, FFT) and a Philox generator and checks that two runs match. It does not link NeuralVFX. It shows the streaming-session pattern that the audio analysis here could use. `FORGE:examples/integrations/fuse_runtime.cpp` shows `forge::task::pool` and SIMD dispatch for row blending.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Settings, presets, default cache |
| `fs.portal` | Project folders, media, exports; grants remembered per project |
| `net.none` | No network use |
| `proc.helper:media-io`, `:render-engine`, `:encode`, `:element-gen`, `:audio-engine` | The five helpers of 6.1 |
| `dev.gpu.compute` | GPU effects, scopes, colour |
| `dev.mic` | Voice-over recording, prompt on first use, indicator on |
| `ai.local`, `svc.model` | Transcription and the tools of 4.13 |
| `svc.media` | Audio output, transport keys |
| `svc.power` | Inhibit sleep during export |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.mic`, `svc.thumbnail`, `svc.clipboard`, `svc.keyring`, `svc.backup`, `svc.intent`, `svc.a11y` | As listed in 6.4 |
| `bg.run` | Let the render queue finish after the window closes; shown in notifications |

Not requested: `net.host`, `net.user`, `proc.exec`, `dev.camera`, `dev.screen`, `dev.location`, `bg.autostart`.

**Assets:** unreleased footage, voices and faces, project files, metadata (location, device), signing keys, CPU and disk (a render can be a denial of service on the user's own machine).

**Adversaries:** crafted media files (containers and codecs are a long-standing attack surface); a project folder from an untrusted sender (paths, expressions, script effects, huge values); crafted caption, LUT and interchange files; a hostile model; a tampered effect pack.

| Surface | Mitigation |
|---|---|
| Demuxers and decoders | Only in `media-io`: Capsicum capability mode, input descriptors only, memory and CPU limits, restart on crash. Frame geometry is checked before the render engine accepts a frame. |
| Encoders and muxers | In `encode`, with only the output handle. |
| Project files | Parsed in ForgeScript (memory safe). Media is referenced by portal token plus hash, never by raw path; a path inside a received project grants nothing. Numeric fields are range-checked. |
| Expressions and script effects | Signed ForgeScript only; disabled for projects not made on this machine until allowed (feature F145); run with no `fs`, `net` or `proc`; time-limited per frame. |
| NeuralVFX packs | Shipped in the signed bundle; the pack's per-member SHA-256 is checked by `libnvfx`; user packs must be signed. `element-gen` has no capabilities. |
| GPU | Fixed kernels from the bundle; no user shaders. |
| Export metadata | Nothing personal by default (feature F134). |
| Caches | Under the project or `~/.cache`; excluded from backup; a "Delete all caches" action. |
| Microphone | Portal prompt and indicator; recording only while the record button is on. |
| Disk exhaustion | Estimates and guards before every job (feature F149). |

**Stored:** project folders where the user puts them; presets and settings in the app's own directories. **Leaves the machine:** nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `general.workspace` | enum | `edit` | Workspace at start |
| `general.reopen_last` | bool | `true` | Reopen the last project |
| `project.default_fps` | rational | `25/1` | New project frame rate |
| `project.default_size` | string | `1920x1080` | New project size |
| `timeline.snap` | bool | `true` | Snapping on |
| `timeline.default_transition_frames` | int | `24` | Default transition length |
| `timeline.thumbnails` | enum | `first_last` | none, first, first_last, filmstrip |
| `timeline.scroll_on_play` | enum | `page` | none, page, smooth |
| `playback.quality` | enum | `auto` | auto, full, half, quarter |
| `playback.use_proxies` | bool | `true` | Play proxies when present |
| `playback.pitch_keep` | bool | `true` | Keep pitch when shuttling |
| `proxy.auto_above` | string | `2560x1440` | Make proxies for larger sources |
| `proxy.scale` | enum | `half` | half, quarter |
| `cache.location` | enum | `project` | project or app cache |
| `cache.cap_gib` | int | `20` | Render cache limit |
| `perf.gpu` | enum | `auto` | auto, on, off |
| `perf.pause_analysis_on_battery` | bool | `true` | Pause background jobs on battery |
| `colour.working_space` | string | `ACEScct` | Default working space |
| `colour.scopes_rate_hz` | int | `15` | Scope refresh rate |
| `audio.loudness_target` | float | `-14.0` | Default LUFS target |
| `audio.monitor_device` | string | system | Output device |
| `autosave.snapshot_minutes` | int | `2` | Snapshot interval |
| `autosave.keep` | int | `50` | Snapshots kept |
| `export.default_preset` | string | `web-1080p-av1` | Preset used by Export Now |
| `export.metadata` | enum | `none` | none, basic |
| `ai.enabled` | bool | `false` | Show model tools |
| `ai.transcribe_language` | string | `auto` | Language hint |
| `scripts.allow_foreign` | bool | `false` | Run signed expressions from received projects without asking |

## 9. Accessibility and languages

- The timeline exposes tracks as rows and clips as items with name, start, duration and state, so a screen reader can read and edit a cut. All edit operations have keyboard forms.
- The playhead position and transport state are a live region (announced on stop, not on every frame).
- Scopes have a text summary (min, max, mean luma; clipped percent).
- Captions are first-class: the editor checks reading rate and line length, and export keeps text streams by default.
- Audio meters show numbers as well as bars; clip badges use shapes and letters, not colour alone.
- Flashing content warning: an analysis job flags sequences with more than three flashes a second.
- Titles and captions support all scripts of the FULL text profile; caption tracks carry BCP 47 language tags.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to project hub | ≤ 400 ms |
| Open a 2-hour project (5000 clips) to usable timeline | ≤ 2 s |
| Timeline scroll and zoom | 60 fps with 10 000 clips (FUSE-WP-50 target) |
| Edit operation applied and drawn | ≤ 16 ms |
| Play 1080p 30 fps, 3 video tracks, 2 effects each | No dropped frames at full quality on the reference GPU; on CPU only, at half quality |
| Scrub latency (proxy or intra media) | ≤ 50 ms to a new frame |
| Import 1000 files to listed in the bin | ≤ 3 s (analysis continues in the background) |
| Element generation, 128 px | About 1 ms a frame on one core (upstream's measured figure, `NeuralVFX:README.md`; not measured here) |
| Memory | ≤ 2 GiB for the app and helpers with a 1080p project, plus the frame cache cap |
| Idle | 0 frames, 0 wake-ups when stopped |
| Longest sequence | 24 hours at 120 fps without time drift (integer frames) |

## 11. Testing and proof

Nothing below has run. All are plans and targets.

- **Unit (ForgeScript):** every edit operation (insert, overwrite, ripple, roll, slip, slide, blade, close gap) against a model with invariants: no overlap on a track, links stay in sync, durations positive, sources within media. Journal replay equals the saved state.
- **Property tests:** random sequences of edits followed by full undo return the original sequence byte for byte.
- **Golden renders:** each dock and dialog; engine goldens per effect, transition, blend mode and colour transform, CPU and GPU within tolerance.
- **Scripted UI through the a11y tree:** the 14 top tasks.
- **Fuzz:** every demuxer and decoder entry in `media-io`; caption parsers; LUT; OTIO, EDL and FCPXML importers; project JSON; the helper IPC decoders.
- **Differential:** decode against reference decoders on public conformance streams; colour transforms against published reference values; loudness against the EBU R 128 test set; element frames against `libnvfx` run directly (bit-exact, as upstream states for worker counts).
- **A/V sync:** a clap-board test file exported and re-imported must keep sync within one audio sample per hour.
- **Soak:** a 2-hour export with injected helper crashes resumes and gives a valid file.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Time arithmetic (frames, rationals, timecode, drop-frame) | No overflow; timecode ↔ frame is a bijection | T1 | PRISM proof | — |
| Edit operations | Track invariants hold after every operation | T3 | Property tests | No ForgeScript prover yet; a C++ core could reach T1 |
| Journal | Replay is deterministic | T3 | Tests | Same |
| Helper IPC decoders | Reject malformed input, no runtime errors | T2 | PRISM plus fuzzing | — |
| Own Matroska and WAV parsers | No out-of-bounds read | T2 | PRISM plus fuzzing | — |
| Third-party codecs and FFmpeg | Contained | T4 | Sandbox tests | Not in the safe subset |
| Render graph scheduler | No deadlock; each frame evaluated once | T3 | Model checking of the scheduler design, tests | Concurrency proof effort |
| `libnvfx` wrapper | Buffer sizes match the API contract | T2 | PRISM on the wrapper | `libnvfx` itself is outside PRISM's subset until checked (Unverified) |
| GPU kernels | Match the CPU reference | T3 | Goldens | Driver outside the boundary |

## 12. Build plan

### Phase 1 — usable core (cut and export)

| ID | Work item | Depends on |
|---|---|---|
| CR02-W01 | Project and sequence model, time arithmetic, edit operations, journal | G-02 |
| CR02-W02 | `media-io` helper: demux and decode of the phase-1 codec set, thumbnails, waveforms | G-11 (FUSE-WP-53), FORGE-WP-27, FORGE-WP-28, G-06 |
| CR02-W03 | `render-engine` v1: cuts, transforms, opacity, dissolves, CPU path | W01, W02 |
| CR02-W04 | `audio-engine` v1: mix, gain, pan, fades, monitoring | G-11 (FUSE-WP-52), CR03 shared code |
| CR02-W05 | Workbench, bin, viewers, inspector | G-01, G-07, G-08, FUSE-WP-31 |
| CR02-W06 | Timeline UI on the upgraded widget: tracks, clips, snap, ripple and roll | FUSE-WP-50, W01 |
| CR02-W07 | `encode` helper: AV1/VP9 + Opus in Matroska/WebM, FFV1 + FLAC, H.264 + AAC in MP4; render queue | CR02-GAP-4, W03 |
| CR02-W08 | Autosave, recovery, undo history, lock file | W01 |
| CR02-W09 | Relink, hashes, import report | W02 |
| CR02-W10 | Markers, multiple sequences, constant speed | W06 |

### Phase 2 — full feature

| ID | Work item | Depends on |
|---|---|---|
| CR02-W11 | Slip, slide, trim view, keyboard trim | W06 |
| CR02-W12 | Proxies, render cache, interpret footage, collect project | W02, W07 |
| CR02-W13 | Effect stack, built-in effects, keying, masks; GPU kernels | W03, FUSE-WP-24, CR02-GAP-3 |
| CR02-W14 | Keyframes, lanes, curve editor | FUSE-WP-50 |
| CR02-W15 | Titles, templates, shapes | G-03, FUSE-WP-18 |
| CR02-W16 | Colour management, wheels, curves, LUTs, scopes | CR02-GAP-1, CR02-GAP-2, FUSE-WP-26 |
| CR02-W17 | Mixer, buses, automation, effects, loudness, waveform sync, voice-over | W04, FUSE-WP-49 |
| CR02-W18 | Multicam | W06, W17 |
| CR02-W19 | Speed ramps, frame blending, classic optical flow, stabilisation | W13 |
| CR02-W20 | Captions: tracks, editor, import/export, burn-in | W15 |
| CR02-W21 | Transcription through `svc.model` | G-15, FORGE-WP-29, FORGE-WP-31, AI02 |
| CR02-W22 | `element-gen` helper: build `libnvfx` on HardenedBSD offline, pack loading, element clips and controls | FORGE-WP-27, NeuralVFX port (Unverified) |
| CR02-W23 | Export presets editor, rate control, stems, regions | W07 |
| CR02-W24 | Untrusted-project mode, signed expressions | FORGE-WP-33 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CR02-W25 | Node graph compositing | FUSE-WP-50, W13 |
| CR02-W26 | Tracking, planar tracking, rotoscoping | W13 |
| CR02-W27 | Secondaries, shot match, HDR grading and export | W16, FUSE-WP-11, FUSE-WP-54 |
| CR02-W28 | Model tools: upscale, denoise, interpolation, matting, relighting | W21, models from AI02 |
| CR02-W29 | Scene script clips, element passes, weather and volume elements, continue from footage | W22, W25 |
| CR02-W30 | Interchange: OTIO, EDL, FCPXML | W01 |
| CR02-W31 | Sequence compare and merge, review export | W01, DV03 |
| CR02-W32 | Hardware encode, smart render | G-11, W07 |
| CR02-W33 | CR03 and CR01 round trips, signed export | CR03, CR01, SE04 |

## 13. Open questions and risks

1. **NeuralVFX is not an editing engine.** The catalogue line "NeuralVFX engine" overstates what the repo has. This spec uses it for generated elements only. The owner should confirm that reading, or say what else was intended.
2. **The media stack is the largest risk.** G-11 as written covers playback. An editor also needs frame-accurate random access, several decoders at once and encode (CR02-GAP-4). FFmpeg is the practical route and is C code outside the safe subset; containment is by sandbox only (T4).
3. **Codec patents and licences** (H.264, HEVC, AAC) vary by country. Defaults are royalty-free formats; the others are optional helpers. Owner decision on what ships.
4. **GPU on HardenedBSD.** Vulkan compute availability, hardware decode and encode are Unverified. The CPU path must be good enough to edit 1080p with proxies.
5. **Timeline widget.** Today's `timeline` stores `f32` seconds inside the widget. FUSE-WP-50 must land before any real timeline UI; this app is its main customer.
6. **NeuralVFX on HardenedBSD.** Upstream builds with g++-14 on Ubuntu and uses per-ISA SIMD. A clang/libc++ build is Unverified. Sprites are small (64–256 px); enlargement for video frames may look soft. Scenes cost more (upstream reports tens of milliseconds a picture for some), so they render to cache.
7. **Models for 4.13** do not exist in the suite yet. Each needs a licence that allows shipping. Until then those rows stay disabled.
8. **Real-time audio** shares the OS requirements stated in CR03 (real-time scheduling for the audio helper).
9. **Project merge.** JSON diff works for simple changes; ripple edits touch many clips. A semantic merge (feature F138) may be needed sooner than phase 3.
10. **Cross-process GPU images** (CR02-GAP-3): without them every frame is copied through shared memory, which costs about 8 MiB a frame at 1080p 8-bit and more at 4K float.
