<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Photos

By Odin Loch

| | |
|---|---|
| ID | `ME01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.photos` |
| Category | media |
| Priority | P1 |
| Backing repos | FUSE (image widget, `stb_image`, `lpng` vendored). No photo app exists. |
| Status | Planned. Nothing built. |

## 1. Purpose

Photos is the image viewer and the photo library. People use it to look at a picture quickly, to keep and find their photographs, and to make small non-destructive corrections. It replaces Gwenview and digiKam for everyday use; heavy editing belongs to CR01.

## 2. Scope

In scope:

- Viewer for every common still-image format, including RAW families and animated images.
- Library: import, albums, tags, people and object search with local models, places, timeline.
- Non-destructive quick edits stored as edit lists, with presets and batch apply.
- Metadata view, edit and strip (EXIF, XMP, IPTC).
- Duplicates, compare and cull, slideshow, colour management.
- Export and share with metadata stripped by default; hidden and encrypted albums; print layouts.

Out of scope:

| Item | Owner |
|---|---|
| Layers, masks, brushes, compositing | CR01 Image Editor |
| Vector editing of SVG | OF08 Draw |
| Video playback beyond a preview of clips in the library | ME03 Video |
| Taking photos, document scanning | ME04 Camera, UT06 Printers and Scanners |
| Screenshots | ME05 Screen Capture |
| Map rendering engine and offline tiles | IN04 Maps (embedded view through an intent panel) |
| Phone and camera transport (MTP, PTP) | VM05 Devices and the USB portal (SH04) |
| Encrypted container format | SE05 Vault |
| Model install and verification | AI02 Model Manager |
| Print spooling | UT06, `svc.portal.print` |
| Cloud photo services | not planned |

## 3. Users and top tasks

Personas:

- **Viewer**: double-clicks an image in Files and flips through the folder.
- **Family archivist**: 80 000 photos over 20 years; wants them sorted by date, person and place.
- **Photographer**: shoots RAW, culls 600 frames to 40, applies a preset, exports JPEG.
- **Careful sharer**: sends a photo and must not leak location or camera serial.
- **Private user**: keeps some albums hidden and encrypted.

| # | Task | Target steps |
|---|---|---|
| T1 | Open an image from Files (SY02) | 1 |
| T2 | Next and previous image in the folder | 1 key |
| T3 | Zoom to 100% at the pointer | 1 (`Z` or double-click) |
| T4 | Import from a camera card | 3 (plug, pick "new only", Import) |
| T5 | Find photos of a person at a place in a year | 1 search line |
| T6 | Rate and reject while culling | 1 key per photo |
| T7 | Crop and straighten | 3 (`C`, drag, Enter) |
| T8 | Apply one edit to 50 photos | 3 (copy edits, select, paste edits) |
| T9 | Export for sharing with no metadata | 2 (`Ctrl+Shift+E`, Export) |
| T10 | Remove location from 200 photos | 3 |
| T11 | Find and remove duplicates | 3 |
| T12 | Start a slideshow of an album | 1 (`F5`) |
| T13 | Move photos into a hidden album | 2 |
| T14 | Print a contact sheet | 3 |
| T15 | Compare two near-identical shots side by side | 2 (select, `Y`) |

## 4. Features

### 4.1 Viewer

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F001 | Open image | From intent `open`/`view`, drop, `Ctrl+O`. First frame shown from the embedded preview within 50 ms, then replaced by the full decode. | M | 1 |
| ME01-F002 | Folder browsing | When the folder is granted, `Left`/`Right`, `PgUp`/`PgDn`, `Home`/`End` move through siblings in the sort order of SY02. Without a grant an inline bar offers "Allow this folder". | M | 1 |
| ME01-F003 | Preload | Next and previous images decoded ahead into a 3-image cache. | M | 1 |
| ME01-F004 | Zoom | Fit, fill, 100%, 200%, free zoom 5%–6400%, zoom at pointer, `Ctrl`+wheel, pinch. Nearest-neighbour above 400% (pixel grid optional). | M | 1 |
| ME01-F005 | Pan | Drag, arrow keys, `Space`+drag; minimap in the corner when zoomed. | M | 1 |
| ME01-F006 | Rotate and flip view | `R`, `Shift+R`, `H`, `V`. Applies as an edit-list entry, never rewrites the file unless "save rotation to file" is chosen (lossless JPEG transform). | M | 1 |
| ME01-F007 | EXIF orientation | Honoured for all formats; shown in the inspector. | M | 1 |
| ME01-F008 | Full screen | `F11`, `F`. Chrome hides; filmstrip on pointer at the bottom edge. | M | 1 |
| ME01-F009 | Filmstrip | Bottom strip of thumbnails of the current set; toggle `T`. | S | 1 |
| ME01-F010 | Background | Dark, light, checkerboard for transparency, custom colour. | S | 1 |
| ME01-F011 | Animated images | GIF, APNG, animated WebP, AVIF sequences, JPEG XL animation: play, pause, frame step, speed, loop count honoured. | M | 1 |
| ME01-F012 | Multi-page images | TIFF pages, ICO sizes, HEIF collections, JPEG XL layers: page strip and page keys. | S | 2 |
| ME01-F013 | Live and motion photos | HEIF with embedded video and paired JPG+MOV: hold `Space` to play through the media pipeline (ME03 section 6.5). | C | 3 |
| ME01-F014 | SVG view | Static SVG subset rendered by own code: no scripts, no external references, no animation. Unsupported features are listed in the inspector. | M | 1 |
| ME01-F015 | Large images | Tiled decode and display up to 65 535 × 65 535 px; gigapixel TIFF pyramids read by level. | S | 2 |
| ME01-F016 | HDR images | AVIF, HEIF, JPEG XL, OpenEXR, Radiance HDR, gain-map JPEG (ISO 21496-1): tone mapped to SDR or shown in HDR when the compositor allows (ME03-GAP-1). | S | 2 |
| ME01-F017 | Histogram overlay | Luma and RGB, live, `Ctrl+H`. Clipping warning overlay `J`. | S | 1 |
| ME01-F018 | Pixel inspector | Colour under the pointer in sRGB, display, and source space; coordinates. | C | 2 |
| ME01-F019 | Copy image | `Ctrl+C` puts PNG and a file handle on the clipboard. Metadata stripped from the PNG. | M | 1 |
| ME01-F020 | Set as wallpaper | Sends the handle to SY01 through intent `set-wallpaper` after confirmation. | C | 2 |
| ME01-F021 | Open with | Edit in CR01, open in OF08 (SVG), other apps through `svc.intent`. | M | 1 |
| ME01-F022 | Delete | `Delete` to trash through `svc.undo-trash` with Undo toast. | M | 1 |
| ME01-F023 | Sort and filter in viewer | Name, date taken, date modified, size, type, rating. | S | 1 |
| ME01-F024 | Video clips in sets | Clips shown with a play badge; `Enter` previews muted; "Open in Video" hands to ME03. | S | 2 |

### 4.2 Formats

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F025 | JPEG | Baseline, progressive, 12-bit, CMYK, arithmetic coding; embedded ICC; MPO. | M | 1 |
| ME01-F026 | PNG, APNG | All bit depths, interlace, `iCCP`, `cICP`, `eXIf` chunks. | M | 1 |
| ME01-F027 | WebP | Lossy, lossless, alpha, animation. | M | 1 |
| ME01-F028 | AVIF | 8/10/12-bit, alpha, HDR, grid images, sequences. | M | 1 |
| ME01-F029 | HEIF/HEIC | In the restricted codec pack (HEVC patents, ME03 section 6.5.3). Without it the app says which pack is missing; embedded JPEG preview shown if present. | S | 2 |
| ME01-F030 | JPEG XL | Lossy, lossless, HDR, animation, JPEG reconstruction. | M | 1 |
| ME01-F031 | TIFF | Strips, tiles, pyramids, 8/16/32-bit, float, LZW, Deflate, PackBits, JPEG-in-TIFF, BigTIFF. | M | 1 |
| ME01-F032 | GIF | 87a and 89a. | M | 1 |
| ME01-F033 | BMP, ICO, CUR, PNM, TGA, PCX, QOI, Farbfeld | Own decoders. | S | 1 |
| ME01-F034 | OpenEXR, Radiance HDR, DDS, KTX2 | View only. | C | 3 |
| ME01-F035 | JPEG 2000 | View only, optional. | C | 3 |
| ME01-F036 | PSD, XCF, ORA, KRA | Flattened preview only; "Open in Image Editor" for layers. | C | 3 |
| ME01-F037 | RAW families | DNG, CR2, CR3, NEF, NRW, ARW, SRF, SR2, RAF, ORF, RW2, PEF, SRW, 3FR, IIQ, X3F, ERF, MRW, DCR, KDC. Camera list comes from the RAW library version; unsupported models fall back to the embedded JPEG with a badge. | M | 2 |
| ME01-F038 | RAW + JPEG pairs | Shown as one item; choose which is primary. | S | 2 |
| ME01-F039 | Sidecars | Reads and writes `.xmp` sidecars; reads darktable and Lightroom rating, label and keyword fields; ignores their develop settings. | S | 2 |
| ME01-F040 | Format report | Help ▸ Supported formats lists every decoder, its version, licence and pack. | S | 1 |

### 4.3 Library and import

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F041 | Library roots | Folders granted through the file portal. Files stay where they are; the library is an index. Optional managed root with `YYYY/MM/DD` layout. | M | 1 |
| ME01-F042 | Watch for changes | File events on granted roots update the index; a rescan can be forced with `F5`. | M | 1 |
| ME01-F043 | Import from folder | Copy or reference; rename by template (`{date}_{time}_{seq}`), folder template, duplicate skip by content hash. | M | 1 |
| ME01-F044 | Import from camera or card | Mass-storage cards through the file portal; PTP and MTP through `dev.usb:ptp` / `dev.usb:mtp` in the device helper. Shows only new items by default. | M | 2 |
| ME01-F045 | Import from phone | MTP; also a folder received through IN03 local device send. No vendor sync protocol. | S | 2 |
| ME01-F046 | Import review | Grid with checkboxes, "new only", date groups, destination preview, free-space check, per-item verify (BLAKE3 after copy). | M | 2 |
| ME01-F047 | Delete from device after import | Off by default; only after verification; confirmation names the count. | C | 2 |
| ME01-F048 | Import provenance | Each item records source device label, import session id and time. | S | 2 |
| ME01-F049 | Timeline | Years ▸ months ▸ days with sticky headers and a year scrubber on the right edge. | M | 1 |
| ME01-F050 | Albums | Manual albums; an item can be in many. Album cover, description, sort order, nested folders of albums. | M | 1 |
| ME01-F051 | Smart albums | Saved searches (rules on any indexed field). | S | 2 |
| ME01-F052 | Tags | Hierarchical keywords (`Animals/Dog`), stored in the database and optionally written to XMP `dc:subject` and `lr:hierarchicalSubject`. | M | 1 |
| ME01-F053 | Ratings, labels, flags | 0–5 stars (`0`–`5`), colour labels (`6`–`9`), pick `P` / reject `X`. Written to XMP on request. | M | 1 |
| ME01-F054 | Favourites | Heart toggle `.`; a built-in album. | S | 1 |
| ME01-F055 | Stacks | Group bursts, brackets, RAW+JPEG and edits; auto-stack by capture time within n seconds. | S | 2 |
| ME01-F056 | Folders view | The granted folder trees as they are on disk. | M | 1 |
| ME01-F057 | Recently imported, recently edited | Built-in views. | S | 1 |
| ME01-F058 | Trash view | Items trashed from the app, restorable through `svc.undo-trash`. | S | 1 |
| ME01-F059 | Missing and offline items | Greyed with the volume name; relink tool matches by content hash. | S | 2 |
| ME01-F060 | Library on removable or network volumes | Thumbnails cached locally; originals marked offline when unmounted. | S | 2 |
| ME01-F061 | Multiple libraries | Open another library file; one is default. | C | 3 |
| ME01-F062 | Library backup hints | Tells `svc.backup` which stores to include; originals are the user's folders. | S | 2 |

### 4.4 Search, people, objects, places

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F063 | Search bar | Free text plus tokens: `tag:`, `person:`, `place:`, `camera:`, `lens:`, `date:2019-07`, `rating:>=4`, `type:raw`, `iso:>1600`, `text:` (OCR). Suggestions as you type. | M | 1 |
| ME01-F064 | Filters panel | Date range, type, rating, label, camera, lens, focal length, aperture, ISO, orientation, has location, edited, in album. | M | 1 |
| ME01-F065 | Object and scene search | Image embeddings from a local model through `svc.model`; text queries matched to embeddings ("red car at night"). Index built in the background, only on mains power by default. | S | 2 |
| ME01-F066 | People | Face detection and clustering with a local model. The user names clusters; merge, split, "not this person", hide a person. Nothing is named automatically. | S | 2 |
| ME01-F067 | People data rules | Face vectors are stored in an encrypted store with a key in `svc.keyring`; never exported, never written to files unless the user asks for XMP region tags. One switch deletes all face data. | M | 2 |
| ME01-F068 | Text in photos | Optional OCR index through `svc.model`. | C | 3 |
| ME01-F069 | Similar photos | "Find similar" from any photo by embedding distance. | C | 3 |
| ME01-F070 | Places | Reverse geocoding offline from the IN04 place database; hierarchy country ▸ region ▸ town. | S | 2 |
| ME01-F071 | Map view | Clustered pins on the embedded IN04 map panel; box select; no tile requests unless IN04 has a network grant of its own. | S | 2 |
| ME01-F072 | Set or fix location | Drag photos onto the map; type coordinates; copy location between photos. | S | 2 |
| ME01-F073 | GPX matching | Match photos to a GPX track by time with a clock-offset control. | C | 3 |
| ME01-F074 | System search | Contributes titles, tags, album names (not people names unless enabled) to `svc.search`. Hidden albums never contribute. | S | 2 |
| ME01-F075 | Model state | Each AI feature shows which model, its signature state, and index progress; all are off until a model is installed (AI02). | M | 2 |

### 4.5 Quick edits (non-destructive)

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F076 | Edit list | Edits are a versioned JSON list per item in the database (and optionally an `.xmp` sidecar field). The original file is never changed. | M | 1 |
| ME01-F077 | Crop | Free and fixed ratios (1:1, 4:3, 3:2, 16:9, 5:4, custom), overlays (thirds, golden, diagonal), swap orientation `X`. | M | 1 |
| ME01-F078 | Straighten and rotate | Angle slider ±45°, draw-a-horizon tool, auto-level from detected lines. | M | 1 |
| ME01-F079 | Perspective | Vertical, horizontal, four-point keystone. | S | 2 |
| ME01-F080 | Exposure | Exposure (±5 EV), contrast, highlights, shadows, whites, blacks. Works in linear scene-referred space for RAW. | M | 1 |
| ME01-F081 | Colour | White balance (temperature, tint, picker, presets), vibrance, saturation, per-hue HSL (8 bands). | M | 1 |
| ME01-F082 | Curves | RGB and per-channel curves with `curveEditor`; point and parametric modes. | M | 2 |
| ME01-F083 | Levels | Black, grey, white points with histogram. | S | 2 |
| ME01-F084 | Detail | Sharpen (amount, radius, masking), noise reduction (luma, chroma), optional neural denoise for RAW through `svc.model`. | S | 2 |
| ME01-F085 | Lens corrections | Distortion, vignetting, chromatic aberration from an offline lens profile database (Lensfun data, CC-BY-SA); manual sliders. | S | 2 |
| ME01-F086 | Heal | Spot heal and clone with brush size and feather; content-aware fill from a local model (optional). | S | 2 |
| ME01-F087 | Red-eye | Click to fix. | C | 2 |
| ME01-F088 | Vignette, grain | Creative effects. | C | 2 |
| ME01-F089 | Black and white | Channel mixer, tone, split toning. | S | 2 |
| ME01-F090 | Local adjustments | Linear and radial gradients with exposure, colour and detail. Brush masks are CR01. | C | 3 |
| ME01-F091 | Auto enhance | One button; shows what it changed as normal sliders. Classic algorithm; optional local model. | S | 2 |
| ME01-F092 | Presets | Built-in and user presets (`.pbpreset`, JSON); import and export; hover preview. | M | 2 |
| ME01-F093 | Copy and paste edits | `Ctrl+Shift+C` / `Ctrl+Shift+V` with a checklist of which edit groups to paste. | M | 2 |
| ME01-F094 | Batch edit | Apply preset, paste edits, auto enhance, rotate, set time offset to a selection; one undo step. | M | 2 |
| ME01-F095 | Before and after | Hold `\` to see the original; split view. | M | 1 |
| ME01-F096 | History and versions | Step list per item; named versions (virtual copies). | S | 2 |
| ME01-F097 | Revert | "Revert to original" clears the edit list (undoable). | M | 1 |
| ME01-F098 | Save a copy | Render edits to a new file; original kept. "Overwrite" exists only behind a setting and moves the original to trash first. | M | 1 |
| ME01-F099 | RAW development defaults | Demosaic method, highlight recovery, camera colour matrix or DNG profile, base curve. | S | 2 |
| ME01-F100 | Processing pipeline | 32-bit float linear working space (Rec.2020 linear); GPU path through `dev.gpu.compute`, CPU fallback with identical results within tolerance. | M | 2 |

### 4.6 Metadata

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F101 | Metadata panel | EXIF, XMP, IPTC-IIM, ICC summary, maker notes (read only), file facts, hash. Grouped, searchable, copyable. | M | 1 |
| ME01-F102 | Edit fields | Title, description, creator, copyright, keywords, rating, date and time, time zone, location, licence (URL and text). | M | 2 |
| ME01-F103 | Time shift | Fix camera clock for a selection: offset or set absolute; time-zone aware. | S | 2 |
| ME01-F104 | Write policy | Per library: database only (default), XMP sidecar, or into the file. Writing into files uses a temp file and atomic rename; the original goes to trash for one session. | M | 2 |
| ME01-F105 | Strip metadata | Remove all, or keep chosen groups (orientation, colour profile, copyright). Removes thumbnails, maker notes, GPS, serial numbers, XMP history, embedded previews, C2PA manifests on request. | M | 1 |
| ME01-F106 | Privacy scan | Lists what in a selection could identify the user: GPS, serials, owner name, software, faces, thumbnails that differ from the image. | S | 2 |
| ME01-F107 | Embedded thumbnail mismatch | Warns when the embedded preview shows uncropped content. | S | 2 |
| ME01-F108 | Templates | Metadata templates (creator, copyright, licence) applied on import or to a selection. | S | 2 |
| ME01-F109 | Content credentials | Read and verify C2PA manifests; show signer and edit claims; trust decided by the local trust store (SE04). Signing own exports is optional with a key from SE04. | C | 3 |
| ME01-F110 | Hash and signature | BLAKE3 and SHA-256 of the original; verify a detached signature beside the file through SE04. | C | 3 |

### 4.7 Cull, compare, duplicates

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F111 | Cull mode | Full-screen one-up with rating, pick and reject keys, auto-advance, focus-peaking overlay, 100% loupe on `Z`. | M | 2 |
| ME01-F112 | Compare | 2-up, 3-up, 4-up with linked zoom and pan (`Y`); swap, promote, reject. | M | 2 |
| ME01-F113 | Survey | Shrinking grid: remove candidates until the keepers remain. | C | 3 |
| ME01-F114 | Sharpness and eyes-closed hints | Optional local scoring to sort a burst; shown as hints, never auto-reject. | C | 3 |
| ME01-F115 | Exact duplicates | Same content hash; shows paths; keep rules (oldest, largest, in album). | M | 2 |
| ME01-F116 | Near duplicates | Perceptual hash and embedding distance with a threshold slider; groups shown side by side. | S | 2 |
| ME01-F117 | Duplicate actions | Trash, stack, or replace-in-albums. Always through `svc.undo-trash`. | M | 2 |
| ME01-F118 | Reject cleanup | "Trash all rejected" with count and Undo. | S | 2 |

### 4.8 Slideshow, export, share, print

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F119 | Slideshow | `F5`. Interval 1–60 s, order, shuffle, loop, transitions (cut, fade 200 ms; none with reduced motion), captions, fit or fill, second-screen output. | M | 1 |
| ME01-F120 | Slideshow music | A playlist handle from ME02 through `svc.media`; optional. | C | 3 |
| ME01-F121 | Export | Format (JPEG, PNG, WebP, AVIF, JPEG XL, TIFF; HEIF only with the restricted pack), quality, size (long edge, megapixels, percent), sharpening for output, colour space (sRGB default, Display P3, Adobe RGB compatible, Rec.2020), bit depth. | M | 1 |
| ME01-F122 | Export metadata default | Strips everything except orientation, colour profile and (optional) copyright. A summary line states what will be included. | M | 1 |
| ME01-F123 | Export naming | Templates, sequence numbers, sub-folders by date or album, collision policy. | S | 2 |
| ME01-F124 | Export presets | Named presets; "Share small", "Print full", "Archive lossless" built in. | S | 2 |
| ME01-F125 | Watermark | Text or image, position, opacity. | C | 3 |
| ME01-F126 | Batch convert | Convert and resize files without adding them to the library. | S | 2 |
| ME01-F127 | Share | Share sheet (SH04) with the export summary; shares a rendered copy, never the original, unless "share original" is ticked. | M | 2 |
| ME01-F128 | Export album | Folder with files plus optional static HTML index (no scripts) or a PDF. | C | 3 |
| ME01-F129 | Print layouts | Single, n-up, contact sheet with captions, passport grid, custom cells in mm; borders, crop marks, paper profile and rendering intent; print through `svc.portal.print` (G-16). | S | 2 |
| ME01-F130 | Soft proof | Preview with a printer ICC profile and gamut warning. | C | 3 |
| ME01-F131 | Send to | Edit in CR01, attach in CO01, set as contact photo in CO03, through intents. | S | 2 |

### 4.9 Hidden and encrypted albums

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F132 | Hidden album | Items leave the timeline, search, people, map and `svc.search`. Shown only after re-authentication (SH03 prompt). | M | 2 |
| ME01-F133 | Encrypted album | A Vault container (SE05) owned by the album. Originals are moved in; thumbnails, edit lists and index rows for those items live inside the container too. | M | 2 |
| ME01-F134 | Lock state | Locked albums show a lock and a count of zero. Auto-lock on screen lock, on idle (5 min default) and on app close. | M | 2 |
| ME01-F135 | Secure move | Moving into an encrypted album overwrites the plaintext thumbnails and index rows and offers secure erase of the original through SE05. States ZFS snapshot limits plainly. | M | 2 |
| ME01-F136 | No leakage | Locked items never appear in recents, clipboard history, thumbnails service or backups in plaintext. | M | 2 |
| ME01-F137 | Decoy-free | No hidden-volume or deniability claims are made. | M | 2 |

### 4.10 Colour management

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F138 | Input profiles | Embedded ICC v2 and v4, `cICP`, EXIF colour space; untagged assumed sRGB (setting). | M | 1 |
| ME01-F139 | Display profile | From the compositor per output; conversion in the display path; windows spanning two outputs use the output with the larger area. | M | 2 |
| ME01-F140 | Rendering intent | Relative colorimetric with black-point compensation by default; perceptual optional. | S | 2 |
| ME01-F141 | Wide gamut and high bit depth | 10-bit and float surfaces when the compositor offers them. | S | 3 |
| ME01-F142 | Profile inspector | Shows profile name, primaries, white point, tone curve; ICC parsed in the codec helper. | C | 2 |

### 4.11 Security, automation, recovery

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME01-F143 | Decoders in zero-capability helpers | Every image, RAW, ICC and metadata parse runs in `image-codec-helper` with no capabilities; the app receives pixels and typed records. | M | 1 |
| ME01-F144 | Disposable viewing | "Open in a disposable sandbox" for files with a "from network" provenance mark: nothing indexed, no thumbnails kept. | S | 2 |
| ME01-F145 | Decode limits | Pixel count, memory and time limits per decode; decompression-bomb detection with a clear message. | M | 1 |
| ME01-F146 | SVG safety | No network, no scripts, no external files, no entities. | M | 1 |
| ME01-F147 | Thumbnailer | Registers image thumbnailers with `svc.thumbnail`; same helper, 1 s CPU limit. | M | 1 |
| ME01-F148 | Command line | `photos [--slideshow] [--disposable] <handles…>`. | S | 1 |
| ME01-F149 | Bus interface | `org.pbsd.photos.Library1`: query, add to album, export; per-caller approval. Used by the pick intent. | S | 2 |
| ME01-F150 | Pick intent | Other apps ask for photos; the user picks in a Photos sheet; the caller gets handles to rendered copies with metadata stripped by default. | M | 2 |
| ME01-F151 | Crash recovery | Edits are journalled per slider release; import resumes after a crash; the database is checked on start. | M | 1 |
| ME01-F152 | Library check and repair | Verify hashes of originals, find orphans, rebuild thumbnails. | S | 2 |
| ME01-F153 | Snapshots | "Restore library from snapshot" lists `svc.backup` snapshots of the app data. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Photos
├─ Main window (library; single instance)
│  ├─ Sidebar: Timeline · Favourites · People · Places · Albums ▸ · Smart albums ▸ ·
│  │           Tags ▸ · Folders ▸ · Imports · Duplicates · Hidden (lock) · Trash
│  ├─ Content: grid | timeline | map | people | single view | compare | cull
│  ├─ Inspector (F4): Info · Metadata · Edit · History
│  └─ Status bar
├─ Viewer window (one per "open" intent when no library context)
├─ Slideshow (full screen)
├─ Dialogs and sheets
│  ├─ Import
│  ├─ Export
│  ├─ Strip metadata / Privacy scan
│  ├─ Duplicates review
│  ├─ Print layout
│  ├─ Batch convert
│  ├─ Time shift
│  ├─ New album / Smart album rules
│  ├─ Name person / Merge people
│  ├─ New encrypted album (hands to SE05)
│  └─ Paste edits checklist
├─ Preferences: General · Library · Import · Viewer · Editing · Colour · Metadata ·
│               People and search · Privacy · Shortcuts
└─ Shell-drawn: file portal, USB portal, share sheet, print dialog, auth prompt
```

### 5.2 Main window

Shown at 1200×720, wide breakpoint, timeline grid, inspector open.

```
+------------------------------------------------------------------------------------------+
| [=] Photos — Timeline          [search: person:Mia place:Hobart 2019]    [▁] [▢] [✕]    |(1)
+--------------+------------------------------------------------------+--------------------+
| Timeline  (2)| [Import] [Grid|Map|People] [★≥ v] [Filters]  size ─●─ | Info | Meta | Edit |(6)
| Favourites   +------------------------------------------------------+--------------------+
| People       | July 2019 · Hobart                         142 items  | IMG_4821.CR3       |
| Places       | +------+ +------+ +------+ +------+ +------+      2024| 6000×4000  24.1 MB |
| ▾ Albums     | | ★★★★ | |      | | RAW  | |  ▶   | | ⧉3   |      2023| 12 Jul 2019 14:03  |
|   Tasmania   | +------+ +------+ +------+ +------+ +------+      2022| f/4  1/500  ISO 200|
|   Garden     | +------+ +------+ +------+ +------+ +------+     >2019| 35 mm              |
| ▸ Smart      | |      | | ✎    | |      | |      | |      |      2018| ★★★★☆  ● green     |
| ▸ Tags       | +------+ +------+ +------+ +------+ +------+  (5) 2017| Tags: Animals/Dog  |
| ▸ Folders    | June 2019 · Launceston                      88 items  | People: Mia        |
| Imports      | +------+ +------+ +------+ +------+ +------+          | Place: Hobart      |
| Duplicates 12| |      | |      | |      | |      | |      |      (3) | [map preview]      |
| Hidden 🔒    | +------+ +------+ +------+ +------+ +------+          | Albums: Tasmania   |
| Trash        |                                                   (4) | [Strip metadata…]  |
+--------------+------------------------------------------------------+--------------------+
| 3 selected · 71.2 MB · indexing people 62% (on mains only)        sRGB · P3 display      |(7)
+------------------------------------------------------------------------------------------+
```

1. Header bar with the search field (`lineEdit` with token chips). No `[net]` badge ever appears: the app has no network capability.
2. Sidebar, `treeView`. Drop photos on an album or tag to add. Counts on the right.
3. Content, `gridView` virtualised over `SqliteModel`, grouped by day with sticky headers.
4. Cell badges: rating, RAW, video `▶`, stack count `⧉`, edited `✎`, offline cloud-free "volume" icon, lock.
5. Year scrubber, custom `scrollBar` with year labels.
6. Inspector, `tabWidget`: Info (summary and tags), Metadata (`propertyGrid`), Edit (sliders, `curveEditor`), History (`listView`).
7. Status bar: selection, background jobs with pause, colour state of source and display.

States: empty library ("Add a folder of photos" with one button), loading (grey cells then thumbnails), error (cell with a broken-image icon and a reason on hover), offline volume (greyed), locked album (lock cells), read-only (library on read-only media: edits kept in app data, a bar says so).

### 5.3 Other views and dialogs

**Single view with Edit tab** (1200×720)

```
+------------------------------------------------------------------------------------------+
| [←] IMG_4821.CR3   12 Jul 2019            [Fit v] [Compare Y] [Share] [Export] [▁][▢][✕]|
+---------------------------------------------------------------------+--------------------+
|                                                                     | [Crop C] [Heal Q]  |
|                                                                     | Histogram ▁▃▆█▅▂   |
|                                                                     | Auto [Enhance]     |
|                         IMAGE CANVAS (1)                            | ▾ Light            |
|                                                                     |  Exposure  ──●── +0.3|
|                                                                     |  Contrast  ─●─── -5 |
|                                                                     |  Highlights ●──── -40|
|                                                                     |  Shadows  ───●─ +25 |
|                                                                     | ▾ Colour            |
|                                                                     |  Temp ──●── 5200 K  |
|                                                                     | ▸ Curves ▸ Detail   |
|                                                                     | ▸ Lens ▸ Effects    |
+---------------------------------------------------------------------+ [Presets v]        |
| [thumb][thumb][>thumb<][thumb][thumb][thumb][thumb]  (2)            | [Copy] [Paste][Revert]|
+---------------------------------------------------------------------+--------------------+
```

(1) Canvas: zoom, pan, crop overlay, heal brush. Needs a 2D canvas with tiles (G-12); phase 1 uses `image` in a `scrollArea`. (2) Filmstrip `listView` horizontal. States: decoding (preview then sharp), unsupported camera (badge "embedded preview"), missing original (edits disabled).

**Import** (sheet, 900×600)

```
+----------------------------------------------------------------------------------+
| Import from "EOS_DIGITAL" (card, 32 GB)                    [x] New only (214)   |
| +------+ +------+ +------+ +------+ +------+ +------+                            |
| |[x]   | |[x]   | |[x]   | |[ ]dup| |[x]   | |[x]   |   12 Jul 2019 (64)        |
| +------+ +------+ +------+ +------+ +------+ +------+                            |
| Copy to  [Pictures/{YYYY}/{MM}/{DD} v]   Rename [{date}_{time}_{seq} v]          |
| Album    [none v]   Tags [__________]   Template [My copyright v]                |
| [x] Verify after copy   [ ] Delete from device after verified import            |
| Needs 5.1 GB · 212 GB free                               [Cancel] [Import 214]  |
+----------------------------------------------------------------------------------+
```

**Export** (dialog, 620×480)

```
+------------------------------------------------------------------+
| Export 3 photos                         Preset [Share small v]   |
| Format [JPEG v]  Quality [85]   Size [long edge v] [2048] px     |
| Colour space [sRGB v]   Output sharpening [screen v]             |
| Metadata ( ) all  (•) none except orientation and profile        |
|          ( ) choose… [ ] copyright [ ] date [ ] location         |
|   Will be removed: GPS, camera serial, owner name, 3 thumbnails  |
| Name [{original}_{seq}]      Folder [Pictures/Export] [Choose…]  |
| [ ] Watermark…                                                   |
| About 2.4 MB in total                    [Cancel] [Export 3]     |
+------------------------------------------------------------------+
```

**Compare** (2-up)

```
+---------------------------------------------+---------------------------------------------+
| IMG_4821  ★★★★   [Pick P] [Reject X]        | IMG_4822  ★★★    [Pick P] [Reject X]        |
|                                             |                                             |
|              image A (linked zoom)          |              image B (linked zoom)          |
|                                             |                                             |
| f/4 1/500 ISO 200  sharpness hint 0.82      | f/4 1/400 ISO 200  sharpness hint 0.61      |
+---------------------------------------------+---------------------------------------------+
| [Link zoom x] [Swap] [2|3|4 up]   candidates: [t][t][t][t][t]                  [Done]    |
+-------------------------------------------------------------------------------------------+
```

**Duplicates review** (900×600)

```
+----------------------------------------------------------------------------------+
| Duplicates: [Exact 12] [Near 31]      Similarity ──●── 92%     Keep rule [oldest v]|
| Group 1 of 12                                                                    |
| +-----------+ +-----------+   Pictures/2019/07/IMG_4821.JPG   6.1 MB  (keep)     |
| | (keep)    | | [x] trash |   Downloads/IMG_4821 (1).JPG      6.1 MB             |
| +-----------+ +-----------+   Same content hash                                  |
| [Previous] [Next]  [Apply rule to all]              [Cancel] [Trash 12 files]    |
+----------------------------------------------------------------------------------+
```

**People view**

```
+----------------------------------------------------------------------------------+
| People (local model "faces-small", verified)        [Pause indexing] [Delete all]|
| (o) Mia 412   (o) Sam 208   (o) ? 57 [Name…]   (o) ? 31 [Name…]   (o) hidden 4   |
| Suggestions: "Is this Mia?"  [face][face][face]   [Yes to all] [Review]          |
+----------------------------------------------------------------------------------+
```

**Print layout** (dialog, 900×640): left a page preview (`image` over paper outline), right `propertyGrid`: layout (single, 2×2, 3×3, contact sheet 5×7, passport 35×45 mm), paper, margins, captions, crop to fill, profile, intent, copies; `[Print…]` opens the shell print dialog.

**Privacy scan** (sheet): table of findings per file (`tableView`: File · GPS · Serial · Owner · Faces · Thumbnail mismatch) with `[Strip selected]`.

### 5.4 Menus

```
File    Open… Ctrl+O · Import… Ctrl+I · Add folder to library… · New album Ctrl+N ·
        New smart album… · New encrypted album… · Export… Ctrl+Shift+E · Batch convert… ·
        Share… · Print… Ctrl+P · Show in Files · Move to trash Delete · Close Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Ctrl+C · Copy edits Ctrl+Shift+C ·
        Paste edits… Ctrl+Shift+V · Select all Ctrl+A · Invert selection · Find Ctrl+F ·
        Preferences Ctrl+,
Photo   Rating ▸ 0–5 · Label ▸ 6–9 · Pick P · Reject X · Favourite . · Rotate left Shift+R ·
        Rotate right R · Flip ▸ · Stack ▸ · Add to album ▸ · Tags… Ctrl+T · Set location… ·
        Time shift… · Edit E · Revert to original · Save a copy… · Open in Image Editor Ctrl+E
Metadata Show Ctrl+M · Edit… · Apply template ▸ · Write to files · Strip… · Privacy scan… ·
        Verify content credentials
View    Timeline 1 · Albums 2 · People 3 · Places 4 · Folders 5 · Grid G · Single Enter ·
        Compare Y · Cull Ctrl+U · Slideshow F5 · Filmstrip T · Histogram Ctrl+H · Clipping J ·
        Before/after \ · Zoom ▸ · Sidebar F9 · Inspector F4 · Full screen F11
Library Rescan F5 · Find duplicates… · Check library… · Relink missing… · Indexing ▸ ·
        Lock hidden albums Ctrl+Shift+L
Help    Help F1 · Supported formats · Shortcuts · About
```

### 5.5 Keyboard and pointer

Extra keys: `Z` 100% toggle, `Space` next (viewer) or quick look (grid), `Backspace` previous, `C` crop, `Q` heal, `W` white-balance picker, `[`/`]` brush size, `Ctrl+G` stack, `Ctrl+Shift+G` unstack, `Ctrl+D` duplicate as version, `Alt+Enter` properties.

| Context | Menu items |
|---|---|
| Grid cell | Open · Edit · Open in Image Editor · Rating ▸ · Add to album ▸ · Tags… · Copy edits · Paste edits · Export… · Share… · Strip metadata… · Move to hidden… · Show in Files · Move to trash |
| Canvas | Zoom ▸ · Copy image · Copy colour · Set as wallpaper · Before/after · Metadata |
| Album | Rename F2 · Set cover · Slideshow · Export album… · Delete album (photos stay) |
| Person | Rename · Merge with… · Hide · Not a person · Set key photo |
| Sidebar folder | Rescan · Remove from library (files stay) · Show in Files |

Drag and drop: files and folders in (import or view); grid cells onto albums, tags, people, the map; cells out as file handles plus PNG (rendered, stripped) — holding `Alt` drags the original. Gestures: pinch zoom, two-finger pan, swipe next and previous.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | "Add a folder of photos" and "Import from a device". Viewer-only use never needs a library. |
| Loading | Thumbnails fade in (no fade with reduced motion); decode progress for images over 100 MP. |
| Error | Per item: unsupported format (names the missing pack), corrupt file (decoder message), limit exceeded. |
| Offline | Not applicable to network. Offline volumes: originals greyed, thumbnails still shown. |
| Locked | Hidden and encrypted albums show a lock; content area asks to unlock. |
| Read-only | Edits stored in app data; "write to file" actions disabled with a tooltip. |
| Narrow (<640) | Sidebar overlay; inspector as a bottom sheet; grid drops to 3 columns; edit sliders full width. |
| Indexing | Status bar progress with pause; AI views show partial results with a note. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Grid, filmstrip | `gridView`, `listView`, `modelListView` |
| Canvas | `image` in `scrollArea` (phase 1); tiled canvas (G-12) for large images, crop and heal overlays |
| Inspector | `tabWidget`, `propertyGrid`, `slider`, `spinBox`, `curveEditor`, `colorPicker`, `chart` (histogram) |
| Search | `lineEdit`, `popupMenu` (suggestions) |
| Dialogs | `dialog`, `comboBox`, `checkBox`, `radioButton`, `progressBar`, `tableView`, `calendar` (date range) |
| Motion photo, clips | `videoWidget` (G-11) |

Missing: G-12 (canvas). `ME01-GAP-1`: an embeddable map panel offered by IN04 to other apps (cross-process surface embedding). `ME01-GAP-2`: token chips inside `lineEdit`. `ME01-GAP-3`: a `set-wallpaper` intent is not in PLATFORM.md section 9. HDR display path: ME03-GAP-1.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Views, selection, edit lists, search parsing, jobs |
| UI | FUSE UI | app host | Section 5 |
| `image-codec-helper` | C++23 plus vendored codec libraries | helper pool, no capabilities | Decode, encode, ICC and metadata parse; bytes in by descriptor, pixels out by shared memory |
| `raw-helper` | C++23 plus RAW library | helper, no capabilities | RAW unpack and demosaic |
| `render-helper` | C++23 | helper, `dev.gpu.compute` only | Runs the edit list on float buffers (GPU or CPU) |
| `metadata-writer` | C++23 | helper, one writable descriptor per call | Writes EXIF, XMP, IPTC into a temp file |
| `device-import-helper` | C++23 | helper, one USB handle from the USB portal | PTP and MTP |
| Index worker | ForgeScript | app host background task | Scan, hash, thumbnail, embeddings queue |
| Face and embedding jobs | — | `svc.model` | Models run in the model service, not in the app |

```
 portal handles ─► app host (ForgeScript + FUSE UI) ──────► svc.model (embeddings, faces, OCR)
                     │        │           │        └──────► IN04 map panel · SE05 container
          bytes (fd) │        │ pixels    │ edit list
               ┌─────▼─────┐  │ (shm) ┌───▼──────────┐   ┌─────────────────┐
               │ image-    ├──┘       │ render-helper│   │ metadata-writer │→ temp file
               │ codec /   │─ float ─►│ (GPU or CPU) │   └─────────────────┘
               │ raw helper│          └──────────────┘   ┌──────────────────────┐
               └───────────┘                             │ device-import-helper │← USB portal
                                                         └──────────────────────┘
```

### 6.2 Data model

Store: `~/.local/share/org.pbsd.photos/library.sqlite`; thumbnails in `~/.cache/org.pbsd.photos/thumbs/<aa>/<hash>.jxl` (256 and 1024 px); face store `faces.enc` (XChaCha20-Poly1305, key in `svc.keyring`).

| Entity | Fields |
|---|---|
| `item` | `id INTEGER PK`, `content_hash BLOB(32)`, `root_id`, `rel_path TEXT`, `size INTEGER`, `mtime INTEGER`, `mime TEXT`, `width`, `height`, `taken_at INTEGER`, `taken_tz TEXT`, `orientation INTEGER`, `rating INTEGER`, `label TEXT`, `flag TEXT`, `favourite INTEGER`, `lat REAL`, `lon REAL`, `alt REAL`, `place_id`, `camera_make TEXT`, `camera_model TEXT`, `lens TEXT`, `focal_mm REAL`, `aperture REAL`, `shutter_s REAL`, `iso INTEGER`, `stack_id`, `import_id`, `hidden INTEGER`, `missing INTEGER`, `phash INTEGER` |
| `root` | `id`, `grant_token TEXT`, `label`, `managed INTEGER`, `write_policy TEXT` |
| `edit` | `item_id`, `version INTEGER`, `name TEXT`, `ops_json TEXT`, `updated_at`, `is_current INTEGER` |
| `album` / `album_item` | `id`, `parent_id`, `name`, `kind TEXT (manual\|smart\|hidden\|encrypted)`, `rules_json`, `cover_item`, `container_ref TEXT` / `album_id`, `item_id`, `pos` |
| `tag` / `item_tag` | `id`, `parent_id`, `name` / `item_id`, `tag_id` |
| `person` / `face` | `id`, `name`, `hidden`, `key_face` / `id`, `item_id`, `person_id`, `bbox_x,y,w,h REAL`, `vector_ref INTEGER`, `confirmed INTEGER` |
| `embedding` | `item_id`, `model_id TEXT`, `dim INTEGER`, `vec BLOB` |
| `import_session` | `id`, `source_label TEXT`, `started_at`, `count`, `verified INTEGER` |
| `job` | `id`, `kind`, `state`, `progress REAL`, `payload_json` |

Edit operation record: `{"op":"exposure","v":1,"ev":0.3}`; the list order is fixed by the pipeline, not by the user, except for heal and local adjustments. `schema_version` in `meta`; migrations forward only with a kept copy.

### 6.3 File formats and protocols

All parsing runs in `image-codec-helper` unless stated. All libraries are vendored, pinned by SHA-256, built offline.

| Format | Read | Write | Library or own code | Licence |
|---|---|---|---|---|
| JPEG | yes | yes | libjpeg-turbo | IJG and BSD-3-Clause |
| PNG, APNG | yes | yes | libpng (FUSE vendors `lpng`: `FUSE:vendor/lpng`) or own decoder | libpng licence |
| WebP | yes | yes | libwebp | BSD-3-Clause |
| AVIF | yes | yes | own ISO BMFF reader (shared with ME03 demuxer) + dav1d / SVT-AV1 | BSD-2-Clause |
| HEIF/HEIC | yes (restricted pack) | optional | same reader + HEVC decoder from the restricted pack | LGPL-2.1-or-later; HEVC patents |
| JPEG XL | yes | yes | libjxl | BSD-3-Clause |
| TIFF, BigTIFF, DNG container | yes | yes (TIFF) | own C++23 reader and writer | own |
| GIF, BMP, ICO, PNM, TGA, PCX, QOI | yes | GIF, BMP | own C++23 | own |
| SVG (static subset) | yes | no | own C++23 on the FUSE vector path | own |
| OpenEXR, Radiance HDR | yes | no | own code for scanline and PIZ/ZIP; no multipart deep data | own |
| JPEG 2000 | yes | no | OpenJPEG | BSD-2-Clause |
| RAW families | yes | no | LibRaw in `raw-helper` | LGPL-2.1 or CDDL-1.0 |
| ICC profiles v2, v4 | yes | embed | own C++23 transform code; lcms2 (MIT) as the differential reference | own |
| EXIF, XMP, IPTC-IIM, maker notes | yes | yes (not maker notes) | own C++23; XMP with an XML subset parser (no entities, no DTD) | own |
| C2PA manifests (JUMBF, CBOR, COSE) | yes | optional | own code; signatures checked by SE04 | own |
| Lens profiles (Lensfun XML data) | yes | no | own reader; data CC-BY-SA 3.0 | data licence |
| PTP (ISO 15740), MTP | yes | delete only | own code in `device-import-helper` | own |
| GPX | yes | no | own code | own |
| `.pbpreset`, edit list JSON | yes | yes | ForgeScript | own |

FUSE's `stb_image` (`FUSE:vendor/stb/stb_image.h`) is not used for user files: it is in-process C.

### 6.4 Integration

- Intents handled: `open`, `view`, `pick` (photos), `share` target (import into library), `print`. Emits: `share`, `edit` (CR01), `print`, `compose` (CO01), `open` (ME03 for clips).
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.portal.usb`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.thumbnail`, `svc.media`, `svc.model`, `svc.backup`, `svc.undo-trash`, `svc.audit`.
- Offers: `org.pbsd.photos.Library1`; thumbnailers for `image/*`; the image codec helper is a platform component other apps reuse (ME03, ME04, ME05, CR01, OF03).
- Other apps: SY02, SH04, IN04 (map panel, place database), SE05 (containers), SE04 (signatures), AI02 (models), CR01, ME03 (media pipeline, section 6.5 there), ME04 (new captures appear through folder watch), UT06, IN03.
- Backing repos, Verified: FUSE has an `image` widget and vendored `stb_image` and `lpng` (`FUSE:vendor/`). No RAW, metadata or colour-management code was found under the paths read. Everything else is Proposed.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Library database, thumbnails, face store |
| `fs.portal` | Library roots, opened files, export folders |
| `net.none` | The app never uses the network |
| `proc.helper:image-codec-helper`, `raw-helper`, `render-helper`, `metadata-writer`, `device-import-helper` | Section 6.1 |
| `dev.usb:ptp`, `dev.usb:mtp` | Camera and phone import; prompt per device |
| `dev.print` | Print layouts |
| `dev.gpu.compute` | Edit pipeline on the GPU |
| `ai.local`, `svc.model` | Embeddings, faces, OCR, denoise |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.portal.usb`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.thumbnail`, `svc.media`, `svc.backup`, `svc.undo-trash`, `svc.audit` | Section 6.4 |

Not requested: any `net.host`/`net.user`, `dev.camera`, `dev.mic`, `dev.screen`, `dev.location`, `proc.exec`, `bg.*`.

| Asset | Adversary | Attack surface | Mitigation |
|---|---|---|---|
| Session and files | Crafted image, RAW, ICC, XMP, SVG | Decoders and parsers | Zero-capability helpers, decode limits, fuzzing, disposable viewing |
| Location, identity, device serials | Recipient of a shared photo | Metadata, embedded thumbnails, C2PA history | Strip-by-default export and share, privacy scan, thumbnail mismatch warning |
| Biometric data (face vectors) | Local attacker, other apps, backups | Face store | Encrypted store, key in keyring, never exported, one-switch delete, excluded from `svc.search` |
| Hidden and encrypted albums | Someone at the unlocked desktop; forensic reader | Thumbnails, index rows, recents, snapshots | Per-container storage, auto-lock, re-authentication, honest note about old snapshots |
| USB host | Malicious camera or phone | PTP/MTP parser | Device helper with one USB handle, no file access; files it returns go through the codec helper |
| Originals | Bugs in the app | Metadata writes, overwrite | Non-destructive edits, temp file + atomic rename, trash, verification after import |
| Other apps | Photos as a confused deputy | `pick` intent, bus interface | User picks every item; rendered copies; per-caller approval |

Stored: index, thumbnails, edit lists, face vectors (encrypted), embeddings. Leaves the machine: nothing. The manifest has no network capability, so this is enforced, not promised.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `viewer.background` | enum `dark\|light\|checker\|custom` | `dark` | Canvas background |
| `viewer.zoom_default` | enum `fit\|fill\|100` | `fit` | Initial zoom |
| `viewer.smooth_above_100` | bool | false | Interpolate when magnified |
| `viewer.loop_animations` | bool | true | Honour loop count when false |
| `library.write_policy` | enum `db\|sidecar\|file` | `db` | Where metadata edits go |
| `library.index_on_battery` | bool | false | Run indexing on battery |
| `library.thumb_cache_mb` | int | 4096 | Thumbnail cache limit |
| `import.copy` | bool | true | Copy instead of reference |
| `import.folder_template` | string | `{YYYY}/{MM}/{DD}` | Destination layout |
| `import.rename_template` | string | empty | Rename rule |
| `import.verify` | bool | true | Hash check after copy |
| `import.metadata_template` | ref | none | Template applied on import |
| `edit.gpu` | bool | true | Use the GPU pipeline |
| `edit.raw_demosaic` | enum `fast\|quality` | `quality` | Demosaic method |
| `edit.allow_overwrite` | bool | false | Show "overwrite original" |
| `colour.untagged` | enum `srgb\|display` | `srgb` | Assumed space for untagged files |
| `colour.intent` | enum `relative\|perceptual` | `relative` | Rendering intent |
| `export.strip` | enum `all\|keep_basic\|none` | `keep_basic` | Default metadata policy |
| `export.default_preset` | ref | `Share small` | Export preset |
| `share.original_allowed` | bool | false | Offer "share original" |
| `ai.objects`, `ai.people`, `ai.ocr` | bool | false | Local model features |
| `ai.people_in_search` | bool | false | Send people names to `svc.search` |
| `places.enabled` | bool | true | Reverse geocode offline |
| `privacy.hidden_autolock_min` | int | 5 | Idle lock for hidden albums |
| `privacy.disposable_for_downloads` | bool | true | Disposable viewing for marked files |
| `slideshow.interval_s` | int | 5 | Slide time |
| `slideshow.transition` | enum `cut\|fade` | `fade` | Transition |
| `decode.max_megapixels` | int | 1000 | Decode limit |

## 9. Accessibility and languages

- Every grid cell has a name built from date, place, people (if enabled) and tags; optional local captioning through `svc.model` fills an "image description" field the screen reader reads. The description field is also written to XMP `dc:description` on request (alt text).
- Rating, pick and label are spoken on change. Compare view announces which pane has focus.
- All edit sliders are keyboard adjustable (`Left`/`Right` 1 step, `Shift` 10 steps) and accept typed values.
- Colour labels have names and shapes, not colour alone.
- Slideshow honours reduced motion (cuts only).
- Dates, numbers and units (mm, EV, K) follow the locale; tag and album names in any script (G-03).

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start, viewer with one image | 150 ms to window, 250 ms to embedded preview, 600 ms to full 24 MP JPEG |
| Next image (preloaded) | 16 ms |
| Grid scroll | 60 fps with 200 000 items; no decode on the UI thread |
| Thumbnail build | 60 JPEG/s, 12 RAW/s on 8 cores |
| Search (indexed fields) | under 50 ms on 200 000 items; embedding search under 150 ms |
| Slider edit preview | under 33 ms at screen resolution on GPU, under 100 ms on CPU |
| Export 24 MP JPEG with edits | under 1.5 s |
| Memory | app host under 300 MB with a 200 000-item library; codec helper limit 2 GiB |
| Largest image | 65 535 × 65 535 px tiled; 1000 MP default limit |
| Idle | 0 frames, 0 wake-ups; indexing pauses on battery |

## 11. Testing and proof

Nothing has run.

- Unit: search parser, edit-list ordering, template expansion, time shift, duplicate rules, smart-album rules.
- Golden: `.fuseui` renders of each view; pixel goldens for each edit operation against reference images.
- Scripted UI: import, rate, cull, edit, export, strip, hidden album lock and unlock.
- Conformance: PNG suite, JPEG XL conformance, AVIF test images, TIFF variants, ICC round trips.
- Differential: metadata against ExifTool output on a corpus; colour transforms against lcms2; RAW against LibRaw's own tools.
- Fuzz: every own decoder and parser (TIFF, GIF, BMP family, SVG, EXIF, XMP, IPTC, ICC, C2PA, PTP, GPX); library decoders fuzzed in the helper harness.
- Privacy tests: exported files scanned for any byte of GPS, serial or thumbnail data; locked albums checked for plaintext in cache and database.
- GPU/CPU parity of the edit pipeline within a ΔE tolerance.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| TIFF, GIF, BMP-family decoders | No runtime errors; bounded memory | T2 | PRISM; fuzz | — |
| EXIF, XMP, IPTC parsers and writers | No runtime errors; writer output re-parses to the same model | T2 (parser), T3 (writer) | PRISM; round-trip property tests | Writer covers many vendor quirks |
| Metadata stripper | Output contains no field outside the allow-list | T1 | Small allow-list function over the typed model; proof plus byte scan tests | — |
| ICC transform | No runtime errors; numeric bounds | T2 | PRISM; differential | — |
| SVG subset renderer | Memory safe, no external access | T3 | Safe subset, fuzz, no-capability helper | Size |
| Vendored codecs (libjpeg-turbo, libjxl, libwebp, dav1d, LibRaw) | Contained | T4 | Helper sandbox, limits, fuzz | Third-party C/C++ |
| Edit pipeline maths | No overflow, deterministic CPU result | T2 | PRISM; goldens | GPU path is T4 |
| Hidden-album gate | Locked items unreachable from any query path | T3 | Tests on every query; code review | Needs a formal model of the store |
| App logic | Language-level memory safety | T3 | Interpreter; unit tests | — |

## 12. Build plan

### Phase 1: viewer and basic library

| ID | Work item | Depends on |
|---|---|---|
| ME01-W01 | `image-codec-helper` with JPEG, PNG, WebP, AVIF, JPEG XL, GIF, TIFF, BMP family; shared-memory pixels | G-07, FORGE-WP-27, FORGE-WP-28, FORGE-R-080 |
| ME01-W02 | Viewer window: zoom, pan, folder browsing, animation, filmstrip | G-01, G-02 |
| ME01-W03 | EXIF/XMP/IPTC read; metadata panel; strip | — |
| ME01-W04 | Library index, roots, timeline grid, albums, tags, ratings | G-09 (`svc.portal.file`, `svc.thumbnail`) |
| ME01-W05 | Edit list with crop, rotate, exposure, colour; render on CPU | — |
| ME01-W06 | Export with strip-by-default; copy; trash | `svc.undo-trash` |
| ME01-W07 | SVG subset renderer | — |
| ME01-W08 | Input ICC handling and sRGB output | — |
| ME01-W09 | Thumbnailers registered with `svc.thumbnail` | G-09 |
| ME01-W10 | Slideshow | — |

### Phase 2: full library and editing

| ID | Work item | Depends on |
|---|---|---|
| ME01-W11 | `raw-helper` and RAW development | LibRaw vendored |
| ME01-W12 | GPU edit pipeline, curves, detail, lens, heal, presets, batch | `dev.gpu.compute`, G-12 |
| ME01-W13 | Import from devices (PTP, MTP) | SH04 USB portal, VM05 |
| ME01-W14 | Metadata writing, templates, time shift, privacy scan | — |
| ME01-W15 | Search tokens, filters, smart albums, `svc.search` | G-09 |
| ME01-W16 | Embeddings, people, model state UI | G-15, FORGE-WP-29, FORGE-WP-31, AI02 |
| ME01-W17 | Places and map view | IN04, ME01-GAP-1 |
| ME01-W18 | Cull, compare, duplicates | — |
| ME01-W19 | Hidden and encrypted albums | SE05, SH03 |
| ME01-W20 | Display colour management | compositor profile API |
| ME01-W21 | Print layouts | G-16, UT06 |
| ME01-W22 | Pick intent and bus interface | G-08, SH04 |
| ME01-W23 | HEIF through the restricted pack | ME03 restricted pack decision |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| ME01-W24 | HDR display, gain maps, wide-gamut surfaces | ME03-GAP-1 |
| ME01-W25 | Local adjustments, soft proof, watermark, album export | — |
| ME01-W26 | C2PA read, verify, optional sign | SE04 |
| ME01-W27 | OCR index, similar photos, burst scoring, captions | G-15 |
| ME01-W28 | Motion photos | ME03 media pipeline |
| ME01-W29 | Multiple libraries, snapshot restore | `svc.backup` |

## 13. Open questions and risks

1. **HEIF** is the default format of many phones and needs HEVC. Without the restricted pack those photos show only a preview. Owner decision (ME03 section 13).
2. **LibRaw** is large C++ handling hostile data; it is contained, not proved. New cameras need library updates.
3. **Face recognition** is sensitive in law in some places. It is off by default and local only; whether to ship it at all is an owner decision.
4. **Snapshots and secure move**: ZFS snapshots can keep plaintext copies of photos moved into an encrypted album. The app can warn; it cannot erase snapshots without `svc.backup` cooperation.
5. **Embedded map panel** (ME01-GAP-1) needs cross-app surface embedding that no plan read here defines.
6. **Canvas widget** (G-12) decides how good crop, heal and large-image viewing can be.
7. **Colour profile per output** from the compositor is assumed; not specified in SH05 or SY01 as read.
8. **Lens data licence** (CC-BY-SA) must be checked against the distribution licence set.
9. **PTP/MTP on HardenedBSD** through a USB portal handle is Unverified.
