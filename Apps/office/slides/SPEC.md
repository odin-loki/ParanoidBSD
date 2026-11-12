<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Slides

By Odin Loch

| | |
|---|---|
| ID | `OF06` (from CATALOGUE.md) |
| Bundle | `org.pbsd.slides` |
| Category | office |
| Priority | P2 |
| Backing repos | FUSE (UI, `timeline`, `videoWidget`, `chart`), FORGE (ForgeScript, signing) |
| Status | Planned. Nothing built. |

## 1. Purpose

The presentation program for the suite. People build slide decks, rehearse them, and present them on a second screen with notes and a timer. It saves ODP natively, reads and writes PPTX, exports PDF, images and video, and never plays a file's embedded code or fetches anything from the network when a deck opens. It replaces LibreOffice Impress from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Slides with text, shapes, images, tables, charts, audio and video.
- Slide masters, layouts, themes and placeholders.
- Animations of objects and transitions between slides.
- Presenter view on a second output, rehearsal with timings, audience tools (pointer, pen, zoom, blank).
- Outline view, slide sorter, notes view, handout layout.
- ODP native; PPTX import and export; export to PDF, PNG, SVG, video, and a self-contained image handout.
- Review by comments stored in the file.
- Remote control from another device on the local network, by capability, paired and encrypted.
- An accessibility checker.

Out of scope:

| Item | Owner |
|---|---|
| Full vector editing: bezier node tools, boolean operations, stencil libraries, connectors with auto-layout | OF08 Draw (embedded as an object; Slides has the basic shape and connector set) |
| Chart data editing beyond a small data sheet | OF05 Sheets (embedded chart objects) |
| Raster image editing beyond crop, mask, brightness and contrast | CR01 Image Editor |
| Video and audio editing beyond trim and poster frame | CR02 Video Editor, CR03 Audio Editor |
| Screen recording of a talk with a camera overlay | ME05 Screen Capture |
| Long-form documents | OF04 Writer |
| Real-time co-editing; cloud presenting; streaming a talk to remote viewers | not planned |
| Casting to network displays | ME03 Video owns casting; Slides presents on local outputs only |

## 3. Users and top tasks

Personas:

- **Speaker**: builds a deck, rehearses, presents with notes on the laptop and slides on the projector.
- **Teacher**: reuses decks, prints handouts, annotates during class.
- **Designer of templates**: builds masters, layouts and themes for an organisation.
- **Reviewer**: reads a deck, leaves comments.
- **Recipient of an untrusted deck**: opens a PPTX from mail safely.

| # | Task | Target steps |
|---|---|---|
| T1 | New deck from a theme and type the title | 3 |
| T2 | Add a slide with a chosen layout | 1 (`Ctrl+M`), 2 with layout choice |
| T3 | Insert an image and have it fit a placeholder | 2 (drop on placeholder) |
| T4 | Reorder slides | 1 drag, or `Ctrl+Shift+Up/Down` |
| T5 | Start presenting from the current slide with presenter view | 1 (`Shift+F5`) |
| T6 | Change the theme colours of the whole deck | 3 |
| T7 | Make bullets appear one by one | 3 |
| T8 | Export to PDF with notes | 4 |
| T9 | Rehearse and record timings | 2 to start |
| T10 | Write the whole deck as an outline, then design | outline view, 0 extra steps per slide |
| T11 | Pair a phone-sized device as a remote | 4 |
| T12 | Run the accessibility check and fix reading order | 3 |
| T13 | Open a PPTX, edit, save back as PPTX with a loss report | 3 |
| T14 | Align and distribute five shapes | 2 |

## 4. Features

### 4.1 Deck and slide management

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F001 | Slide sizes | 16:9 (33.867×19.05 cm), 16:10, 4:3, A4, Letter, banner, custom up to 142 cm per side; portrait or landscape; resize with scale-or-keep-size choice. | M | 1 |
| OF06-F002 | New, duplicate, delete slides | `Ctrl+M` new after current with the same layout family, `Ctrl+D` duplicate, `Delete` in the slide list with Undo toast. | M | 1 |
| OF06-F003 | Reorder | Drag in the slide list or sorter; `Ctrl+Shift+Up/Down`; move to start or end. | M | 1 |
| OF06-F004 | Sections | Named, collapsible groups of slides; move, rename, delete a section; exported as PPTX sections and as an ODF extension. | S | 2 |
| OF06-F005 | Hide slide | Hidden slides are skipped in the show; shown struck through; reachable by number during the show. | M | 1 |
| OF06-F006 | Slide numbering, date, footer fields | Per master and per slide; start number; hide on title slide. | M | 1 |
| OF06-F007 | Import slides from another deck | Pick slides from a file in a sorter dialog; keep source formatting or adopt the target theme. | S | 2 |
| OF06-F008 | Summary and agenda slide | Generate a slide of titles from selected slides or sections, with internal links. | C | 3 |
| OF06-F009 | Custom shows | Named subsets and orders of slides for different audiences. | S | 2 |
| OF06-F010 | Slide list panel | Thumbnails with number, hidden mark, transition mark, comment count, section headers; three thumbnail sizes. | M | 1 |
| OF06-F011 | Zoom and fit | 10–800%, fit slide, fit width, zoom to selection. | M | 1 |
| OF06-F012 | Multiple windows and tabs | Several decks as tabs; two windows on one deck. | S | 2 |
| OF06-F013 | Templates | New from template; save as template (`.otp`); template gallery with built-in and user templates. | M | 1 |
| OF06-F014 | Deck-wide find and replace | Text in slides, notes, comments, alt text; regex; replace fonts across the deck. | M | 2 |

### 4.2 Masters, layouts and themes

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F015 | Slide masters | Several masters per deck; each holds background, placeholders' default style, fixed objects, footer fields. Master view edits them. | M | 1 |
| OF06-F016 | Layouts | Per master: title, title and content, section header, two content, comparison, title only, blank, content with caption, picture with caption, quote, big number; user layouts. | M | 1 |
| OF06-F017 | Placeholders | Title, subtitle, body, picture, chart, table, media, object, date, footer, slide number. A placeholder keeps its link to the layout until the user moves or restyles it; "Reset to layout" restores. | M | 1 |
| OF06-F018 | Themes | Colour scheme (12 slots), font pair (heading, body, with per-script fallbacks), effect defaults, background styles. Shared theme files with OF04 and OF05. | M | 1 |
| OF06-F019 | Theme variants | Light and dark variants of one theme; switch per deck or per slide. | S | 2 |
| OF06-F020 | Apply master or layout | To selected slides or all; preview on hover in the picker. | M | 1 |
| OF06-F021 | Backgrounds | Solid, gradient (linear, radial, multi-stop), image (fit, fill, tile), pattern; per slide override; hide master objects. | M | 1 |
| OF06-F022 | Notes master and handout master | Layout of the notes page and of handouts (1, 2, 3, 4, 6, 9 slides per page, with note lines). | S | 2 |
| OF06-F023 | Theme editor | Edit colours with live contrast ratios against backgrounds; warn under 4.5:1. | S | 2 |
| OF06-F024 | Master clean-up | List unused masters and layouts; remove them; merge duplicates after imports. | S | 3 |

### 4.3 Text

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F025 | Text boxes and shape text | Any shape holds text; text boxes auto-grow, shrink text on overflow, or keep fixed size. | M | 1 |
| OF06-F026 | Character formatting | Font, size, bold, italic, underline styles, strike, colour, highlight, super/subscript, small caps, letter spacing, shadow, outline. | M | 1 |
| OF06-F027 | Paragraph formatting | Alignment, justify, indents, spacing before and after, line spacing, tabs, text direction, columns in a box. | M | 1 |
| OF06-F028 | Lists | Bullets (symbol, image), numbering (1, a, i, A, I), 9 levels, `Tab`/`Shift+Tab` to change level, restart and continue. | M | 1 |
| OF06-F029 | Styles | Presentation styles per placeholder level (ODF presentation styles) and graphic styles; change once, apply everywhere. | M | 2 |
| OF06-F030 | Autocorrect and autofit | Smart quotes, dashes, list detection; each can be turned off; autofit never goes below a minimum size (default 18 pt) without a warning. | S | 2 |
| OF06-F031 | Spell check | Through the dictionary service (G-17); language per run. | M | 2 |
| OF06-F032 | Fields | Slide number, count, date, time, file name, author, section name. | S | 2 |
| OF06-F033 | Links | To a slide, a custom show, a file (portal token) or a URL. URLs open through `svc.intent` only on click, after showing the target. No "run program" or "run macro" actions; such actions in imported files are removed and reported. | M | 2 |
| OF06-F034 | Mathematics | Insert a formula typed in a LaTeX subset or picked from a palette; rendered as vector text; stored as MathML in ODF. Uses the formula object shared with OF04. | S | 3 |
| OF06-F035 | Code blocks | Monospace box with syntax colours from the OF01 grammar files; line highlights that can be animation steps. | C | 3 |
| OF06-F036 | Text import | Paste as plain text, keep source formatting, or match destination; paste Markdown as a slide structure. | S | 2 |
| OF06-F037 | Word art and text on path | Text following a path; fill and outline effects. | C | 3 |

### 4.4 Shapes, images and other objects

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F038 | Basic shapes | Rectangle, rounded rectangle, ellipse, triangle, polygons, stars, arrows, callouts, brackets, flowchart set, lines, curves, freeform, with adjust handles. Geometry from the vector core shared with OF08. | M | 1 |
| OF06-F039 | Connectors | Straight, elbow, curved; glue to shape connection points; reroute on move. | S | 2 |
| OF06-F040 | Fill, stroke and effects | Solid, gradient, pattern, image fill; stroke width, dash, caps, joins, arrowheads; shadow, soft edge, glow, reflection; opacity. | M | 1 |
| OF06-F041 | Images | PNG, JPEG, WebP, GIF (animated), SVG, AVIF (if the platform image pipeline has it). Crop, crop to shape, mask, rotate, flip, brightness, contrast, saturation, recolour to theme, transparency colour. | M | 1 |
| OF06-F042 | Image handling | Images are embedded, never linked to a URL. Downscale on insert to a chosen pixel density (default 220 ppi at slide size) with the original kept until "Compress images". EXIF location and camera data stripped on insert by default. | M | 1 |
| OF06-F043 | Compress images | Deck-wide tool: target ppi, drop cropped areas, convert to a smaller format; shows saved bytes. | S | 2 |
| OF06-F044 | Tables | Insert n×m; merge and split; row and column insert, delete, distribute; table styles with header, total, banding; cell margins; tab between cells. | M | 1 |
| OF06-F045 | Charts | Insert a chart with a small data sheet (`tableView`), or embed or link a chart from OF05. Rendering uses the same chart code as OF05. | M | 2 |
| OF06-F046 | Audio and video | Insert MP4/WebM/Matroska/Ogg/FLAC/WAV/MP3/Opus files (as the platform media pipeline supports, G-11); poster frame; trim in and out; start on click or automatically; loop; volume; play across slides (audio). Decoding runs in the platform's media helper, not in the app. | S | 2 |
| OF06-F047 | Embedded objects | A drawing from OF08, a range from OF05, a formula; each stored as an ODF object with an SVG fallback picture; edit through the `edit` intent. OLE objects from PPTX are kept as opaque bytes with their preview image and never activated. | S | 3 |
| OF06-F048 | Icons and symbols | Built-in icon set (SVG subset) searchable by name; special character picker (hands off to AX04's picker). | S | 2 |
| OF06-F049 | Screenshot insert | Ask ME05 through the `pick` intent for a region capture; arrives as an image. | C | 3 |
| OF06-F050 | Alt text | Every non-text object has a title, a description, and a "decorative" flag. Optional draft description through `svc.model` when `ai.local` is granted; the user must confirm it. | M | 1 |
| OF06-F051 | QR code object | Generate a QR code from text as a vector object, offline. | C | 3 |
| OF06-F052 | Live objects | A clock or timer object that runs during the show; a slide-progress bar. No web content objects. | C | 3 |

### 4.5 Arrange and layout aids

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F053 | Select, move, resize, rotate | Rubber band, `Shift` add, `Tab` cycle; arrow nudge 1 px, `Shift` 10 px, `Alt` 0.1 mm; resize from centre with `Ctrl`; keep ratio with `Shift`; rotate in 15° steps with `Shift`. | M | 1 |
| OF06-F054 | Smart guides | Edges, centres and equal spacing against other objects and the slide; distance labels while dragging. | M | 1 |
| OF06-F055 | Grid, rulers, guides | Grid with spacing and subdivisions; rulers in cm, mm, in, pt, px; draggable guides per slide and per master; lock guides. | M | 1 |
| OF06-F056 | Snap | To grid, guides, object edges, margins; hold `Alt` to suspend. | M | 1 |
| OF06-F057 | Align and distribute | Left, centre, right, top, middle, bottom; relative to selection, slide or margins; distribute horizontally and vertically; equal width and height. | M | 1 |
| OF06-F058 | Order | Bring to front, forward, backward, to back; in front of or behind a picked object. | M | 1 |
| OF06-F059 | Group | Group, ungroup, enter group to edit, regroup. | M | 1 |
| OF06-F060 | Lock and hide objects | Lock position, lock from selection, hide in the editor. | S | 2 |
| OF06-F061 | Selection pane | List of all objects on the slide in z-order with names, eye and lock toggles, drag to reorder; this list is also the reading order. | M | 2 |
| OF06-F062 | Position and size inspector | X, Y, width, height, rotation, anchor point, with expressions (`w/2`, `+10mm`). | M | 1 |
| OF06-F063 | Format painter and style copy | Copy style to one or many objects; `Ctrl+Shift+C` / `Ctrl+Shift+V`. | S | 1 |
| OF06-F064 | Replace object keeping geometry | Swap an image or shape type without losing size, position and animation. | S | 2 |
| OF06-F065 | Layout suggestions | Rule-based: given the objects on a slide, offer 3–6 tidy arrangements from the layout set. No model required. | C | 3 |

### 4.6 Animations and transitions

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F066 | Effect classes | Entrance, emphasis, exit, motion path, media commands (play, pause, stop). | M | 2 |
| OF06-F067 | Effect set | Appear, fade, fly in, wipe, zoom, split, wheel, grow and shrink, spin, colour change, transparency, pulse, and their exits. About 40 effects, each defined as SMIL in ODF and mapped to PPTX timing nodes. | M | 2 |
| OF06-F068 | Triggers and timing | On click, with previous, after previous; delay, duration, repeat, rewind; trigger on click of a named object. | M | 2 |
| OF06-F069 | Text animation | By paragraph level, by word, by letter; in reverse order; dim after animation. | M | 2 |
| OF06-F070 | Motion paths | Lines, arcs, curves, custom drawn path; edit points; auto-rotate along path. | S | 3 |
| OF06-F071 | Animation panel | Ordered list with a time bar per effect (`timeline` widget), drag to reorder and retime, play from selected. | M | 2 |
| OF06-F072 | Transitions | None, fade, push, wipe, cover, uncover, zoom, morph-style object match (objects with the same name tween position, size, colour), dissolve; direction, duration 0.1–5 s; advance on click or after n seconds. | M | 2 |
| OF06-F073 | Reduced motion | When the system setting is on, the show replaces every effect with an instant or 80 ms fade, and the editor previews honour it. A deck option "calm deck" does the same for all audiences. | M | 2 |
| OF06-F074 | Animation painter | Copy an object's animations to another. | C | 3 |
| OF06-F075 | Deterministic playback | Effects are functions of time, not of frame count; export to video and live playback give the same frames at the same timestamps. | M | 2 |
| OF06-F076 | Flash-safety check | Warns when an effect or transition sequence could exceed three flashes per second. | S | 3 |

### 4.7 Slide show and presenter view

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F077 | Start show | From the first slide `F5`, from the current slide `Shift+F5`, a custom show, or in a window. | M | 1 |
| OF06-F078 | Output selection | Pick which output shows the audience view and which shows presenter view; remembered per output set; swap with one key (`S`); works with one output (presenter view in a window, or plain show). | M | 1 |
| OF06-F079 | Presenter view | Current slide, next slide or next animation step, notes with adjustable size, elapsed and remaining time, wall clock, slide n of m, thumbnail strip. | M | 1 |
| OF06-F080 | Timers | Count up, count down to a target length or a target end time; colour and word change at 5 min and 1 min remaining; pause and reset. | M | 1 |
| OF06-F081 | Navigation in show | Next `Right/Space/PageDown/N`, previous `Left/PageUp/P`, first `Home`, last `End`, number then `Enter` jumps, `G` opens the slide grid, `B` black, `W` white, `Esc` ends. Presenter remotes that send PageUp/PageDown, `B`, `F5`, `Esc` work without setup. | M | 1 |
| OF06-F082 | Pointer tools | Laser pointer, spotlight, magnifier, pen and highlighter in several colours, eraser; drawings can be kept as slide annotations or discarded at the end. | S | 2 |
| OF06-F083 | Zoom in show | Zoom into a region and pan. | C | 3 |
| OF06-F084 | Notes editing in show | Notes can be edited in presenter view; changes are saved to the deck. | S | 2 |
| OF06-F085 | Sleep and notification control | The show inhibits display sleep through `svc.power` and asks `svc.notify` for do-not-disturb for its duration; both are released on exit or crash. | M | 1 |
| OF06-F086 | Kiosk and loop mode | Loop until `Esc`; auto-advance timings; optional restart after idle; keyboard limited to navigation. | S | 2 |
| OF06-F087 | Rehearsal | Rehearse with a timer per slide; record timings; show the difference from the target; store timings for auto-advance. | M | 2 |
| OF06-F088 | Narration recording | Record microphone audio per slide during rehearsal (through `dev.mic`, with the system indicator); stored as audio objects; used by video export. | C | 3 |
| OF06-F089 | Rehearsal report | Time per slide, words of notes per slide, pace estimate, slides over a threshold. Local only. | C | 3 |
| OF06-F090 | Captions from notes | Show the notes of the current slide as captions on the audience view (for accessibility), with size and position options. | C | 3 |
| OF06-F091 | Screen privacy during show | Presenter view is marked as sensitive so screen capture and remote desktop show only the audience output unless the user allows otherwise. | S | 2 |
| OF06-F092 | Crash-safe show | The show runs in the app host; if it restarts, the show resumes at the same slide and step within 2 s using the session token. | S | 2 |

### 4.8 Views

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F093 | Normal view | Slide list, canvas, notes strip, inspector. | M | 1 |
| OF06-F094 | Outline view | The whole deck as an indented text outline (`Enter` new item, `Tab` demote, `Shift+Tab` promote, promote to top level makes a new slide); collapse to titles; drag to reorder; import and export as Markdown or plain text. | M | 1 |
| OF06-F095 | Slide sorter | Grid of thumbnails; multi-select; drag reorder; transitions and timings shown; section bands; zoom slider. | M | 1 |
| OF06-F096 | Notes view | One slide and its notes page per screen, with rich text notes. | M | 2 |
| OF06-F097 | Master view | Edits masters and layouts; a clear banner says the user is editing a master. | M | 1 |
| OF06-F098 | Handout view | Lays out handout pages for print. | S | 2 |
| OF06-F099 | Reading view | The show in a window with a scroll bar, for reviewers. | S | 2 |
| OF06-F100 | Greyscale and print preview modes | Preview how the deck prints in greyscale or black and white. | C | 3 |

### 4.9 Import, export and print

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F101 | ODP native | ODF 1.3 presentation (`.odp`), template (`.otp`), flat (`.fodp`). Package signatures verified; encryption read and written. | M | 1 |
| OF06-F102 | PPTX import and export | PresentationML (ISO/IEC 29500): `.pptx .ppsx .potx .pptm` (macros never run). Masters, layouts, themes, text, shapes (preset and custom geometry), images, tables, charts, media, notes, comments, transitions, animations, sections, custom shows. | M | 1 |
| OF06-F103 | Compatibility report | Before PPTX export and after PPTX import: what was approximated or dropped (SmartArt kept as grouped shapes; 3D models as their preview image; unsupported effects replaced by fade), with a link to each slide. | M | 1 |
| OF06-F104 | Font handling on import | Missing fonts are listed with the substitute chosen; metric-compatible substitutes preferred; "replace fonts" tool. Embedded fonts in a file are parsed only in the font sandbox of the platform and only if the user allows it per file. | M | 2 |
| OF06-F105 | Legacy PPT | `.ppt` binary import, import helper only. | C | 3 |
| OF06-F106 | PDF export | Slides, slides with notes, handouts, outline; one page per slide or one per animation step; tagged PDF with reading order and alt text; PDF/A-2b option; embedded fonts subset; links kept; image quality choice. | M | 1 |
| OF06-F107 | Image export | PNG, JPEG, SVG per slide or for a selection; resolution and transparent background options. | M | 2 |
| OF06-F108 | Video export | Renders the show with timings, animations, narration and media audio to a video file. Frames come from the same renderer as the live show; encoding uses the platform media pipeline (G-11). Formats depend on what that pipeline ships (AV1 or VP9 in WebM or Matroska expected; Unverified). Runs as a background job with progress in `svc.notify`. | S | 3 |
| OF06-F109 | Animated image export | Short loops as animated PNG or GIF for one slide. | C | 3 |
| OF06-F110 | Outline and notes export | Markdown, plain text, or hand-off of an outline document to OF04. | S | 2 |
| OF06-F111 | Markdown deck import | A Markdown file with `---` slide breaks becomes slides (title from the first heading, lists to body, fenced code to code blocks, images embedded if a handle is granted). | S | 3 |
| OF06-F112 | Print | Slides, notes pages, handouts, outline; colour, greyscale, black and white; frame slides; through `svc.portal.print` (G-16). | M | 2 |
| OF06-F113 | Package for another machine | Save a copy with fonts' names listed, media embedded, and a PDF fallback beside it, in one folder or archive. | S | 3 |
| OF06-F114 | Round-trip preservation | Unknown PPTX parts are carried through as opaque blobs on save to PPTX and listed in the report. | S | 2 |

### 4.10 Review and collaboration by file

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F115 | Comments | Anchored to a slide, an object or a text range; threads with replies; resolve; author name and initials from settings (not from the system account unless the user sets it); comments panel with filters. | M | 2 |
| OF06-F116 | Comment exchange | Comments live in the file (ODF annotations; PPTX modern comments). Export and import a comments-only sidecar file so reviewers can return small files. | S | 3 |
| OF06-F117 | Compare and merge decks | Slide-level and object-level diff of two decks; take slides or changes from either side. | S | 3 |
| OF06-F118 | Local versions | A version on each explicit save, browsable with thumbnails; restore a slide or the whole deck. | S | 2 |
| OF06-F119 | Share | `share` intent with a choice of ODP, PPTX, PDF; a pre-share check lists notes, comments, hidden slides, off-slide objects and metadata, each removable in the copy. | M | 2 |

### 4.11 Remote control

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F120 | Remote control service | Off by default. When the user turns it on for a show, a helper listens on one TCP port on the chosen local interface only. The `[net]` badge shows and lists the address and paired devices. Stops when the show ends. | S | 3 |
| OF06-F121 | Pairing | The presenting machine shows a QR code and a 6-digit code holding its address, port and a one-time secret. The remote and the host run a password-authenticated key exchange and keep a long-term key if the user ticks "remember this device". | S | 3 |
| OF06-F122 | Transport | Noise-pattern encrypted channel over TCP; no TLS certificates, no discovery broadcast by default (optional mDNS announce needs a platform multicast permission; see OF06-GAP-3). | S | 3 |
| OF06-F123 | Remote client | Slides itself on a second PBSD device runs "Remote mode": next, previous, jump, black, timer, notes, next-slide thumbnail, laser pointer by touchpad. | S | 3 |
| OF06-F124 | What the remote may do | Navigation and pointer commands, and read notes and thumbnails of the deck being shown. It cannot open files, edit, or read other decks. The command set is a fixed enum; anything else closes the channel. | M | 3 |
| OF06-F125 | Paired device list | Names, last used, revoke; stored in `svc.keyring`. Every connection is logged to `svc.audit`. | S | 3 |

### 4.12 Accessibility checker, safety and recovery

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF06-F126 | Accessibility checker | Rules: missing alt text, missing slide title, duplicate titles, reading order not set, contrast under 4.5:1 (3:1 for large text), text under 18 pt, tables without header row, merged cells, media without captions, colour-only meaning hints, link text that is a raw URL, flashing risk, empty placeholders, language not set. Results panel with "go to" and one-click fixes where safe. | M | 2 |
| OF06-F127 | Reading order editor | Drag objects in the selection pane to set the order used by screen readers and tagged PDF. | M | 2 |
| OF06-F128 | Trust bar on open | For files from outside: origin, macros present (never run), action links removed, external media links cut, embedded fonts not loaded, OLE objects inert. | M | 1 |
| OF06-F129 | Disposable open | Open a deck in a throw-away instance with no remembered grants and no network. | S | 2 |
| OF06-F130 | Metadata tools | Properties panel; strip author, revision, template path, thumbnails on save. | M | 2 |
| OF06-F131 | Autosave and recovery | Command journal flushed every 2 s; media stored once by content hash; recovery reopens at the last command. | M | 1 |
| OF06-F132 | Undo | Unlimited per session; one step per user action; covers master edits and animation changes. | M | 1 |
| OF06-F133 | Signatures | Verify ODF and OOXML signatures; sign with keys from SE04. | S | 3 |
| OF06-F134 | Search integration | Titles, body text and notes offered to `svc.search`. | S | 2 |
| OF06-F135 | Thumbnailer | First slide rendered for `svc.thumbnail` inside the I/O helper. | S | 2 |
| OF06-F136 | Command palette, shortcuts, help | Every action in the palette; rebindable keys; help pages under `help/`. | M | 1 |
| OF06-F137 | Scripted decks | A signed ForgeScript module can generate or update slides from data through a narrow deck API (no file, network or process access). For report automation. | C | 3 |

## 5. GUI

Layout pattern: document canvas + inspector. Default density: normal.

### 5.1 Window map

```
Slides
├─ Deck window (tabs for several decks)
│  ├─ Header bar: menu, title, search / command palette, trust badge, [net] badge (remote on)
│  ├─ Toolbar: insert group, text group, arrange group, view switch, [Present v]
│  ├─ Sidebar (F9): Slide list | Outline | Comments | Versions
│  ├─ Canvas (rulers, guides, slide, pasteboard)
│  ├─ Notes strip (resizable, under the canvas)
│  ├─ Inspector (F4): Slide | Layout | Text | Shape | Image | Table | Chart | Media |
│  │                  Animation | Transition | Arrange | Selection pane | Accessibility
│  └─ Status bar
├─ Views in the same window: Normal · Outline · Sorter · Notes · Master · Handout · Reading
├─ Show windows
│  ├─ Audience window (full screen on the chosen output)
│  ├─ Presenter view window
│  └─ Slide grid overlay, pointer tool bar, timer popover
├─ Remote mode window (when this device is the remote)
├─ Dialogs
│  ├─ New deck (theme and size)        ├─ Export PDF
│  ├─ Slide size                       ├─ Export images / video
│  ├─ Theme editor                     ├─ Compatibility report
│  ├─ Insert table / chart / media     ├─ Compress images
│  ├─ Import slides                    ├─ Custom shows
│  ├─ Show settings (outputs, timers)  ├─ Pair remote
│  ├─ Rehearsal summary                ├─ Compare decks
│  ├─ Find and replace                 ├─ Properties and metadata
│  ├─ Print                            └─ Preferences
├─ Popovers: layout picker, shape gallery, colour, line style, alignment, transition gallery,
│            effect gallery, zoom, comment bubble, font list
└─ Inline bars: trust bar, master-edit banner, missing fonts bar, read-only bar
```

### 5.2 Main window

Shown at 1360×820, wide layout.

```
+------------------------------------------------------------------------------------------+
| [=] Slides — quarterly-review.odp        [search  Ctrl+K]        (1)[ok]   [▁] [▢] [✕]   |
+------------------------------------------------------------------------------------------+
| (2) [+Slide v][Layout v] | [Text][Shape v][Image][Table][Chart][Media] | [Align v][Order v]|
|     [Group] | View: [Normal][Outline][Sorter][Notes] |                 (3) [▶ Present v] |
+-------------+-------------------------------------------------------+--------------------+
| (4) Slides  | (5)  0    5    10    15    20    25    30  cm         | (8) Inspector      |
| ┌─────────┐ |    +---------------------------------------------+   | [Shape][Anim][Arr] |
| │1 Title  │ |  0 |                                             |   |                    |
| └─────────┘ |    |   Quarterly review                          |   | Fill               |
| ┌─────────┐ |  5 |   ─────────────────                         |   | (•) Solid [■ Acc1 v]|
|>│2 Agenda │ |    |   • Results           +-----------------+   |   | ( ) Gradient       |
| └─────────┘ | 10 |   • Risks             |  (6) image      |   |   | Opacity [100%]     |
| ┌─────────┐ |    |   • Plan              |   selected ▣    |   |   |                    |
| │3 Results│ | 15 |                       +--------o--------+   |   | Line               |
| └─────────┘ |    |                                             |   | Width [1.5 pt] [— v]|
|  ▸ Risks    |    +---------------------------------------------+   |                    |
| ┌─────────┐ |                                                       | Size and position  |
| │4 …  (c2)│ +-------------------------------------------------------+ X [18.2] Y [8.4] cm|
| └─────────┘ | (7) Notes: Start with the headline number. Pause.     | W [12.0] H [6.8] cm|
|             |     Mention the two delayed items before questions.   | Rot [0°] [x] Ratio |
+-------------+-------------------------------------------------------+--------------------+
| (9) Slide 2 of 14 · Section: Intro · English (AU)    a11y: 3 issues    [-] 68% [+] [Fit] |
+------------------------------------------------------------------------------------------+
```

1. Trust badge; `[net]` badge appears beside it only while the remote control helper listens.
2. Toolbar, one or two rows; groups collapse into menus when narrow.
3. Present split button: from start, from current, presenter view on or off, rehearse, show settings.
4. Slide list, 180 px, `gridView` in one column with section headers; `(c2)` marks two open comments.
5. Canvas with rulers. The slide sits on a pasteboard; objects off the slide are kept and flagged by the pre-share check.
6. Selection with eight handles, a rotate handle, and smart-guide lines while dragging.
7. Notes strip, 72 px default, drag to resize, rich text.
8. Inspector tabs follow the selection.
9. Status bar: position, section, text language, accessibility issue count (click opens the checker), zoom.

States: empty (start view with themes and recent decks), loading (thumbnails fill progressively), error (inline bar), offline (no difference), locked (password sheet), read-only (bar; show still runs).

### 5.3 Other views and dialogs

**Presenter view** (on the presenter's output, 1280×720 shown).

```
+------------------------------------------------------------------------------------------+
| (1) 00:07:42 elapsed   ▮▮▮▮▮▮▯▯▯▯ 12:18 left   14:32    [Pause] [Reset]   Slide 5 of 14  |
+---------------------------------------------------+--------------------------------------+
| (2) Current                                       | (3) Next: step 2 of 3                |
|  +---------------------------------------------+  |  +--------------------------------+  |
|  |                                             |  |  |                                |  |
|  |      Results                                |  |  |   Results (second bullet)      |  |
|  |      • Revenue up 8%                        |  |  |                                |  |
|  |                                             |  |  +--------------------------------+  |
|  |                                             |  | (4) Notes                    [A-][A+] |
|  +---------------------------------------------+  |  Lead with revenue. The 8% is after   |
|                                                   |  currency effects. If asked about     |
| (5) [◀ Prev] [Next ▶]  [Grid G] [Black B]         |  churn, point to slide 9.             |
|     [Laser] [Pen v] [Zoom]  [Swap outputs S]      |                                      |
+---------------------------------------------------+--------------------------------------+
| (6) [1][2][3][4][>5][6][7][8][9][10][11][12][13][14]                 remote: 1 paired ●   |
+------------------------------------------------------------------------------------------+
```

1. Timer bar: elapsed, progress against target, wall clock. 2. Current slide as the audience sees it, including the current animation step. 3. Next step or next slide. 4. Notes, scrollable, font size keys `Ctrl++/-`, editable with `E`. 5. Controls; all have keys. 6. Thumbnail strip (`listView`, horizontal). States: single output (presenter view in a window, warning that the audience can see it if mirrored), output lost (show moves to the remaining output and pauses the timer), locked session (show output goes black).

**Slide sorter** (same window).

```
+------------------------------------------------------------------------------------------+
| View: [Normal][Outline][>Sorter][Notes]      Filter [All v]   Size [──o────]   14 slides |
+------------------------------------------------------------------------------------------+
| ▾ Intro                                                                                  |
| ┌────────┐ ┌────────┐ ┌────────┐                                                         |
| │ 1      │ │ 2      │ │ 3      │                                                         |
| │ Title  │ │ Agenda │ │ Results│                                                         |
| └──00:20─┘ └─⇄00:45─┘ └─⇄01:30─┘                                                         |
| ▾ Risks                                                                                  |
| ┌────────┐ ┌╌╌╌╌╌╌╌╌┐ ┌────────┐ ┌────────┐                                              |
| │ 4      │ ╎ 5 hid. ╎ │>6      │ │ 7      │                                              |
| └─⇄02:10─┘ └╌╌╌╌╌╌╌╌┘ └─⇄01:00─┘ └────────┘                                              |
+------------------------------------------------------------------------------------------+
```

`gridView` with section bands; `⇄` marks a transition; the time is the rehearsed timing; dashed border marks a hidden slide.

**Outline view**: left 55% a text outline (rich text widget, G-10) where top-level lines are slide titles; right 45% a live preview of the slide under the caret; toolbar `[Promote][Demote][Move up][Move down][Collapse all][Expand all]`.

```
+---------------------------------------------------+--------------------------------------+
| 1  Quarterly review                               |  +--------------------------------+  |
| 2  Agenda                                         |  |  Agenda                        |  |
|      • Results                                    |  |  • Results                     |  |
|      • Risks|                                     |  |  • Risks                       |  |
|      • Plan                                       |  |  • Plan                        |  |
| 3  Results                                        |  +--------------------------------+  |
|      • Revenue up 8%                              |  Layout [Title and content v]        |
+---------------------------------------------------+--------------------------------------+
```

**Animation inspector tab** (280 px).

```
+-- Animation ---------------------+
| [+ Add effect v]  [▶ Preview]    |
| 1 ⊙ click  Title      Fade   ▮▮  |
| 2 ⊙ click  Bullet 1   Fly in ▮▮▮ |
|   ↳ with   Image 3    Zoom   ▮▮  |
| 3 ⊙ click  Bullet 2   Fly in ▮▮▮ |
|   ↳ after  Chart 1    Wipe    ▮▮▮|
+----------------------------------+
| Effect [Fly in v] From [Left v]  |
| Start [On click v]               |
| Delay [0.0 s] Duration [0.5 s]   |
| Text  [By paragraph v]           |
| After [Don't dim v]              |
| [x] Honour reduced motion        |
+----------------------------------+
```

The list is a `listView` with a small time bar per row; a bottom dock holds the full `timeline` when opened.

**Export PDF** (560×520).

```
+-- Export PDF ----------------------------------------------+
| What     (•) Slides ( ) Slides with notes ( ) Handouts [6 v]|
|          ( ) Outline                                       |
| Range    (•) All ( ) Current ( ) Slides [2-9, 12____]      |
| Steps    [ ] One page per animation step                   |
| Include  [ ] Hidden slides  [ ] Comments  [x] Links        |
| Quality  Images [220 ppi v]  [x] Embed fonts (subset)      |
| Access   [x] Tagged PDF with reading order and alt text    |
|          [ ] PDF/A-2b                                      |
| Privacy  [x] Remove author and software metadata           |
| a11y check: 3 issues  [Review…]                            |
|                                   [Cancel]  [Export 14 pages]|
+------------------------------------------------------------+
```

**Pair remote** (480×420).

```
+-- Pair a remote ----------------------------------------+
| Remote control lets one paired device change slides and |
| read your notes for this show. It listens on:           |
|   Interface [em0 192.168.1.20 v]   Port [47111]         |
|                                                         |
|      ▄▄▄▄▄▄▄  ▄ ▄▄  ▄▄▄▄▄▄▄                             |
|      █ ▄▄▄ █ ▄█▀▄  █ ▄▄▄ █         Code  418 207        |
|      █ ███ █ ▀▄ ▀█ █ ███ █         expires in 01:52     |
|      ▀▀▀▀▀▀▀ ▀ ▀ ▀ ▀▀▀▀▀▀▀                              |
| [ ] Remember this device      Paired: none              |
|                         [Stop listening]  [Close]       |
+---------------------------------------------------------+
```

The capability prompt for listening on the network is drawn by the shell before this dialog appears.

**Accessibility checker tab**: `treeView` grouped by rule with counts; selecting an issue selects the object; buttons `[Fix]` (where a safe fix exists), `[Ignore once]`, `[Go to]`; a summary line with contrast ratios.

**Show settings** (520×440): audience output `comboBox`, presenter output `comboBox`, `[x] Presenter view`, target length, count up or down, `[x] Do not disturb`, `[x] Keep display awake`, pointer defaults, kiosk options, `[x] Calm deck (no motion)`.

**Compatibility report** (640×480): `tableView` with columns Slide, Object, Issue, What happens; filter by severity; `[Go to]`; `[Export anyway]`.

### 5.4 Menus

```
File    New… (Ctrl+N) · New from template… · Open… (Ctrl+O) · Open recent ▸ · Open in disposable sandbox…
        Import slides… · Import outline or Markdown…
        Save (Ctrl+S) · Save as… (Ctrl+Shift+S) · Save as template…
        Export ▸ PPTX… · PDF… · Images… · Video… · Outline… · Package…
        Versions… · Properties… · Check before sharing… · Share… · Print… (Ctrl+P) · Close (Ctrl+W) · Quit (Ctrl+Q)
Edit    Undo (Ctrl+Z) · Redo (Ctrl+Shift+Z) · Cut · Copy · Paste · Paste special… (Ctrl+Shift+V) · Paste as plain text (Ctrl+Alt+V)
        Duplicate (Ctrl+D) · Delete · Select all (Ctrl+A) · Copy style (Ctrl+Shift+C) · Paste style (Ctrl+Alt+Shift+V)
        Find… (Ctrl+F) · Replace… (Ctrl+H) · Replace fonts… · Preferences… (Ctrl+,)
View    Normal (Alt+1) · Outline (Alt+2) · Slide sorter (Alt+3) · Notes (Alt+4) · Master (Alt+5) · Handout (Alt+6) · Reading (Alt+7)
        Sidebar (F9) · Inspector (F4) · Notes strip (Ctrl+Alt+N) · Rulers (Ctrl+Shift+R) · Grid (Ctrl+') · Guides (Ctrl+;)
        Snap ▸ · Zoom ▸ · Greyscale preview · Full screen (F11)
Insert  Slide (Ctrl+M) · Section · Text box (T) · Shape ▸ · Line or connector ▸ · Image… · Table… · Chart… · Audio… · Video…
        Icon… · Symbol… · Formula… · Code block · Drawing (OF08)… · Sheet range (OF05)… · Link… (Ctrl+L)
        Comment (Ctrl+Alt+M) · Slide number · Date · Footer…
Format  Text ▸ Bold (Ctrl+B) · Italic (Ctrl+I) · Underline (Ctrl+U) · Bigger (Ctrl+]) · Smaller (Ctrl+[) · Clear (Ctrl+\)
        Paragraph ▸ · Bullets and numbering… · Styles… · Shape ▸ Fill · Line · Effects · Image ▸ Crop · Mask · Adjust · Compress all…
        Table ▸ · Theme ▸ · Slide background… · Reset to layout
Arrange Bring to front (Ctrl+Shift+]) · Forward (Ctrl+]) · Backward (Ctrl+[) · To back (Ctrl+Shift+[)   (text edit mode gives these keys to text size)
        Align ▸ · Distribute ▸ · Group (Ctrl+G) · Ungroup (Ctrl+Shift+G) · Lock · Rotate ▸ · Flip ▸ · Selection pane (Alt+F10)
Slide   Layout ▸ · Master ▸ · Slide size… · Hide slide (Ctrl+Shift+H) · Move up (Ctrl+Shift+Up) · Move down (Ctrl+Shift+Down)
        Transition… · Animation panel (Alt+A) · Summary slide
Show    From start (F5) · From current (Shift+F5) · In a window · Custom shows… · Rehearse (Ctrl+F5) · Clear timings
        Presenter view · Show settings… · Remote control ▸ Pair a remote… · Paired devices… · Use this device as a remote…
Review  Spelling (F7) · Comments ▸ · Accessibility check (Alt+Shift+A) · Reading order… · Compare with… · Signatures…
Help    Slides help (F1) · Keyboard shortcuts · About
```

### 5.5 Keyboard and pointer

Editor: `T` text box, `R` rectangle, `O` ellipse, `L` line, `C` connector when nothing is being typed; `Enter` on a selected shape edits its text, `Esc` leaves text then deselects; `Tab` next object; `Ctrl+Enter` moves to the next placeholder and, after the last, makes a new slide; `Alt+Shift+Left/Right` promote and demote; arrow nudges as listed under Arrange; `Ctrl+drag` copy; `Shift+drag` constrain axis; `Ctrl+wheel` zoom; middle-drag or `Space+drag` pan.

Show keys are listed under Navigation in show in 4.7. They are fixed defaults that can be rebound; presenter-remote keys cannot be unbound.

Context menus: slide in list (new, duplicate, delete, hide, layout, section, transition, copy as image); object (cut, copy, paste, style, order, align, group, lock, alt text, animation, link, replace); text (formatting, paste options, spelling suggestions, link); canvas (paste, guides, grid, background); thumbnail in presenter view (jump).

Drag and drop: files onto the canvas (images, media, decks to import slides, Markdown for outline); image onto a placeholder fills it; slides between two deck windows; objects out as PNG or SVG; text out as plain text; theme file onto the window applies it after a preview.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start view: themes (`gridView`), recent decks, "Open…", "Import outline…". |
| Loading | Slide list shows numbered placeholders; thumbnails render in the background, visible ones first. |
| Error | Corrupt file: report with the part name; "Recover" loads what parses and lists what was dropped. Missing media: placeholder with a striped box and the original name. |
| Offline | No difference. Remote control needs only the local link. |
| Locked | Password sheet before parsing content. During a show, session lock blanks both outputs. |
| Read-only | Bar; editing disabled; show, export and copy still work. |
| Master edit | Coloured banner "Editing master: Corporate — changes affect 12 slides" with `[Done]`. |
| Narrow (<640) | Slide list becomes an overlay; inspector a bottom sheet; notes strip hidden behind a toggle. |
| One output during show | Presenter view offered in a window; default is audience view only. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Slide list, sorter, theme gallery | `gridView` | Needs multi-select, drag reorder and sections (FUSE-WP-28) |
| Canvas | 2D canvas (**G-12**) | Needs vector objects, handles, guides, rulers, text editing in place; requirements shared with OF08 section 5.7 |
| Text in shapes, notes, outline | rich text (**G-10**) | Styled runs, lists, bidi |
| Inspector | `tabWidget`, `propertyGrid`, `colorPicker`, `fontPicker`, `spinBox`, `slider`, `comboBox` | |
| Selection pane, checker results, comments | `treeView`, `listView` | |
| Animation timeline | `timeline` | Today it has clips, playhead and snap but no keyframe lanes or multi-select (FUSE build plan 2.2); enough for effect bars |
| Charts | chart widget v2 (OF05-GAP-2) | |
| Media objects | `videoWidget` (**G-11**) | Stub today (V3) |
| Audience and presenter windows | `AppWindow` per output + `viewportWidget` or canvas in view-only mode | Needs per-output full screen (G-01, FUSE-WP-11) |
| Small data sheet for charts | `tableView` | Needs inline editors (FUSE-WP-28) |
| QR code | `image` (generated bitmap) | |
| Help | `markdownView` | |

Gaps:

- **OF06-GAP-1 Show renderer.** A way to render a slide scene plus animation state at time `t` to a surface or an off-screen image, identical for live show, thumbnails, PDF and video export. Built on the draw list path API (FUSE-WP-23); not a widget.
- **OF06-GAP-2 Per-output full-screen windows** with stable output identity and hot-plug events (part of G-01 and FUSE-WP-11).
- **OF06-GAP-3 Local-network listen capability.** PLATFORM.md section 6 has `net.host` (outbound, named) and `net.user`; it has no capability for accepting connections on a local interface or for multicast announce. Proposed: `net.listen:<lan>:<port>` with a shell prompt and an audit line.
- G-03 (text shaping), G-16 (print and PDF), G-17 (spelling).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Deck commands, view models, undo, show control state machine, checker rules, settings |
| UI documents and styles | FUSE UI | app host | Windows, views, dialogs |
| Deck model and scene builder | ForgeScript | app host | In-memory deck, style resolution (master → layout → slide → object), scene graph per slide |
| Vector core | C++23 | helper `vector-core` | Geometry for preset and custom shapes, path flattening, text-on-path, hit testing, morph tween; shared source with OF08. Justification: speed |
| Show renderer | C++23 | helper `vector-core` (same process) | Scene + time → draw list or image; used by show, thumbnails, export |
| Deck I/O helper | C++23 | helper `deck-io` | ODP, PPTX, PPT, Markdown parse and write; PDF page building; thumbnailer. Justification: untrusted bytes |
| Media | platform media helper (G-11) | separate process | Decode of audio and video; Slides never parses media bytes itself |
| Export encoder | platform media helper (G-11) | separate process | Video encode from frames |
| Remote control helper | C++23 | helper `remote-link` | Listens, pairs, decrypts, validates the fixed command enum, forwards commands over IPC. Holds the pairing keys. Justification: holds a secret, parses untrusted bytes |
| Macro host for scripted decks | C++23 embedding the ForgeScript engine | helper `macro-host` | Runs signed deck scripts with no capabilities |

```
        +---------------------- app host ----------------------+
        | FUSE UI <-> ForgeScript: deck model, commands, show   |
        +----|-------------|--------------------|---------------+
   scene IPC |   file jobs |      commands only |
             v             v                    v
      +-------------+  +---------+        +-------------+   Noise/TCP   +---------------+
      | vector-core |  | deck-io |        | remote-link | <-----------> | paired remote |
      | + renderer  |  +---------+        +-------------+   (LAN only)  +---------------+
      +-------------+       ^ file handles from the portal
             | frames
             v
      platform media helper (decode, encode) — G-11
```

### 6.2 Data model

| Entity | Fields |
|---|---|
| `Deck` | `id: uuid`, `slide_size: (w_emu, h_emu)`, `masters: [Master]`, `slides: [SlideId]`, `sections: [Section{name, first_slide}]`, `theme: Theme`, `custom_shows: [CustomShow{name, slide_ids}]`, `show: ShowSettings`, `media: {blake3 → MediaItem}`, `fonts_used: [text]`, `meta: Metadata`, `opaque_parts: [Blob]`, `revision: u64` |
| `Master` | `id`, `name`, `background: Fill`, `objects: [Object]`, `layouts: [Layout]`, `text_styles: [LevelStyle; 9]` |
| `Layout` | `id`, `name`, `kind: enum`, `placeholders: [Placeholder{kind, rect, style_ref, index}]` |
| `Slide` | `id: u32`, `layout_id`, `name`, `hidden: bool`, `background: Fill?`, `objects: [Object]` (z-order), `reading_order: [ObjectId]`, `notes: RichText`, `transition: Transition`, `timeline: [Effect]`, `advance_after_ms: u32?`, `comments: [Comment]` |
| `Object` | `id: u32`, `name`, `kind: enum{text, shape, image, table, chart, media, group, connector, embedded, formula}`, `xfrm: {x, y, w, h, rot, flip_h, flip_v}` (EMU, 1/360,000 cm), `placeholder_ref?`, `fill`, `stroke`, `effects`, `text: RichText?`, `alt: {title, description, decorative}`, `locked`, `hidden`, `link?`, kind-specific payload |
| `Effect` | `id`, `target: ObjectId` (+ `paragraph: u32?`), `class: enum{entrance, emphasis, exit, path, media}`, `preset: text`, `trigger: enum{click, with_prev, after_prev, on_object}`, `delay_ms`, `duration_ms`, `repeat`, `params: map` |
| `Transition` | `kind`, `direction`, `duration_ms`, `sound: MediaRef?` |
| `MediaItem` | `blake3`, `mime`, `bytes_len`, `original_name`, `trim_in_ms`, `trim_out_ms`, `poster: ImageRef?` |
| `Comment` | `id: uuid`, `anchor: (slide, object?, text_range?)`, `author`, `initials`, `time`, `body`, `parent?`, `resolved` |
| `PairedRemote` | `device_id: bytes32`, `name`, `public_key`, `added`, `last_used` (in `svc.keyring`) |

Stores: recovery journal `~/.local/share/org.pbsd.slides/recovery/<deck-id>.journal` (command records, BLAKE3 chain) with media in `recovery/media/<blake3>`; versions under `versions/` with `versions.sqlite`; `app.sqlite` for recent decks (portal tokens), output arrangements, template index; thumbnails in `~/.cache/org.pbsd.slides/thumbs/`. Each store has a `schema_version`; migration is forward only with a copy kept.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| ODP, OTP, FODP (ODF 1.3) | yes | yes | Own ZIP and strict XML readers (the same source as OF05's), zlib (zlib licence) | `deck-io` |
| PPTX, PPSX, POTX, PPTM (ISO/IEC 29500) | yes | yes | Own PresentationML and DrawingML mapping | `deck-io` |
| PPT (binary) | yes (C) | no | Own code | `deck-io` |
| Markdown, plain text outline | yes | yes | Own code | `deck-io` |
| PNG, JPEG, WebP, GIF, SVG | yes | PNG, JPEG, SVG | Platform image pipeline (FUSE-WP-27) | platform image helper |
| Audio and video containers and codecs | yes | video export | Platform media pipeline (G-11); codec set Unverified | platform media helper |
| PDF | no | yes | Platform print pipeline (G-16) with tagging | spooler |
| Theme files (shared with OF04, OF05) | yes | yes | Own JSON-in-ZIP format (Proposed) | `deck-io` |
| Remote control protocol `org.pbsd.slides.remote/1` | yes | yes | Own code: Noise XX with a pre-shared pairing secret, length-prefixed records, fixed command enum. Primitives from the platform crypto library (Unverified which) | `remote-link` |
| ODF and OOXML signatures and encryption | yes | yes | Platform crypto, keys through SE04 | `deck-io` |

### 6.4 Integration

- Intents handled: `open`, `edit`, `view`, `new`, `print`. Emitted: `share`, `print`, `edit` (embedded OF05 and OF08 objects), `pick` (screenshot from ME05), `open` (links).
- MIME view/edit: `application/vnd.oasis.opendocument.presentation`, `…presentation-template`, `…presentation-flat-xml`. Import/export: `application/vnd.openxmlformats-officedocument.presentationml.presentation`, `…slideshow`, `…template`. Import only: `application/vnd.ms-powerpoint`, `text/markdown`. Export only: `application/pdf`, `image/png`, `image/svg+xml`, video types of the media pipeline.
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.portal.mic` (narration), `svc.intent`, `svc.search`, `svc.clipboard`, `svc.thumbnail`, `svc.media`, `svc.power`, `svc.keyring`, `svc.audit`, `svc.a11y`, `svc.backup`, `svc.model` (optional).
- Other apps: OF05 (charts and ranges), OF08 (drawings, shared vector core), OF04 (outline hand-off, shared themes and formula object), ME05 (screenshots), SE04 (signing keys), SY01 (displays), AD05 (grants for the remote), VM04 (must respect the sensitive-surface mark of presenter view).
- Backing repos. Verified: FUSE has `timeline`, `videoWidget` (frame-source stub), `chart`, `gridView`, `viewportWidget` (`FUSE:docs/ui/README.md`, `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp`); the draw list today has no paths, transforms or multi-stop gradients (FUSE build plan 2.2, citing `draw_list.hpp:15-24`), which Slides needs. Verified: ForgeScript signing (V14). Proposed: everything else.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Journal, versions, thumbnails |
| `fs.portal` | Open and save decks, insert media |
| `net.none` | Default. Nothing in a deck can open a socket |
| `net.listen:<lan>:<port>` (Proposed, OF06-GAP-3; optional, off) | Remote control, only in `remote-link`, only during a show |
| `proc.helper:vector-core`, `proc.helper:deck-io`, `proc.helper:remote-link`, `proc.helper:macro-host` | Helpers of 6.1 |
| `dev.mic` (optional, prompt) | Narration recording |
| `dev.print` | Printing |
| `dev.gpu.compute` (optional) | Faster show rendering and video export through the Vulkan backend |
| `svc.*` as in 6.4 | |
| `ai.local`, `svc.model` (optional, off) | Draft alt text |

Assets: deck contents and speaker notes (often confidential), the presenter's screen during a show, pairing keys, the microphone.

Adversaries: a crafted deck (parser bugs, bombs, action links that run programs, remote media and font fetches used as tracking beacons, OLE objects); someone on the same network during a talk; a paired device that was lost; a screen recorder capturing notes.

Mitigations:

| Threat | Mitigation |
|---|---|
| Parser bugs | All parsing in `deck-io`, capability mode, no network, per-job process; media and fonts parsed only by platform helpers |
| Active content | No macros, no "run program" actions, no OLE activation, no web content objects; removed or inert, and reported |
| Tracking by linked media | Linked images, media and fonts are never fetched; shown as placeholders with "Embed from file…" |
| Hostile remote on the LAN | Listener off by default; pairing with a one-time secret; encrypted channel; fixed command set; rate limit; stops with the show; visible `[net]` badge; audit log |
| Lost remote | Revoke in the paired list; keys are per device |
| Notes leaking by capture | Presenter view marked sensitive; mirrored-output warning |
| Audience sees notifications | Do-not-disturb during the show |
| Metadata leaks | Strip options; pre-share check lists notes, comments, hidden slides, off-slide objects |
| Seizure risk from content | Flash-safety check; reduced motion honoured |

Stored: journal, versions, recent list under app directories; pairing keys in `svc.keyring`. Leaves the machine: nothing by default. With remote control on: slide thumbnails and notes to the paired device over the encrypted LAN channel.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `new.slide_size` | enum | `16:9` | |
| `new.theme` | text | `plain-dark` | Built-in theme id |
| `author.name` / `author.initials` | text | empty | For comments; never read from the account automatically |
| `edit.snap_objects` / `.snap_grid` / `.smart_guides` | bool | `true` / `false` / `true` | |
| `edit.grid_mm` | float | `5.0` | |
| `edit.units` | enum | locale | cm, mm, in, pt |
| `text.autofit_min_pt` | int | `18` | |
| `text.autocorrect` | bool | `true` | |
| `image.insert_ppi` | int | `220` | |
| `image.strip_exif` | bool | `true` | |
| `show.presenter_view` | bool | `true` | When two outputs exist |
| `show.timer_mode` | enum | `up` | `up`, `down_length`, `down_end_time` |
| `show.target_minutes` | int | `0` | 0 means none |
| `show.dnd` / `show.keep_awake` | bool | `true` | |
| `show.pointer` | enum | `laser` | |
| `show.reduced_motion` | enum | `system` | `system`, `always`, `never` |
| `remote.enabled` | bool | `false` | |
| `remote.port` | int | `47111` | |
| `remote.announce` | bool | `false` | mDNS announce |
| `export.pdf.tagged` | bool | `true` | |
| `export.pdf.strip_metadata` | bool | `true` | |
| `security.embedded_fonts` | enum | `never` | `never`, `ask` |
| `recovery.journal_interval_ms` | int | `2000` | |
| `versions.keep` | int | `30` | |
| `a11y.check_while_editing` | bool | `true` | Issue count in the status bar |
| `ai.alt_text_drafts` | bool | `false` | Needs `ai.local` |

## 9. Accessibility and languages

- The canvas exposes each object as a node in reading order with role, name (alt title or text) and bounds; object moves and resizes are possible from the keyboard and announced ("moved 10 millimetres right").
- The show exposes the current slide's text in reading order to the screen reader on the presenter side; captions from notes help the audience.
- The checker (4.12) is the app-specific part; tagged PDF export carries its results to readers.
- Right-to-left decks mirror placeholders and bullet indents; vertical text for CJK (needs G-03); language per text run for spelling and hyphenation.
- Presenter view scales notes to 300%; high-contrast theme for presenter view regardless of deck theme.

## 10. Performance budgets

| Measure | Target |
|---|---|
| Cold start to start view | 400 ms |
| Open a 200-slide, 300 MB deck | first slide editable in 2 s; thumbnails within 10 s in the background |
| Edit frame time | under 8 ms with 500 objects on a slide |
| Show frame time | 60 fps at 3840×2160 for all built-in effects with the Vulkan backend; 30 fps minimum on the CPU rasteriser at 1920×1080 |
| Slide change latency in show | under 50 ms from key to first frame |
| Remote command latency | under 30 ms on a LAN |
| Memory | under 150 MiB for an empty deck; media streamed, not loaded whole |
| PDF export | 20 slides per second for vector slides |
| Idle | zero frames in the editor; in the show, zero frames between effects on a static slide |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: style resolution chain, placeholder inheritance, effect timing functions, outline ↔ slide mapping, checker rules.
- **Round trip**: ODP → model → ODP and PPTX → model → PPTX with XML normalisation; a corpus of public sample decks.
- **Differential render**: slides rendered by Slides and by LibreOffice Impress (test oracle only) compared by structural similarity with a threshold; differences triaged.
- **Golden renders**: every preset shape, every effect at fixed timestamps (0%, 25%, 50%, 75%, 100%), every transition, at two scales and three themes.
- **Determinism**: video export of a test deck twice gives identical frames; live show frames at the same timestamps match export frames.
- **Scripted UI**: the 14 top tasks through the a11y tree; a two-output show on a headless compositor with two virtual outputs.
- **Fuzz**: ZIP, XML, PresentationML, ODP content, PPT, Markdown import, journal, remote protocol records, pairing messages.
- **Security tests**: decks with action links, OLE, remote media and fonts must produce no process, no socket and a report line; remote protocol with wrong keys, replays and unknown commands.
- **Accessibility**: checker rule corpus; tagged PDF validated with an external checker.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Remote protocol state machine | Only the fixed commands are accepted after a completed handshake; no command before pairing | T1 | PRISM proof over the state machine | Crypto primitives are assumed correct |
| Remote record parser | No runtime errors on any input | T2 | PRISM plus fuzzing | — |
| Effect timing functions | Total, bounded outputs for all `t`; deterministic | T1 | Proof over pure functions | — |
| ZIP and XML readers | No runtime errors | T2 | Shared with OF05; PRISM plus fuzzing | — |
| PresentationML and ODP mapping | Memory safe | T3 | Safe subset, sanitisers, fuzzing | Format size |
| Vector core geometry | No runtime errors; numeric robustness tested | T2 | PRISM for bounds and overflow; property tests | Floating-point reasoning |
| Show renderer | Memory safe | T3 | Sanitisers, golden renders | Size |
| Deck model (ForgeScript) | Memory safe by the language | T3 | Unit tests | Interpreter is trusted base |
| Platform media and image helpers | — | T4 here | Confinement | Owned by the platform |

## 12. Build plan

Phase 1 — usable core:

| ID | Work item | Depends on |
|---|---|---|
| OF06-W01 | Deck model, command layer, undo, journal | G-07, G-08 |
| OF06-W02 | Vector core helper: preset shapes, fills, strokes, hit testing | OF08 vector core work (shared), FUSE-WP-23 |
| OF06-W03 | Canvas editing: select, move, resize, rotate, guides, snap, align | G-12, G-02, OF06-W02 |
| OF06-W04 | Text in shapes and placeholders, lists | G-10, G-03 |
| OF06-W05 | Masters, layouts, themes, placeholders, backgrounds | OF06-W01 |
| OF06-W06 | Normal view, slide list, outline view, sorter, master view | OF06-W03, FUSE-WP-28 |
| OF06-W07 | Images and tables | FUSE-WP-27, OF06-W03 |
| OF06-W08 | ODP read and write | ZIP and XML readers (shared with OF05) |
| OF06-W09 | PPTX read and write, compatibility report | OF06-W08 |
| OF06-W10 | Show renderer (OF06-GAP-1), audience window, presenter view, timers, output selection | G-01, FUSE-WP-11, OF06-W02 |
| OF06-W11 | PDF export | G-16 |
| OF06-W12 | Trust bar, recovery, templates, palette, help | G-09 |

Phase 2 — full feature:

| ID | Work item | Depends on |
|---|---|---|
| OF06-W13 | Animations, transitions, animation panel, reduced motion | OF06-W10 |
| OF06-W14 | Charts and small data sheet | OF05 chart widget (OF05-GAP-2) |
| OF06-W15 | Audio and video objects | G-11 |
| OF06-W16 | Rehearsal, pointer tools, kiosk mode, notes view, handouts, print | OF06-W10, G-16 |
| OF06-W17 | Comments, versions, share check, metadata tools | G-09 |
| OF06-W18 | Accessibility checker, reading order editor, tagged PDF | OF06-W11 |
| OF06-W19 | Styles, spelling, links, sections, custom shows, find and replace, fonts report | G-17 |
| OF06-W20 | Connectors, selection pane, compress images, image export | OF06-W03 |
| OF06-W21 | Search and thumbnail integration, disposable open | G-09 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF06-W22 | Remote control helper, pairing, remote mode | OF06-GAP-3, AD05, platform crypto |
| OF06-W23 | Video export, narration, animated image export | G-11, OF06-W13 |
| OF06-W24 | Embedded objects from OF05 and OF08, formula object | OF05, OF08, OF04 |
| OF06-W25 | Motion paths, object-match transition, flash-safety check | OF06-W13 |
| OF06-W26 | Compare and merge, comment sidecars, signatures | SE04 |
| OF06-W27 | Markdown deck import, PPT import, package export | OF06-W09 |
| OF06-W28 | Scripted decks, layout suggestions, alt text drafts | FORGE signing, G-15 |

## 13. Open questions and risks

- **Canvas and rich text are both platform gaps** (G-12, G-10). Slides cannot start its editor before both exist; the show renderer and file I/O can start earlier.
- **Listen capability** (OF06-GAP-3) does not exist in PLATFORM.md. Until it does, remote control cannot be built within the rules.
- **Remote client on non-PBSD devices.** A phone would need its own client. A small static web page served by the helper would work in any browser but adds an HTTP surface and loses the Noise channel; left out unless the owner asks.
- **PPTX fidelity** (SmartArt, 3D, many effects) is a long tail. The report must be accurate; a differential corpus is needed early.
- **Video codecs** depend on G-11 and on licensing choices not made here.
- **Fonts.** Decks made elsewhere name fonts that are not installed. Metric-compatible substitutes need to ship with the system; which ones is a platform decision.
- **Shared code with OF08 and OF05.** Vector core, chart rendering, ZIP and XML readers are shared sources built into each bundle's helper. A shared, separately signed library bundle would reduce duplication; the bundle format (PLATFORM.md section 3) has no such concept yet.
- **Presenter view on one GPU with two outputs at different scales** needs fractional-scale work (FUSE-WP-06); unmeasured.
