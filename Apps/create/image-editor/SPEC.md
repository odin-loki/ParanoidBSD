<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Image Editor

By Odin Loch

| | |
|---|---|
| ID | `CR01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.image-editor` |
| Category | create |
| Priority | P2 |
| Backing repos | FUSE (UI, compute kernels), FORGE (ForgeScript, model service) |
| Status | Planned. Nothing built. |

Labels: **Verified** = read in a repo at the cited path. **Proposed** = design in this plan. **Unverified** = could not be read.

## 1. Purpose

A layered raster image editor for photographers, illustrators and designers. It covers retouching, compositing, painting and export on documents from icons to multi-gigapixel scans. It replaces GIMP, Krita and Photoshop on this OS and fills the gap left by removing Qt and GTK apps.

## 2. Scope

**In scope:** layered raster documents; masks and blend modes; selections; a pressure brush engine; retouching; transforms; filters; adjustments; colour management; text and simple vector shape layers; RAW development into a document; animation frames; slices and export presets; ForgeScript actions and batch runs; local AI tools through `svc.model`.

**Out of scope:**

| Item | Owner |
|---|---|
| Photo library, albums, face search, quick one-click edits | ME01 Photos |
| Vector illustration and diagrams as the main document type | OF08 Draw |
| Page layout, multi-page documents | OF04 Writer, OF03 Document Viewer |
| Screenshots and annotation of captures | ME05 Screen Capture |
| Video frames as a timeline, motion graphics | CR02 Video Editor |
| 3D painting on meshes, texture baking | CR04 Studio |
| Model import, signing and benchmarking | AI02 Model Manager |
| Printer set-up and queues | UT06 Printers and Scanners (print dialog is SH04) |
| Font installation | UT03 Fonts and Characters |

## 3. Users and top tasks

Personas: **Photographer** (RAW, retouch, colour-accurate export). **Illustrator** (tablet, brushes, many layers). **Designer** (web and app assets, slices, text). **Scientist or archivist** (huge scans, 16 and 32-bit data, exact pixels). **Casual user** (crop, resize, remove an object).

| # | Task | Target steps |
|---|---|---|
| T1 | Open a JPEG, crop, resize, export as WebP | 5 |
| T2 | Remove an object with content-aware fill | 3 (lasso, Fill, accept) |
| T3 | Select the subject and replace the background | 4 |
| T4 | Start painting on a new A4 300 dpi canvas with a tablet | 2 |
| T5 | Add a curves adjustment layer limited to a mask | 3 |
| T6 | Develop a RAW file and open it as a 16-bit document | 3 |
| T7 | Export all slices at 1x, 2x, 3x | 2 |
| T8 | Run a saved action on a folder of 500 files | 4 |
| T9 | Soft-proof for a CMYK press profile and export TIFF | 4 |
| T10 | Import a PSD and keep its layers editable | 1 |
| T11 | Return to any earlier state from the history tree | 2 |
| T12 | Open a 100 000 × 100 000 px scan and pan at 60 fps | 1 |
| T13 | Open an untrusted file in a disposable sandbox | 2 |
| T14 | Strip metadata and sign the exported file | 2 |

## 4. Features

Priority: M must, S should, C could. Phase 1 usable core, 2 full feature, 3 advanced.

### 4.1 Documents and canvas

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F001 | New document | Width, height, unit (px, mm, in, pt), resolution, bit depth, colour model, profile, background (white, black, transparent, colour). Presets for paper, screen, icon, video frame. | M | 1 |
| CR01-F002 | Tabs and windows | One document per tab; tear-off to a window; two views of one document at different zoom. | M | 1 |
| CR01-F003 | Zoom and pan | 1% to 6400%, zoom at pointer, fit, fill, 100%, pixel grid above 800%. Space-drag pans. | M | 1 |
| CR01-F004 | View rotation and mirror | Rotate the view in 15° steps or freely; mirror view; neither changes pixels. | S | 2 |
| CR01-F005 | Canvas size | Extend or crop the canvas with a 9-point anchor; relative mode. | M | 1 |
| CR01-F006 | Image size | Resample with nearest, bilinear, bicubic, Lanczos-3, or a local upscaling model (4.11 Upscale). | M | 1 |
| CR01-F007 | Crop tool | Free, fixed ratio, fixed size; rule-of-thirds overlay; straighten by drawing a horizon line; non-destructive crop keeps hidden pixels. | M | 1 |
| CR01-F008 | Rotate and flip image | 90°, 180°, arbitrary angle, flip horizontal or vertical for the whole document. | M | 1 |
| CR01-F009 | Tiled storage | All pixel data lives in 256 × 256 tiles with copy-on-write; untouched tiles are shared between history states. | M | 1 |
| CR01-F010 | Huge images | Documents up to 2^31 px per side; tiles page to a scratch file in `~/.cache`; mip pyramid for fast zoomed-out view. | M | 2 |
| CR01-F011 | Bit depths | 8-bit integer, 16-bit integer, 16-bit half float, 32-bit float per channel; convert with optional dither. | M | 1 |
| CR01-F012 | Colour models | RGB, greyscale, indexed (palette up to 256); CMYK as soft-proof and export only in phase 2, as a working model in phase 3. | M | 2 |
| CR01-F013 | Rulers, guides, grids | Rulers in any unit; draggable guides; guide sets; pixel, custom and isometric grids; snap to guide, grid, layer edge, canvas centre. | M | 1 |
| CR01-F014 | Measure and colour sampler | Measure distance and angle; up to 8 pinned colour samplers with read-outs in RGB, HSL, Lab, CMYK (proof). | S | 2 |
| CR01-F015 | Navigator | Thumbnail with the view rectangle; drag to pan. | S | 1 |
| CR01-F016 | Multiple artboards | Named rectangles on one canvas, each exportable; optional. | C | 3 |

### 4.2 Layers

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F017 | Raster layers | Own bounds, offset, opacity, fill opacity, blend mode, lock (pixels, position, alpha, all). | M | 1 |
| CR01-F018 | Groups | Nested to 32 levels; pass-through or isolated blending; collapse; colour labels. | M | 1 |
| CR01-F019 | Adjustment layers | Any adjustment of 4.8 as a non-destructive layer with its own mask. | M | 1 |
| CR01-F020 | Fill layers | Solid colour, gradient, pattern; editable at any time. | M | 2 |
| CR01-F021 | Text layers | Editable text (4.10); rasterised only on request. | M | 2 |
| CR01-F022 | Vector shape layers | Rectangle, ellipse, polygon, line, path with fill and stroke; stays editable; boolean operations. | S | 2 |
| CR01-F023 | Embedded smart layers | A layer that wraps another document inside the file; transforms and filters apply non-destructively. | S | 2 |
| CR01-F024 | Linked smart layers | A smart layer that refers to an external file by portal handle; shows a stale or missing badge; relink; hash recorded. | S | 3 |
| CR01-F025 | Smart filters | Filters on a smart layer stay in a list; reorder, hide, edit, mask. | S | 3 |
| CR01-F026 | Clipping | Clip one or more layers to the alpha of the layer below. | M | 1 |
| CR01-F027 | Layer styles | Drop shadow, inner shadow, outer and inner glow, stroke, colour, gradient and pattern overlay, bevel; copy and paste styles. | S | 2 |
| CR01-F028 | Blend modes | Normal, dissolve, darken, multiply, colour burn, linear burn, darker colour, lighten, screen, colour dodge, linear dodge, lighter colour, overlay, soft light, hard light, vivid light, linear light, pin light, hard mix, difference, exclusion, subtract, divide, hue, saturation, colour, luminosity. Formulas documented. | M | 1 |
| CR01-F029 | Blend-if | Blend ranges on this layer and the underlying layer, with split sliders. | C | 3 |
| CR01-F030 | Layer operations | New, duplicate, delete, merge down, merge visible, flatten, stamp visible, rasterise, convert to smart layer. | M | 1 |
| CR01-F031 | Arrange and align | Reorder by drag; align and distribute to selection, canvas or key layer. | M | 1 |
| CR01-F032 | Layer search and filter | Filter the panel by name, kind, label colour, blend mode, visibility. | S | 2 |
| CR01-F033 | Layer comps | Saved sets of visibility, position and style; export each comp. | C | 3 |

### 4.3 Masks and channels

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F034 | Pixel masks | 8 or 16-bit mask per layer or group; paint, invert, apply, delete, disable, unlink from layer. | M | 1 |
| CR01-F035 | Vector masks | Path-based mask with feather. | S | 2 |
| CR01-F036 | Mask density and feather | Non-destructive sliders on any mask. | S | 2 |
| CR01-F037 | Channels panel | View and edit colour channels and alpha; save a selection as a channel; spot channels in phase 3. | S | 2 |
| CR01-F038 | Quick mask | Paint a selection as a red overlay (`Q`). | M | 1 |
| CR01-F039 | Mask from selection, luminance or model | Create from the current selection, from layer luminance, or from an AI matte (4.4 AI subject select). | M | 2 |

### 4.4 Selections

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F040 | Marquee | Rectangle, ellipse, single row, single column; fixed ratio or size; from centre. | M | 1 |
| CR01-F041 | Lasso | Freehand, polygonal, magnetic (edge following). | M | 1 |
| CR01-F042 | Magic wand | Tolerance, contiguous, sample all layers, anti-alias. | M | 1 |
| CR01-F043 | Quick select | Brush that grows a region by edge-aware graph cut; add and subtract. | M | 2 |
| CR01-F044 | Select by colour | Colour range with fuzziness and a preview; presets for highlights, midtones, shadows, skin tones. | M | 2 |
| CR01-F045 | Selection maths | New, add, subtract, intersect with Shift, Alt, Shift+Alt. | M | 1 |
| CR01-F046 | Modify | Invert, grow, shrink, feather, border, smooth, transform selection. | M | 1 |
| CR01-F047 | Refine edge | Radius, smooth, feather, contrast, shift edge, decontaminate colours; output to selection, mask or new layer. | S | 2 |
| CR01-F048 | AI subject select | One-click subject, sky or background matte from a local segmentation model through `svc.model`; a point or box prompt refines it. Works offline. | S | 2 |
| CR01-F049 | Paths to selection | Pen tool path becomes a selection and back. | S | 2 |
| CR01-F050 | Save and load selections | Stored as channels in the document. | S | 2 |

### 4.5 Painting and brushes

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F051 | Brush tool | Size 1–5000 px, hardness, opacity, flow, spacing, angle, roundness, build-up. | M | 1 |
| CR01-F052 | Tablet input | Pressure, tilt X and Y, rotation, eraser end, barrel buttons; per-device pressure curve. Needs FUSE-WP-08. | M | 1 |
| CR01-F053 | Dynamics | Map pressure, tilt, speed, direction, random and fade to size, opacity, flow, angle, scatter, colour jitter, each with a curve. | M | 2 |
| CR01-F054 | Custom tips | Tip from a greyscale image or the current selection; dual tip; texture with depth. | S | 2 |
| CR01-F055 | Stroke smoothing | Stabiliser with lag distance; pulled-string mode; catch-up on pen lift. | S | 2 |
| CR01-F056 | Wet-ink preview | The stroke in progress is drawn on an overlay in the same frame as the pen sample. | M | 1 |
| CR01-F057 | Pencil and eraser | Aliased pencil; eraser to transparency or background; erase to history. | M | 1 |
| CR01-F058 | Fill and gradient | Bucket with tolerance; gradients: linear, radial, angle, reflected, diamond; editable stops and midpoints; dither. | M | 1 |
| CR01-F059 | Smudge, blur, sharpen, dodge, burn, sponge | Local tools with strength and range (shadows, midtones, highlights). | S | 2 |
| CR01-F060 | Mixer brush | Wet paint that picks up canvas colour; load, mix, wet, flow. | C | 3 |
| CR01-F061 | Symmetry | Mirror horizontal, vertical, both, radial 2–32, with a movable axis. | S | 2 |
| CR01-F062 | Brush presets and sets | Presets in a searchable grid; tags; import `.abr` (read), `.gbr` and `.kpp` tips (read); own open preset format (JSON + PNG). | S | 2 |
| CR01-F063 | Colour panel | `colorPicker` plus wheel, sliders (RGB, HSL, HSV, Lab), hex, recent colours, swatches (`.gpl`, `.ase` read), eyedropper with sample size. | M | 1 |
| CR01-F064 | Pattern and clone-stamp painting | Paint with a pattern; pattern editor with seamless preview. | C | 3 |

### 4.6 Retouching

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F065 | Clone stamp | Alt-click source; aligned or not; sample current, below, all; source overlay; up to 5 sources. | M | 1 |
| CR01-F066 | Healing brush | Clone with texture and tone blending by Poisson solve. | M | 2 |
| CR01-F067 | Spot heal | One click; picks the source patch itself. | M | 2 |
| CR01-F068 | Patch tool | Drag a selection to a source or destination; content-aware mode. | S | 2 |
| CR01-F069 | Content-aware fill (classic) | Patch-match fill of the selection; sampling-area overlay; output to a new layer. No model needed. | M | 2 |
| CR01-F070 | Content-aware fill (model) | Inpainting with a local model through `svc.model`; runs offline; shows the model name and hash; result is a new layer. | S | 3 |
| CR01-F071 | Red-eye and blemish | One-click red-eye; frequency-separation helper action. | C | 2 |
| CR01-F072 | Content-aware scale and move | Seam carving with protect mask; move an object and fill its hole. | C | 3 |

### 4.7 Transforms

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F073 | Free transform | Move, scale, rotate, skew with handles and numeric fields; reference point; constrain with Shift. | M | 1 |
| CR01-F074 | Perspective and distort | Four-corner perspective; free distort. | M | 2 |
| CR01-F075 | Warp | Mesh warp 2×2 to 16×16; presets (arc, flag, fisheye); cage warp. | S | 2 |
| CR01-F076 | Liquify | Push, twirl, pucker, bloat, reconstruct, freeze mask; live mesh preview; saved meshes. | S | 3 |
| CR01-F077 | Perspective crop and lens correction | Fix keystone; barrel, pincushion, vignette, chromatic aberration with manual values. | S | 2 |
| CR01-F078 | Resampling choice | Nearest, bilinear, bicubic, Lanczos per transform; preview at reduced quality while dragging. | M | 1 |
| CR01-F079 | Transform again and repeat | Repeat last transform; step-and-repeat with a copy. | C | 2 |

### 4.8 Adjustments

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F080 | Levels | Per channel; black, white, gamma, output; histogram; auto with clip percentage. | M | 1 |
| CR01-F081 | Curves | Per channel and luminance; up to 16 points; `curveEditor` with histogram underlay; on-image picker. | M | 1 |
| CR01-F082 | Hue, saturation, lightness | Master and six ranges with adjustable range edges; colourise. | M | 1 |
| CR01-F083 | Colour balance | Shadows, midtones, highlights; preserve luminosity. | M | 1 |
| CR01-F084 | Brightness, contrast, exposure, vibrance | Exposure in stops for float documents. | M | 1 |
| CR01-F085 | Black and white, channel mixer, selective colour, photo filter | Standard controls; presets. | S | 2 |
| CR01-F086 | Invert, threshold, posterise, gradient map | As adjustments or adjustment layers. | S | 1 |
| CR01-F087 | LUTs | Apply 1D and 3D LUTs: `.cube`, `.3dl`, Hald CLUT PNG; tetrahedral interpolation; strength slider; export the current adjustment stack as `.cube`. | S | 2 |
| CR01-F088 | Shadows and highlights, HDR tone mapping | Local tone operators for 32-bit documents (Reinhard, filmic, local Laplacian). | S | 3 |
| CR01-F089 | Histogram and scopes | RGB, luminance and per-channel histogram; waveform; clipping warning overlay. | M | 1 |
| CR01-F090 | Auto tone, contrast, colour | Deterministic algorithms; each shows what it changed. | C | 2 |

### 4.9 Filters

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F091 | Blur | Gaussian, box, motion, radial, zoom, lens (bokeh shape), surface, tilt-shift. | M | 1 |
| CR01-F092 | Sharpen | Unsharp mask, smart sharpen (deconvolution), high pass. | M | 1 |
| CR01-F093 | Noise | Add (uniform, Gaussian, film grain), median, despeckle, dust and scratches, bilateral and non-local-means denoise. | M | 2 |
| CR01-F094 | Distort | Displace, ripple, wave, twirl, pinch, spherise, polar coordinates, shear. | S | 2 |
| CR01-F095 | Stylise | Emboss, find edges, oil paint, pixelate, mosaic, halftone, posterise edges, glow. | S | 2 |
| CR01-F096 | Render | Clouds, noise fields (Perlin, simplex, Worley), gradient, checker, lens flare. | C | 3 |
| CR01-F097 | Custom convolution | 3×3 to 9×9 kernel with scale and offset. | C | 2 |
| CR01-F098 | GPU execution | Filters are single-source kernels that run on CPU or on GPU through FUSE compute (`dev.gpu.compute`). Results must match the CPU reference within a stated tolerance. | S | 2 |
| CR01-F099 | Live preview | Preview on canvas at view resolution; split before/after; full-quality on commit; progress and cancel. | M | 1 |
| CR01-F100 | Filter gallery and last filter | Browse with thumbnails; `Ctrl+Alt+F` repeats the last filter. | S | 2 |
| CR01-F101 | Script filters | Signed ForgeScript filters that compose built-in kernels (no per-pixel script loops on large images). See 4.14. | S | 3 |

### 4.10 Text

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F102 | Point and area text | Click for point text, drag for a box; edit in place on canvas. | M | 2 |
| CR01-F103 | Character and paragraph | Family, style, size, leading, tracking, kerning, baseline shift, colour, align, justify, indents, OpenType features. Needs FULL text (G-03). | M | 2 |
| CR01-F105 | Text on path and warp | Text follows a path; warp presets. | C | 3 |
| CR01-F106 | Missing fonts | Document lists fonts used; missing fonts show a badge and a substitute; link to UT03. | S | 2 |

### 4.11 Local AI tools (all through `svc.model`, all offline)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F104 | Upscale | 2× and 4× super-resolution on tiles with overlap; output as a new document. | C | 3 |
| CR01-F107 | Denoise and deblur | Model denoise with a strength slider; result on a new layer. | C | 3 |
| CR01-F108 | Depth map | Monocular depth as a channel; drives lens blur. | C | 3 |
| CR01-F109 | Model disclosure | Every AI tool shows model name, version, BLAKE3 hash and signer before first use; no model, no tool (button disabled with the reason). | M | 2 |
| CR01-F110 | Provenance tag | AI-made pixels are recorded in the history entry and, on request, in export metadata. | S | 2 |

### 4.12 RAW development

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F111 | RAW open | DNG first; other camera formats as the decoder supports them. The file is parsed in the `raw-develop` helper only. | S | 2 |
| CR01-F112 | Develop controls | White balance, exposure, highlights, shadows, contrast, tone curve, demosaic method, noise reduction, sharpening, lens profile, crop. | S | 2 |
| CR01-F113 | Output | Open as a 16-bit or 32-bit linear document, or as a smart layer that keeps the develop settings editable. | S | 2 |
| CR01-F114 | Sidecar | Settings saved in an open JSON sidecar beside the RAW when the folder was granted, else inside the document. | C | 3 |

### 4.13 Colour management

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F115 | Working spaces | sRGB, Display P3, Adobe RGB (1998) compatible, Rec. 2020, ProPhoto compatible, linear variants, ACEScg; grey gamma 2.2. | M | 1 |
| CR01-F116 | Embedded profiles | Read ICC v2 and v4; policy on open: keep, convert, or ask; mismatch shown in the status bar. | M | 1 |
| CR01-F117 | Assign and convert | Assign profile; convert with intent (perceptual, relative, saturation, absolute) and black point compensation. | M | 2 |
| CR01-F118 | Display transform | Canvas colours go to the display profile through FUSE-WP-26. | M | 1 |
| CR01-F119 | Soft proofing | Proof profile, intent, paper white; gamut warning overlay. | S | 2 |
| CR01-F120 | HDR | 32-bit scene-referred editing; HDR display output when the surface supports it (FUSE-WP-11); PQ and HLG export. | S | 3 |
| CR01-F121 | CMYK export | Convert on export to TIFF or JPEG with a chosen press profile; total ink limit check. | S | 2 |

### 4.14 History, actions and automation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F122 | Undo and redo | Unlimited within the memory and scratch budget. | M | 1 |
| CR01-F123 | Non-linear history | History is a tree: undo then a new edit makes a branch, nothing is lost. Jump to any node; name and pin snapshots. | M | 2 |
| CR01-F124 | History brush | Paint from any history node. | C | 3 |
| CR01-F125 | Actions | Record steps as a ForgeScript file; edit it as text; play with a shortcut; insert stops and prompts. | S | 2 |
| CR01-F126 | Batch | Run an action on a folder picked through the portal; output naming pattern; skip, overwrite or version; log of results. | S | 2 |
| CR01-F127 | Script console | Run ForgeScript against the document API; scripts are confined to the app's capabilities. | S | 3 |
| CR01-F128 | Plugins policy | Only signed ForgeScript plugins (`integrity.scripts = required`). No native plugins. Unsigned scripts run only in developer mode with a banner. | M | 2 |
| CR01-F129 | Command-line batch | `image-editor --run action.fs --in <handle>`; used by SY02 Files' context menu through an intent. | C | 3 |

### 4.15 Files, import, export

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F130 | Native format | OpenRaster container with documented extensions (section 6.2). Default for Save. | M | 1 |
| CR01-F131 | Import layered | PSD and PSB, XCF, KRA, plain ORA: layers, groups, masks, blend modes, text as text where possible; an import report lists what was flattened. | M | 2 |
| CR01-F132 | Flat formats | Read and write PNG, JPEG, WebP, AVIF, JPEG XL, TIFF, OpenEXR; read GIF, BMP, TGA, HEIF (if a decoder is licensed), PNM, QOI, SVG (rasterised). | M | 1 |
| CR01-F133 | Export dialog | Format, quality, size, profile, metadata policy, preview with file size estimate, compare 2-up. | M | 1 |
| CR01-F134 | Export presets | Named presets; "export as before" (`Ctrl+Shift+E`). | S | 2 |
| CR01-F135 | Slices | Rectangles or layer-based slices; per-slice format and scales (1x, 2x, 3x); name pattern. | S | 2 |
| CR01-F136 | Metadata | View and edit EXIF, IPTC, XMP; strip all, strip location, or keep on export (default: strip location). | M | 2 |
| CR01-F137 | Sign export | Detached Ed25519 signature over the exported file with a key from `svc.keyring` (SE04). | C | 3 |
| CR01-F138 | Print | Send to the print portal as PDF with profile and scaling. Needs G-16. | S | 2 |
| CR01-F139 | Acquire | Pull an image from a scanner or camera through the `scan` intent (UT06, ME04). | C | 3 |
| CR01-F140 | Clipboard and drag | Copy merged, copy layer, paste as new layer, document or mask; PNG is always offered beside the native format. | M | 1 |

### 4.16 Animation frames

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F141 | Frame timeline | Layers gain a frame axis; per-frame cels; hold frames; frame rate; loop. | C | 3 |
| CR01-F142 | Onion skin | Previous and next frames tinted, count and opacity adjustable. | C | 3 |
| CR01-F143 | Export animation | Animated WebP, APNG, AVIF sequence, GIF, PNG sequence; hand off to CR02 for video. | C | 3 |

### 4.17 Security, recovery, settings, accessibility

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR01-F144 | Sandboxed decoding | Every file is decoded in the no-capability `image-codec` helper; the app never parses file bytes. | M | 1 |
| CR01-F145 | Disposable open | "Open in a disposable sandbox": a fresh helper and a read-only view; nothing is written until the user chooses Keep. | S | 2 |
| CR01-F146 | Trust badge | Status bar shows whether the file carried a valid signature, an unknown one, or none. | S | 2 |
| CR01-F147 | Autosave | Changed tiles are journalled every 30 s and on idle; restore after a crash offers each document. | M | 1 |
| CR01-F148 | Versions | "Save version" keeps earlier saves through `svc.backup` snapshots; browse and restore. | C | 3 |
| CR01-F149 | Scratch limits | User-set memory and scratch caps; a warning before they are reached; never fills the disk. | M | 1 |
| CR01-F150 | Workspaces | Saved dock layouts: Photo, Paint, Design, Minimal; per-document view state. | S | 2 |
| CR01-F151 | Shortcut editor | All actions rebindable; single-key tool shortcuts; import a familiar key set. | M | 1 |
| CR01-F152 | Keyboard-only editing | Move selections and layers with arrows; numeric transform; every panel reachable. | M | 1 |
| CR01-F153 | Search index | Contributes document title, layer names and text-layer content to `svc.search`; off by default. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Main window (workbench with docks)
├─ Header bar: menu, title, search / command palette, badges
├─ Tool options bar (changes with the tool)
├─ Toolbox (left)
├─ Document tabs → Canvas view (rulers, guides, canvas)
├─ Docks (right, bottom): Layers · Channels · Paths · History · Colour · Swatches · Brushes ·
│                         Brush settings · Adjustments · Properties · Histogram · Navigator ·
│                         Character · Paragraph · Actions · Info · Frames (phase 3)
├─ Status bar
├─ Dialogs
│   ├─ New document          ├─ Image size / Canvas size     ├─ Export
│   ├─ Filter dialog (generic, with preview)                  ├─ Levels / Curves / HSL …
│   ├─ Refine edge workspace ├─ Liquify workspace             ├─ RAW develop workspace
│   ├─ Content-aware fill workspace                           ├─ Colour settings
│   ├─ Convert profile       ├─ Batch                         ├─ Import report
│   ├─ Layer style           ├─ Brush preset manager          ├─ Slices export
│   ├─ Preferences           └─ Keyboard shortcuts
├─ Popovers: brush picker, colour picker, gradient editor, blend mode list, guide position
└─ Sheets (narrow window): layers, properties
```

### 5.2 Main window

Shown at 1440 × 900, compact density. States: empty, loading, error, read-only, locked.

```
+------------------------------------------------------------------------------------------+
| [=] Image Editor — poster.ora*            [search Ctrl+K]            [gpu] [▁] [▢] [✕]  |(1)
+------------------------------------------------------------------------------------------+
| Brush: [●24 v] Mode [Normal v] Opacity [100%] Flow [80%] Smooth [10] [sym v] [pressure]  |(2)
+----+------------------------------------------------------------------+------------------+
| ↖  | poster.ora* × | scan.tif × | +                                   | Colour           |(6)
| ▭  +--0----200----400----600----800----1000---1200--------------------+ [wheel] [sliders]|
| ◌  | 0                                                                | #4C8DFF  [fg][bg]|
| ✎  |                                                                  +------------------+
| ▨  | 2      +--------------------------------------------+           | Layers           |(7)
| ✚  | 0      |                                            |           | [Normal v] 100%  |
| ⌫  | 0      |                                            |           | 👁 ▸ Group: title |
| ▣  |        |                 canvas (4)                 |           | 👁   T Headline   |
| T  | 4      |                                            |           |>👁   ▨ Photo  [m] |
| ⬠  | 0      |                                            |           | 👁   ◐ Curves [m] |
| ✂  | 0      |                                            |           | 👁   ▨ Background |
| 🔍 |        +--------------------------------------------+           | [+][grp][adj][m][🗑]|
| ✋ | 6                                                                +------------------+
|(3) | 0                                                    (5)         | History          |(8)
| ■□ |                                                                  | ├ Open           |
|    |                                                                  | ├ Curves layer   |
|    |                                                                  | │ └ Brush (branch)|
|    |                                                                  | └>Brush stroke   |
+----+------------------------------------------------------------------+------------------+
| 66.7% | 4961×7016 px 300 dpi | RGB 16 | sRGB | Layer: Photo | scratch 1.2/8 GiB | ✓ saved |(9)
+------------------------------------------------------------------------------------------+
```

1. **Header bar.** `[gpu]` badge shows while `dev.gpu.compute` is in use; a lock shows for an encrypted source. No `[net]` badge ever: the app has `net.none`.
2. **Tool options bar**, 36 px. Content depends on the active tool. Built from `comboBox`, `spinBox`, `slider`, `toolButton`.
3. **Toolbox**, 44 px, one or two columns. Each `toolButton` holds a tool family (long press opens the family). Foreground and background colour wells at the bottom.
4. **Canvas.** The G-12 canvas widget. Checkerboard for transparency, pasteboard in `--bg-0`.
5. **Rulers and guides** on the top and left edges, 18 px; drag from a ruler to make a guide.
6. **Colour dock**, 280 px wide column of docks. Any dock can float, tab or hide.
7. **Layers dock.** `treeView` with eye, kind icon, name, mask thumbnail `[m]`, lock. Row height 32 px (40 px with large thumbnails).
8. **History dock.** `treeView` showing the history tree; the current node is marked `>`.
9. **Status bar.** Zoom, size, colour model and depth, profile (click to change), active layer, scratch use, save state. Long jobs show a `progressBar` here.

### 5.3 Other views and dialogs

**New document** (640 × 480). States: default, invalid size (error under the field).

```
+---------------------------- New document ----------------------------+
| Presets            | Name    [Untitled-1______________]              |
| > Recent           | Width   [2480__] [px v]   Height [3508__]       |
|   Paper            | Resolution [300__] [px/in v]   Orientation [▯][▭]|
|   Screen           | Colour  [RGB v]  Depth [16-bit integer v]       |
|   Icon             | Profile [sRGB IEC61966-2.1 v]                   |
|   Video frame      | Background ( ) White (•) Transparent ( ) Colour |
|   Saved…           | Memory: 139 MiB per layer                       |
|                    |                         [Cancel] [Create]       |
+----------------------------------------------------------------------+
```

**Export** (960 × 640). States: rendering preview, too large for format (error with the limit), read-only target.

```
+--------------------------------- Export ---------------------------------------------+
| Preset [Web WebP 85 v] [Save preset…]                     | Format [WebP v]          |
| +--------------------------+--------------------------+   | Quality [85__] [lossless]|
| | original                 | export preview           |   | Size [100%] 2480×3508    |
| |                          |                          |   | Resample [Lanczos v]     |
| |            (2-up, synced zoom and pan)              |   | Profile ( ) keep (•) sRGB|
| |                          |                          |   | Metadata [Strip location v]|
| +--------------------------+--------------------------+   | [ ] Sign with key […]    |
| 31.8 MiB → est. 412 KiB                                   | [ ] Export slices (6)    |
|                                                 [Cancel] [Export…]                   |
+--------------------------------------------------------------------------------------+
```

**Curves** (as a properties dock or a dialog, 420 × 520). States: no histogram yet (loading), clipped channel warning.

```
+------------- Curves -------------+
| Preset [Custom v]  Channel [RGB v]|
| +------------------------------+ |
| |            ·                 | |  curveEditor over a histogram
| |        ·                     | |
| |    ·                         | |
| +------------------------------+ |
| In [128] Out [140]  [✎ on-image] |
| [Auto] [Reset]   [x] Preview     |
|            [Cancel] [Apply]      |
+----------------------------------+
```

**Content-aware fill workspace** (full window). States: computing, model missing (model mode disabled with the reason), no selection (empty).

```
+-----------------------------------------------------------------------------------------+
| Content-aware fill                                              [Cancel] [Apply]        |
+----------------------------------------------+---------------------+--------------------+
| canvas with sampling-area overlay (green)    | result preview      | Method             |
| [brush + / −] to edit the sampling area      |                     | (•) Patch match    |
|                                              |                     | ( ) Local model    |
|                                              |                     |   name, hash, signer|
|                                              |                     | Colour adapt [v]   |
|                                              |                     | Rotation [None v]  |
|                                              |                     | [x] Scale [ ] Mirror|
|                                              |                     | Output [New layer v]|
+----------------------------------------------+---------------------+--------------------+
```

**RAW develop workspace** (full window). States: decoding, unsupported camera (error with the maker and model read), read-only.

```
+-----------------------------------------------------------------------------------------+
| RAW develop — IMG_0142.dng   [before/after]             [Cancel] [Open as smart] [Open] |
+----------------------------------------------------------------------+------------------+
|                                                                      | Histogram        |
|                    preview (canvas, fit)                             | White balance    |
|                                                                      |  Temp [5200] Tint|
|                                                                      | Exposure [+0.30] |
|                                                                      | Highlights Shadows|
|                                                                      | Tone curve       |
|                                                                      | Detail: NR, sharpen|
|                                                                      | Lens · Crop      |
+----------------------------------------------------------------------+------------------+
| ISO 200 · 1/250 · f/5.6 · 35 mm · 6000×4000 · decoded in sandbox ✓                      |
+-----------------------------------------------------------------------------------------+
```

**Batch** (720 × 520). States: running (progress per file), finished with errors (list stays).

```
+------------------------------- Batch -------------------------------+
| Action   [Resize and watermark v]                [Edit script…]     |
| Source   [Folder… ~/Pictures/shoot (512 files)]  [x] Subfolders     |
| Output   [Folder… ~/Pictures/out]   Name [{name}_{w}w.{ext}]        |
| If exists ( ) skip (•) version ( ) overwrite                        |
| Format   [Use export preset: Web WebP 85 v]                         |
| +-----------------------------------------------------------------+ |
| | file            | status      | time   | note                   | |
| | a001.dng        | ✓ done      | 1.2 s  |                        | |
| | a002.dng        | ✕ failed    |        | decoder: truncated     | |
| +-----------------------------------------------------------------+ |
|                                             [Close] [Run]           |
+---------------------------------------------------------------------+
```

**Import report** (sheet after opening PSD, XCF or KRA): a `tableView` of layer name, kind, result (kept, rasterised, dropped) and reason; buttons "Keep", "Open flattened instead".

**Brush settings dock**: tip shape preview, a `listView` of dynamics rows (source → target with a small `curveEditor`), texture, dual tip, smoothing; a stroke preview strip at the bottom.

### 5.4 Menus

```
File    New Ctrl+N · Open Ctrl+O · Open Recent ▸ · Open in Sandbox Ctrl+Alt+O · Open as Layers
        Develop RAW… · Acquire ▸ · Save Ctrl+S · Save As Ctrl+Shift+S · Save Version
        Export… Ctrl+E · Export As Before Ctrl+Shift+E · Export Slices… · Batch…
        Document Info… · Print Ctrl+P · Close Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · History ▸ · Cut/Copy/Paste · Copy Merged Ctrl+Shift+C
        Paste as New Document Ctrl+Alt+V · Paste Into Mask · Fill… Shift+F5 · Stroke…
        Content-Aware Fill… · Free Transform Ctrl+T · Transform ▸ (Perspective, Distort, Warp)
        Define Brush · Define Pattern · Colour Settings… · Shortcuts… · Preferences Ctrl+,
Image   Mode ▸ (RGB, Greyscale, Indexed; 8/16/32-bit) · Assign Profile… · Convert Profile…
        Image Size Ctrl+Alt+I · Canvas Size Ctrl+Alt+C · Crop to Selection · Trim…
        Rotate ▸ · Flip ▸ · Adjustments ▸ (Levels Ctrl+L, Curves Ctrl+M, HSL Ctrl+U, Colour
        Balance Ctrl+B, Invert Ctrl+I, …) · Auto ▸
Layer   New ▸ · Duplicate Ctrl+J · Delete · Group Ctrl+G · Ungroup Ctrl+Shift+G · Mask ▸
        Clip Ctrl+Alt+G · Style… · Smart Layer ▸ · Rasterise · Arrange ▸ · Align ▸
        Merge Down Ctrl+E (when a layer is focused) · Merge Visible Ctrl+Shift+M · Flatten
Select  All Ctrl+A · None Ctrl+D · Reselect Ctrl+Shift+D · Invert Ctrl+Shift+I · Subject
        Sky · Colour Range… · Refine Edge Ctrl+Alt+R · Modify ▸ · Transform Selection
        Quick Mask Q · Save… · Load…
Filter  Last Filter Ctrl+Alt+F · Gallery… · Blur ▸ · Sharpen ▸ · Noise ▸ · Distort ▸
        Stylise ▸ · Render ▸ · Custom… · Liquify Ctrl+Shift+X · Lens Correction… · Scripts ▸
View    Zoom In/Out/Fit/100% Ctrl++ / Ctrl+- / Ctrl+0 / Ctrl+1 · Rotate View R · Mirror View
        Proof Colours Ctrl+Y · Gamut Warning Ctrl+Shift+Y · Rulers Ctrl+R · Guides Ctrl+;
        Grid Ctrl+' · Snap Ctrl+Shift+; · Pixel Grid · New View · Full Screen F11
Window  Workspace ▸ · (each dock, checkable) · Reset Layout · Tool Options · Status Bar
Help    Image Editor Help F1 · Shortcuts · About
```

### 5.5 Keyboard and pointer

Tool keys (single key, no modifier, only when the canvas has focus): `V` move, `M` marquee, `L` lasso, `W` wand and quick select, `C` crop, `I` eyedropper, `J` heal, `B` brush, `S` clone, `E` eraser, `G` fill and gradient, `O` dodge and burn, `P` pen, `T` text, `U` shape, `H` hand, `Z` zoom, `X` swap colours, `D` default colours, `[` `]` brush size, `Shift+[` `]` hardness, `1`–`0` opacity 10–100%.

| Where | Context menu |
|---|---|
| Canvas (no selection) | Paste, Select All, Subject, layer under pointer ▸, View ▸ |
| Canvas (selection) | Deselect, Invert, Feather…, Refine Edge…, Fill…, Content-Aware Fill…, Layer via Copy, Mask from Selection, Save Selection… |
| Layer row | Duplicate, Delete, Rename, Group, Convert to Smart Layer, Rasterise, Add Mask, Clip, Copy Style, Paste Style, Merge Down, Export Layer…, Colour label ▸ |
| Mask thumbnail | Disable, Delete, Apply, Add to Selection, Refine… |
| History node | Jump Here, Name…, Pin, New Document from State, Delete Branch |
| Brush preset | Rename, Duplicate, Export, Delete, Tags… |
| Guide | Position…, Delete, Lock Guides |

Drag and drop: a file onto the canvas adds a smart layer; onto the tab bar opens a document; a layer row onto another document copies it; a swatch onto a layer fills it; a layer out to SY02 Files exports PNG (offer includes PNG and native). Gestures: pinch zoom, two-finger pan and rotate; pen barrel button opens the brush popover; `Alt` held gives the eyedropper while painting.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No document: centre shows "Create or open an image" with New, Open and recent files as a `gridView`. Docks are hidden until a document exists. |
| Loading | Tab shows a spinner and the decode progress; the canvas shows the embedded thumbnail first, then tiles as they arrive. Cancel is always available. |
| Error | Decode errors show inline in the tab with the helper's message and "Open in sandbox report". A helper crash never closes other documents. |
| Offline | No change; the app never uses the network. |
| Locked | Session lock hides the canvas; an encrypted source closed on lock if the Vault policy says so. |
| Read-only | Banner "Read-only: Save As to edit a copy"; tools that change pixels are disabled with a tooltip. |
| Low scratch | Banner with free space, a button to purge history, and a link to settings. |
| Narrow (< 640 px) | Toolbox collapses to one column; docks become a bottom sheet with tabs; options bar scrolls. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Docks | `dockArea`, `dockWidget`, `tabWidget`, `splitter` |
| Toolbox, options | `toolButton`, `comboBox`, `spinBox`, `slider`, `checkBox`, `popupMenu` |
| Layers, history, channels | `treeView`, `listView`, `image` |
| Colour | `colorPicker`, `slider`, `lineEdit`, `gridView` |
| Curves, dynamics | `curveEditor` (single curve today; many curves and bezier tangents need FUSE-WP-50) |
| Histogram | `chart` |
| Properties | `propertyGrid` |
| Tables | `tableView`, `modelListView` |
| Dialogs | `dialog`, `fileDialog` is not used (portal instead), `fontPicker`, `progressBar` |
| Canvas | **Missing**: G-12 (FUSE-WP-43) |

Gaps:

- **G-12, specified for this app.** The canvas must provide: (a) a `CanvasSource` the app implements to supply 256 × 256 tiles at pyramid levels as shared-memory CPU images or GPU external textures; (b) 8-bit, 16-bit and float tiles, colour managed to the display; (c) pan, zoom 1%–6400%, view rotation and mirror, pixel grid; (d) 64-bit document coordinates; (e) pen events with pressure, tilt, rotation and eraser at device rate, delivered before the frame; (f) a wet-ink overlay painted in the same frame as the pen sample; (g) vector overlays for selections (marching ants), guides, handles, cages and text carets; (h) rulers and guides; (i) region export for thumbnails. Layer compositing for this app is done by the raster engine helper, not by the widget, because adjustment layers, smart filters and 32-bit blending need the full stack; the widget shows the engine's composited tiles plus overlays. This matches the design in FUSE-WP-43 and adds requirement (g) for cage and mesh handles.
- **CR01-GAP-1**: histogram and waveform scope widget fast enough for live update (the `chart` widget may do; to be measured).
- **CR01-GAP-2**: gradient editor widget (stops on a bar). Small; can be built in `.fuseui` from `panel` and `slider` if drag on custom-painted panels is available.
- **CR01-GAP-3**: cross-process GPU texture sharing between a helper and the app's UI context (needed for zero-copy tiles). Not in G-01..G-18; see section 13.
- Pen tablet input: FUSE-WP-08. HDR and display colour: FUSE-WP-11, FUSE-WP-26. Text shaping: G-03.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Document model (layer tree, history tree, selections as references), tools as state machines, actions, batch, settings, intents |
| UI | FUSE UI (`.fuseui` + CSS) | app host | Workbench, docks, dialogs, canvas overlays |
| `raster-engine` | C++23 | helper, one per document group | Tile store, compositing, brush rasteriser, selection maths, transforms, filters, adjustments, colour conversion, GPU kernels. Allowed by PLATFORM.md section 2 (b). |
| `image-codec` | C++23 (wraps third-party decoders) | helper, one per file, no capabilities | Decode and encode every file format; ICC and metadata parsing. Section 2 (a). |
| `raw-develop` | C++23 | helper, one per file, no capabilities | RAW container parsing, demosaic, develop pipeline. Section 2 (a). |
| Model client | ForgeScript | app host | Sends tiles to `svc.model`, receives mattes and pixels |
| Script sandbox | ForgeScript | child engine in app host | Runs user actions and signed plugins with a narrowed policy |

```
                 +------------------- app host (org.pbsd.image-editor) -------------------+
 portal handles  |  ForgeScript: document model, tools, history tree, actions            |
 ───────────────▶|  FUSE UI: docks, dialogs, canvas widget (G-12) ◀── tiles (shm / GPU)  |
                 +----------┬───────────────────┬───────────────────┬---------------------+
                            │ commands, tile ids│ file handle       │ tiles
                            ▼                   ▼                   ▼
                    raster-engine         image-codec           svc.model
                    (dev.gpu.compute)     raw-develop           (ai.local)
                    scratch in ~/.cache   (no capabilities)
```

The app host never touches pixel data except thumbnails. Commands to the engine are small records (tool, parameters, stroke samples). Tiles move as shared-memory handles (PLATFORM.md section 7).

### 6.2 Data model

**Decision: the native format is OpenRaster with a documented extension set, file extension `.ora`.** Reasons: OpenRaster is an open, published container (ZIP with `stack.xml`, PNG layers, `mergedimage.png`, `Thumbnails/thumbnail.png`) that GIMP, Krita and MyPaint read; PLATFORM.md section 9 requires an open default format; a baseline reader still gets a correct flattened image and the raster layers. Baseline OpenRaster cannot hold adjustment layers, text, vector shapes, smart layers, float pixels or tiled huge layers, so those go into extension elements and extra archive members under a `pbsd:` namespace. Alternatives rejected: PSD (closed, undocumented in parts), XCF (tied to one app's internals, not a stable interchange format), KRA (another app's native format, changes with that app), a wholly new format (no reader anywhere else).

Archive members:

| Member | Content |
|---|---|
| `mimetype` | `image/openraster` (first, stored) |
| `stack.xml` | Baseline stack plus `pbsd:` attributes and elements |
| `data/<layer_id>.png` | 8 or 16-bit integer layer pixels (baseline) |
| `data/<layer_id>.tiles` | Tiled layer for float data or layers above 16 384 px a side: header, tile index, zstd-compressed 256 × 256 tiles, pyramid levels optional |
| `masks/<mask_id>.png` | Layer and group masks |
| `pbsd/document.json` | Guides, grids, slices, samplers, colour settings, artboards, view state |
| `pbsd/history.json` | Named snapshots only (the undo tree is not saved by default) |
| `pbsd/smart/<id>.ora` or other file | Embedded smart layer sources |
| `pbsd/icc/<name>.icc` | Document and proof profiles |
| `pbsd/actions/*.fs` | Document-attached actions (never run automatically) |
| `mergedimage.png`, `Thumbnails/thumbnail.png` | Flattened image and 256 px thumbnail (baseline) |

Entities (in memory and in `stack.xml` / `document.json`):

| Entity | Fields |
|---|---|
| `Document` | `id: uuid`, `width: i64`, `height: i64`, `ppi: f64`, `model: enum{rgb,grey,indexed,cmyk}`, `depth: enum{u8,u16,f16,f32}`, `profile: ref`, `root: LayerId`, `schema: u32` |
| `Layer` | `id: u32`, `kind: enum{raster,group,adjust,fill,text,shape,smart}`, `name: string`, `visible: bool`, `opacity: f32`, `fill: f32`, `blend: enum`, `lock: flags`, `clip: bool`, `offset: (i64,i64)`, `bounds: rect`, `mask: MaskId?`, `vmask: PathId?`, `styles: [Style]`, `label: u8`, `children: [LayerId]` |
| `RasterData` | `layer: u32`, `tiles: map<(level,x,y) → TileRef>`; `TileRef` is a refcounted block in the engine |
| `Adjust` | `type: enum`, `params: json` (schema per type) |
| `Text` | `runs: [{text, font, size, colour, features}]`, `box: rect?`, `para: {...}`, `path: PathId?` |
| `Smart` | `source: embedded member or {handle_token, blake3}`, `transform: mat3`, `filters: [FilterOp]` |
| `Selection` | `kind: enum{mask,path}`, `data: TileSet or PathId`, `feather: f32` |
| `HistoryNode` | `id: u64`, `parent: u64`, `label: string`, `time: i64`, `delta: [TileSwap or ModelPatch]`, `pinned: bool`, `ai: {model, hash}?` |
| `Slice` | `name`, `rect`, `layer: u32?`, `formats: [{format, scale, quality}]` |

Stores: per-document scratch at `~/.cache/org.pbsd.image-editor/scratch/<uuid>/` (tile pages, journal); presets, brushes, swatches, workspaces, actions at `~/.local/share/org.pbsd.image-editor/`; recents and settings through `svc.settings`. `schema` is written in `document.json`; older files are migrated in memory and saved in the new schema only when the user saves; the old file is kept as a version.

### 6.3 File formats and protocols

| Format | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| OpenRaster + extensions (`.ora`) | yes | yes | Own code; ZIP container own code; zstd (BSD, in FreeBSD base) for `.tiles` | `image-codec` |
| PNG, APNG | yes | yes | Own decoder preferred (small format); else libpng (libpng licence) | `image-codec` |
| JPEG | yes | yes | libjpeg-turbo (BSD-style) | `image-codec` |
| WebP | yes | yes | libwebp (BSD) | `image-codec` |
| AVIF | yes | yes | libavif (BSD) with dav1d (BSD) decode, an AV1 encoder to be chosen | `image-codec` |
| JPEG XL | yes | yes | libjxl (BSD) | `image-codec` |
| TIFF, BigTIFF | yes | yes | Own reader for baseline + LZW/Deflate/PackBits; libtiff (BSD-style) if the option count proves too large | `image-codec` |
| OpenEXR | yes | yes | OpenEXR (BSD) | `image-codec` |
| PSD, PSB | yes | export flat + layers (phase 3) | Own code from the public file-format description | `image-codec` |
| XCF | yes | no | Own code | `image-codec` |
| KRA | yes | no | Own code (ZIP + XML + tiled data) | `image-codec` |
| GIF, BMP, TGA, PNM, QOI | yes | GIF, BMP | Own code | `image-codec` |
| SVG (subset) | rasterise | no | FUSE SVG subset (FUSE-WP-27) | `image-codec` |
| DNG and camera RAW | yes | no | Own DNG reader; LibRaw (LGPL-2.1 or CDDL) considered for other cameras | `raw-develop` |
| ICC v2, v4 | yes | embed | Own matrix/TRC and LUT evaluation, or Little CMS 2 (MIT) | `image-codec` validates; engine evaluates a checked structure |
| EXIF, IPTC, XMP | yes | yes | Own code | `image-codec` |
| `.cube`, `.3dl`, Hald | yes | `.cube` | Own code | app host (text, bounded) and engine |
| `.abr`, `.gbr`, `.kpp`, `.gpl`, `.ase` | yes | no | Own code | `image-codec` |
| Engine IPC | — | — | Session-bus schema `org.pbsd.image-editor.engine.v1` | both ends validate |

All third-party decoders are C or C++ outside the safe subset. They are confined to the no-capability helper, built from vendored sources offline, and fuzzed. If the platform later offers a shared image codec service (as ME01 Photos needs the same), this app uses it and drops its own helper.

### 6.4 Integration

- **Intents handled:** `open`, `edit`, `view` (falls back to ME01 for plain viewing), `new`, `share` target (receive an image). **Emitted:** `share`, `print`, `scan`, `open` (linked files, CR02 hand-off).
- **MIME:** edit `image/openraster`, `image/png`, `image/jpeg`, `image/webp`, `image/avif`, `image/jxl`, `image/tiff`, `image/x-exr`; import `image/vnd.adobe.photoshop`, `image/x-xcf`, `application/x-krita`, `image/x-adobe-dng`, `image/gif`, `image/bmp`, `image/svg+xml`.
- **Services:** `svc.portal.file`, `svc.settings`, `svc.notify`, `svc.clipboard`, `svc.thumbnail` (provides a thumbnailer for `.ora`), `svc.model`, `svc.keyring` (signing key), `svc.portal.print`, `svc.power` (inhibit sleep during batch), `svc.backup`, `svc.search` (optional), `svc.intent`, `svc.a11y`.
- **Other apps:** ME01 "Edit in Image Editor"; ME05 sends captures; CR02 takes frames and image sequences; OF08 exchanges SVG and PNG; SE04 for keys; SE05 for encrypted sources; AI02 installs the models; UT03 for fonts.
- **Backing repos.** Verified: FUSE has single-source compute kernels with CPU backends and CUDA trampolines (`FUSE:docs/modules.md`, row `fuse_compute`), a Vulkan compute UI backend (`FUSE:docs/ui/README.md`), `colorPicker` and `curveEditor` widgets (`FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp`). The curve editor holds one float curve with linear, smooth and step keys (`widgets.hpp:296-305`). ForgeScript signing exists (PLATFORM.md V14). Proposed: everything in this spec, including use of `fuse_compute` for image filters. Unverified: whether `fuse_compute` has a Vulkan path usable on HardenedBSD (CUDA is not expected there).

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Presets, brushes, scratch, autosave journal |
| `fs.portal` | Open, save, export, batch folders, linked smart layers |
| `net.none` | The app has no network use at all |
| `proc.helper:raster-engine` | Pixel engine |
| `proc.helper:image-codec` | Untrusted file parsing |
| `proc.helper:raw-develop` | Untrusted RAW parsing |
| `dev.gpu.compute` | GPU filters and compositing (optional; CPU path always exists) |
| `dev.print` | Print through the portal |
| `ai.local`, `svc.model` | Subject select, inpainting, upscale, denoise, depth |
| `svc.settings`, `svc.notify`, `svc.clipboard`, `svc.thumbnail`, `svc.keyring`, `svc.power`, `svc.backup`, `svc.search`, `svc.intent`, `svc.a11y`, `svc.portal.file`, `svc.portal.print` | As listed in 6.4 |

Not requested: any `net.*` beyond `net.none`, `proc.exec`, `dev.camera`, `dev.mic`, `dev.screen`, `dev.location`, `bg.*`, `fs.path`.

**Assets:** the user's images (may be private or unreleased), embedded metadata (location, device serial), signing keys (in the keyring, never in the app), the rest of the user's files.

**Adversaries:** a crafted image file (the main one); a crafted brush, swatch, LUT or ICC file; a hostile script or plugin; a document that carries an action; a malicious model file; another app reading the clipboard or scratch.

**Attack surface and mitigations:**

| Surface | Mitigation |
|---|---|
| Image and RAW decoders | Run only in helpers with no capabilities, in Capsicum capability mode, with only the input descriptor and an output shared-memory handle. Memory and time limits. Output is validated (dimensions, stride, tile counts) before the engine accepts it. |
| Decompression bombs | Declared size is checked against the scratch cap before allocation; the user is asked above 1 gigapixel. |
| ICC profiles | Parsed to a checked structure in the helper; tag counts and LUT sizes bounded. |
| GPU drivers | Kernels are fixed and shipped in the bundle; no user-supplied shaders. A CPU fallback exists for every kernel. |
| Scripts and plugins | Signed ForgeScript only; run with the app's capabilities narrowed to the document API (no `fs`, no `proc`); bytecode only, no native code (Verified: `FORGE:script/src/runtime.cpp:729-730`, `script/src/vm.cpp:220-229`). |
| Actions inside documents | Never run on open. Shown as text; the user must import them. |
| Linked smart layers | Reached only through remembered portal grants; a changed hash shows a stale badge. |
| Metadata leaks | Export strips location by default; the export dialog lists what will be written. |
| Scratch and autosave | In the app's own cache; deleted on document close; on an encrypted home dataset by OS policy. |
| Models | Loaded by `svc.model`, signed `.fpk`; the app never loads model files itself. |

**Stored:** presets, recents (handle tokens, not paths), autosave journals, settings. **Leaves the machine:** nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `general.workspace` | enum | `photo` | Dock layout at start |
| `general.restore_documents` | bool | `true` | Reopen documents from the last session |
| `canvas.pixel_grid_from` | int | `800` | Zoom percent where the pixel grid shows |
| `canvas.checker` | enum | `mid` | Transparency pattern: light, mid, dark |
| `canvas.zoom_at_pointer` | bool | `true` | Wheel zoom centres on the pointer |
| `units.ruler` | enum | `px` | px, mm, cm, in, pt, % |
| `perf.memory_cap_mib` | int | 50% of RAM | Tile cache size |
| `perf.scratch_cap_gib` | int | `16` | Scratch file limit |
| `perf.gpu` | enum | `auto` | auto, on, off |
| `perf.threads` | int | `0` | 0 = all cores minus one |
| `history.max_nodes` | int | `1000` | Oldest unpinned nodes drop first |
| `history.save_snapshots` | bool | `true` | Save pinned snapshots in the file |
| `autosave.interval_s` | int | `30` | Journal interval |
| `colour.rgb_working` | string | `sRGB IEC61966-2.1` | Default RGB profile |
| `colour.open_policy` | enum | `keep` | keep, convert, ask |
| `colour.intent` | enum | `relative` | Default rendering intent |
| `colour.bpc` | bool | `true` | Black point compensation |
| `export.metadata` | enum | `strip_location` | keep, strip_location, strip_all |
| `export.sign_key` | string | empty | Keyring item id for signing |
| `tablet.pressure_curve` | curve | linear | Per device, keyed by device id |
| `tablet.barrel_button` | enum | `brush_popover` | Action of the pen button |
| `brush.smoothing_default` | int | `10` | 0–100 |
| `ai.enabled` | bool | `false` | Show AI tools (needs `ai.local` grant) |
| `ai.record_provenance` | bool | `true` | Mark AI edits in history and metadata |
| `plugins.developer_mode` | bool | `false` | Allow unsigned scripts with a banner |
| `search.index_documents` | bool | `false` | Contribute to `svc.search` |

## 9. Accessibility and languages

- The canvas exposes a text summary to the a11y tree: document size, active layer, selection bounds, tool. Layers, history and channels are ordinary trees and lists.
- Every tool can be driven by keyboard: arrow keys move by 1 px (10 with Shift), numeric fields for all transforms, `Enter` commits, `Esc` cancels.
- Colour is never the only signal: clipping warnings use a pattern; label colours have names.
- A colour-blind preview (protan, deutan, tritan) is a view filter, not an edit.
- High-contrast theme draws selection outlines and handles at 2 px with a contrasting halo.
- Text layers support all scripts that the FULL text profile supports (G-03), including right-to-left and vertical text in phase 3.
- Units, decimal separators and paper presets follow the locale.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | ≤ 400 ms (large app, PLATFORM.md section 11) |
| Open 24 MP JPEG to visible | ≤ 300 ms (thumbnail first), full tiles ≤ 1 s |
| Pan and zoom, 20 layers, 4K view | ≤ 4 ms GPU frame (FUSE-WP-43 budget) |
| Pen sample to wet ink on screen | ≤ 1 frame |
| Brush stroke, 500 px soft round, 16-bit | ≥ 200 dabs/s without lag |
| Gaussian blur r = 50, 24 MP, GPU | ≤ 150 ms; CPU ≤ 1.5 s |
| Largest document | 2^31 px per side; tested target 100 000 × 100 000 px 8-bit |
| Idle | 0 frames, 0 wake-ups with no pointer over the canvas |
| Memory at idle with no document | ≤ 120 MiB across app host and engine |
| Undo of a stroke | ≤ 16 ms (tile swap) |

## 11. Testing and proof

Nothing below has run. All are plans and targets.

- **Unit (ForgeScript):** layer tree operations, history tree branching and pruning, selection maths at the model level, slice naming, action recording and replay.
- **Unit (C++):** blend mode formulas against reference values at 8, 16 and 32 bits; tile copy-on-write and refcounts; resampling kernels; ICC evaluation against reference profiles.
- **Golden renders:** each `.fuseui` dock and dialog in dark, light, high contrast at 100% and 200%; engine goldens for each filter and adjustment on a fixed test image, CPU and GPU within tolerance.
- **Scripted UI through the a11y tree:** the 14 top tasks of section 3.
- **Fuzz:** every decoder in `image-codec` and `raw-develop` (one harness per format), the `.tiles` reader, ICC, metadata, brush and swatch importers, the engine IPC decoder.
- **Differential:** PNG, JPEG, TIFF and EXR decode against the reference libraries on public conformance sets; PSD, XCF, KRA import compared with flattened renders exported from the origin apps (corpus to be collected under clear licences).
- **Round trip:** save and load of every layer kind; ORA baseline files opened by another ORA reader give the same flattened image.
- **Soak:** 10 000 random edits with undo and redo; scratch never exceeds its cap.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Tile index maths | No overflow or out-of-range key up to 2^31 px | T2 | PRISM absence-of-runtime-error job | — |
| Tile store copy-on-write | Refcount never underflows; no tile freed while referenced | T2 | PRISM on the store module | Concurrency model must be fixed first |
| Blend formulas | Output within [0,1] for inputs in [0,1] (integer depths) | T1 | Proof per formula | Float modes stay T3 |
| History tree | Undo then redo returns the identical document hash | T3 | Property tests | A formal model of deltas |
| `.tiles` and ORA reader | No out-of-bounds read on any input | T2 | PRISM plus fuzzing | zstd is third-party (T4 inside the helper) |
| Third-party decoders | Contained | T4 | Sandbox tests only | Not in the safe subset |
| Engine IPC decoder | Rejects malformed records | T2 | PRISM plus fuzzing | — |
| ForgeScript app logic | Memory safe by language | T3 | Language guarantees, unit tests | No ForgeScript prover yet |
| GPU kernels | Match CPU reference within tolerance | T3 | Golden comparison | Driver is outside the proof boundary |

## 12. Build plan

### Phase 1 — usable core

| ID | Work item | Depends on |
|---|---|---|
| CR01-W01 | Engine IPC schema and stub engine | G-08 |
| CR01-W02 | Tile store, copy-on-write, scratch paging | — |
| CR01-W03 | Compositor: raster layers, groups, opacity, all blend modes, masks, clipping (CPU) | W02 |
| CR01-W04 | `image-codec` helper: PNG, JPEG, WebP, TIFF, native ORA read and write | FORGE-WP-27, FORGE-WP-28, G-06 |
| CR01-W05 | App shell: workbench, docks, tabs, menus, action table | G-01, G-02, G-07, FUSE-WP-31 |
| CR01-W06 | Canvas integration: tiles, overlays, rulers, guides | G-12 (FUSE-WP-43), W01 |
| CR01-W07 | Tools: move, marquee, lasso, wand, crop, eyedropper, fill, gradient | W03, W06 |
| CR01-W08 | Brush engine v1: round tips, pressure, eraser, clone stamp, wet ink | W06, FUSE-WP-08 |
| CR01-W09 | Adjustments: levels, curves, HSL, colour balance, brightness; adjustment layers | W03 |
| CR01-W10 | Filters v1: blur, sharpen, with preview | W03 |
| CR01-W11 | Colour management v1: profiles on open, working spaces, display transform | FUSE-WP-26 |
| CR01-W12 | Linear undo, autosave journal, crash restore | W02 |
| CR01-W13 | Export dialog, clipboard, drag and drop | W04, G-09 |
| CR01-W14 | Free transform and image / canvas size | W03 |

### Phase 2 — full feature

| ID | Work item | Depends on |
|---|---|---|
| CR01-W15 | History tree and snapshots | W12 |
| CR01-W16 | Brush dynamics, custom tips, smoothing, symmetry, preset manager | W08 |
| CR01-W17 | Healing, spot heal, patch, patch-match fill | W03 |
| CR01-W18 | Quick select, colour range, refine edge, paths | W07 |
| CR01-W19 | Text layers | G-03, FUSE-WP-18 |
| CR01-W20 | Shape layers, fill layers, layer styles, embedded smart layers | W03, FUSE-WP-23 |
| CR01-W21 | Perspective, warp, lens correction | W14 |
| CR01-W22 | GPU kernels for compositing and filters | W10, FUSE-WP-24, CR01-GAP-3 |
| CR01-W23 | PSD, XCF, KRA import; AVIF, JPEG XL, EXR; metadata editor | W04 |
| CR01-W24 | `raw-develop` helper and workspace | W04 |
| CR01-W25 | Soft proofing, convert profile, CMYK export, LUTs | W11 |
| CR01-W26 | Actions, batch, signed script plugins | FORGE-WP-14, FORGE-WP-33 |
| CR01-W27 | AI subject select and model disclosure | G-15, FORGE-WP-29, FORGE-WP-31, AI02 |
| CR01-W28 | Slices, export presets, print | W13, G-16 |
| CR01-W29 | Huge-image path: pyramids, tiled `.tiles` member, 100k × 100k test | W02 |
| CR01-W30 | Disposable-sandbox open, trust badge | W04, SH04 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CR01-W31 | Liquify, mixer brush, history brush, content-aware scale | W16, W21 |
| CR01-W32 | Linked smart layers and smart filters | W20 |
| CR01-W33 | Model inpainting, upscale, denoise, depth | W27 |
| CR01-W34 | HDR display and export; tone mapping | FUSE-WP-11, W25 |
| CR01-W35 | Animation frames and export | W03, CR02 hand-off |
| CR01-W36 | CMYK working model, spot channels, blend-if, layer comps, artboards | W25 |
| CR01-W37 | Script console, script filters, command-line batch | W26 |
| CR01-W38 | Signed export, versions through snapshots, search contribution | SE04, AD03 |

## 13. Open questions and risks

1. **GPU sharing across processes (CR01-GAP-3).** The engine helper and the UI context are in different processes. Zero-copy tiles need a shared GPU image handle between them. If the platform cannot give that, tiles go through shared CPU memory and the 4 ms pan budget may not hold at 4K with float tiles.
2. **`fuse_compute` on HardenedBSD.** Verified only that it has CPU backends and CUDA trampolines. A Vulkan compute path for image kernels is Proposed. Without it, GPU filters are a separate piece of work.
3. **Third-party decoders.** Seven or more C libraries. Each must be vendored, built offline and kept patched. Own decoders for PNG and TIFF reduce this; JPEG XL and AVIF are too large to rewrite soon.
4. **RAW coverage.** DNG is documented. Other camera formats are not; LibRaw's licence (LGPL-2.1 or CDDL) is compatible only as a separate helper binary. Owner decision needed.
5. **OpenRaster extensions.** Other apps will ignore the `pbsd:` parts. The extension set must be published so others can read it. Risk of drift from the baseline specification.
6. **Patent or licence status of HEIF decoding** is not settled; it is read-only and optional here.
7. **Local models.** Segmentation and inpainting models must exist as signed `.fpk` files with licences that allow shipping. None is named here; AI02 owns that list.
8. **Colour engine choice.** Own ICC evaluation is a large job to get exactly right; Little CMS 2 is proven but outside the safe subset and would run in the engine process. Decide before W11.
9. **Tablet support on HardenedBSD** (driver and Wayland tablet protocol) is Unverified.
10. **Tool shortcuts as single keys** conflict with type-ahead in docks; they apply only while the canvas has focus. To be tested with screen-reader users.
