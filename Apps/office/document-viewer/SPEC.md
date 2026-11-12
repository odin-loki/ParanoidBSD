<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Document Viewer

By Odin Loch

| | |
|---|---|
| ID | `OF03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.document-viewer` |
| Category | office |
| Priority | P1 |
| Backing repos | FUSE (page view FUSE-WP-45, canvas FUSE-WP-43: Proposed), FORGE (helper framework FORGE-WP-27/28, OCR model class FORGE-WP-31: Proposed) |
| Status | Planned. Nothing built. |

## 1. Purpose

Reads paged and reflowable documents: PDF, EPUB, DjVu, comic archives, XPS and PostScript. It annotates, fills forms, verifies and makes digital signatures, redacts so that content is truly gone, reorganises pages, and keeps a bookshelf with reading progress. Every document is treated as hostile: parsing happens in a sandbox with no capabilities and embedded scripts never run. It replaces Okular from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope: everything in section 4.

Out of scope:

| Item | Owner |
|---|---|
| Authoring documents, reflowing PDF text into an editable document | OF04 Writer (it can import a PDF as text; this app does not edit body text) |
| Vector editing of page content | OF08 Draw |
| Scanning hardware, print queues | UT06 Printers and Scanners |
| Key and certificate management, trust store editing | SE04 Passwords and Keys |
| File encryption outside the PDF standard, secure erase | SE05 Vault |
| Speech output | AX01 Screen Reader (this app supplies text and receives highlight positions) |
| Image-only viewing | ME01 Photos |
| Office formats (ODT, DOCX, ODS, …) | OF04, OF05, OF06 |
| Archive browsing | UT01 Archives |
| Ebook DRM | Not supported. DRM-locked files show a clear message |
| Web pages | IN01 Web Browser |

## 3. Users and top tasks

Personas:

- **Reader**: papers, manuals, books and comics; wants fast paging, search, bookmarks, progress.
- **Office user**: fills forms, signs, merges and splits, prints.
- **Reviewer**: highlights, comments, exports a summary of annotations.
- **Discloser** (lawyer, journalist, records officer): redacts and strips metadata and must be sure it worked.
- **Cautious user**: opens attachments from unknown senders and needs them to be harmless.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a PDF from Files or Mail and read | 1 |
| T2 | Find a phrase in a 1 000-page document | 2 (`Ctrl+F`, type) |
| T3 | Highlight a passage | 2 (select, `H`) |
| T4 | Fill a form and save | type, `Ctrl+S` |
| T5 | Check who signed a document and whether it changed | 1 (signature banner) |
| T6 | Sign a document | 4 (Sign, place, pick key, confirm in shell prompt) |
| T7 | Redact a name everywhere and verify | 5 (search, mark all, apply, review report, save) |
| T8 | Merge two PDFs and reorder pages | 3 (drop, drag, save) |
| T9 | Extract pages 5–9 to a new file | 3 |
| T10 | Make a scanned PDF searchable | 2 (OCR banner, Run) |
| T11 | Continue a book where I stopped | 1 (click cover) |
| T12 | Have a chapter read aloud | 2 |
| T13 | Strip metadata before sending | 3 |
| T14 | Present a PDF on a second screen | 1 (`F5`) |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Opening and formats

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F001 | PDF | PDF 1.0–2.0: all standard filters (Flate, LZW, DCT, JPX, JBIG2, CCITT, RunLength), fonts (Type 1, TrueType, CFF, Type 3, CID), transparency, blend modes, shadings, patterns, optional content (layers), annotations, AcroForms, tagged structure, embedded files, portfolios (listed, not rendered as a navigator). | M | 1 |
| OF03-F002 | Encrypted PDF | RC4-40/128 and AES-128/256 (revisions 2–6), user and owner passwords. Password asked in a secret field; optional save to `svc.keyring` per file hash. Certificate-encrypted PDFs decrypt with a key from SE04. | M | 1 |
| OF03-F003 | Permission flags | Owner "restrictions" (no print, no copy) are shown as information. They are honoured by default with a one-click override that is recorded in the file's audit note; they are not security. | S | 2 |
| OF03-F004 | EPUB | EPUB 2 and 3 reflowable and fixed layout: OPF, NCX and nav, XHTML with a CSS subset, embedded fonts (deobfuscated per the IDPF algorithm), SVG subset, MathML shown as fallback image or text, footnote popovers. Scripts and remote resources are ignored. | M | 2 |
| OF03-F005 | DjVu | Single and multi-page (bundled and indirect), IW44 and JB2 layers, hidden text layer, outline, annotations (read). | S | 2 |
| OF03-F006 | Comic archives | CBZ (zip), CBR (RAR 4 and 5 read), CB7 (7z), CBT (tar): images in natural name order, `ComicInfo.xml` metadata, double-page spreads detected by aspect ratio. | S | 2 |
| OF03-F007 | XPS and OpenXPS | Fixed pages, fonts (obfuscated ODTTF), images, outline, links. | S | 3 |
| OF03-F008 | PostScript and EPS | Converted to PDF in the `convert-helper` (interpreter in safer mode, no file or device operators), then viewed as PDF. The original is untouched. | S | 3 |
| OF03-F009 | Other paged formats | TIFF multi-page and fax, plain-text and Markdown as reading view (hand-off offered to OF01), FictionBook 2 (`.fb2`, `.fb2.zip`), Mobipocket and KF8 without DRM. | C | 3 |
| OF03-F010 | Open by intent | Handles `open` and `view` with a file handle; arguments `page`, `dest` (named destination), `search`, `zoom`, `annot` so other apps can deep-link (OF02 links to a page). | M | 1 |
| OF03-F011 | Tabs and windows | Each document in a tab; drag out to a window; the same document in two panes with independent positions. | M | 1 |
| OF03-F012 | Recent documents | Last 50 with page, zoom and thumbnail of the last page; remembered portal grants reopen them. Clearable; off in private mode. | M | 1 |
| OF03-F013 | Reload on change | The file is watched; a changed file reloads keeping position (useful while a document is being generated). Unsaved annotations are kept and re-anchored. | S | 1 |
| OF03-F014 | Damaged files | Cross-reference repair, tolerant stream lengths, per-page errors: a page that fails shows an error card and the rest of the document works. A repair report lists what was rebuilt. | M | 1 |
| OF03-F015 | Large documents | 100 000 pages, 4 GiB files: lazy object loading, linearised first-page display, memory capped per helper (default 1 GiB). | M | 2 |
| OF03-F016 | Embedded files | Attachments panel: name, size, type, description, hash. Save As or Open go through the portal and an intent, with the document's provenance tag copied on. Nothing opens automatically. | M | 2 |
| OF03-F017 | Format detection | By magic bytes, not by extension; a mismatch shows a notice. Polyglot files (valid as two types) are flagged. | M | 1 |
| OF03-F018 | Standards information | Shows claimed and checked conformance: PDF/A-1, 2, 3 (basic checks: fonts embedded, no encryption, no JavaScript, XMP present), PDF/X, PDF/UA tags present. Not a full validator. | C | 3 |

### 4.2 Viewing and navigation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F020 | Layout modes | Single page, continuous, two-up, two-up continuous, book (cover alone), horizontal strip. Right-to-left page order option. | M | 1 |
| OF03-F021 | Zoom | 10%–6400%; fit page, fit width, fit visible content (trim margins), actual size (uses display DPI); `Ctrl+wheel`, pinch, `Ctrl++/-/0`; marquee zoom with `Ctrl+drag`. | M | 1 |
| OF03-F022 | Progressive rendering | Old tiles are scaled while sharp ones render; no blank frames. Tiles 512 px, rendered in the helper, cached by (page, scale, tile, appearance state). | M | 1 |
| OF03-F023 | Rotation | View rotation per document or per page in 90° steps (does not change the file; "Rotate pages" in 4.8 does). | M | 1 |
| OF03-F024 | Page navigation | Page field accepts numbers and page labels ("iv", "A-3"); first, last, previous, next; `Alt+Left/Right` history of jumps; `Ctrl+L` go to page. | M | 1 |
| OF03-F025 | Outline | Sidebar tree of bookmarks from the document; current section highlighted; expand and collapse all; filter field. | M | 1 |
| OF03-F026 | Thumbnails | Sidebar grid, lazily rendered; shows page labels, annotation and bookmark marks; size slider; multi-select for page operations. | M | 1 |
| OF03-F027 | Links | Internal links jump with history; external URLs show the full target in the status bar and ask before emitting an `open` intent; remote "GoToR", "Launch", "SubmitForm" and "ImportData" actions are never performed, only shown. | M | 1 |
| OF03-F028 | Link previews | Hovering an internal link to a figure, citation or footnote shows a popover rendering of the target region. | S | 2 |
| OF03-F029 | User bookmarks | `Ctrl+D` bookmarks the current view; named, reorderable; stored in the sidecar or, on request, written into the PDF outline. | M | 1 |
| OF03-F030 | Layers | Optional content groups listed with visibility toggles; default state from the file; print and export respect the user's choice. | S | 2 |
| OF03-F031 | Presentation mode | `F5`: full screen, one page per screen, transitions limited to cut and fade; presenter view on a second output with next page, notes annotations, clock and timer; laser pointer and temporary ink; blank screen with `B`. | S | 2 |
| OF03-F032 | Reading themes | Page colours remapped: sepia, dark (lightness inverted, hue kept, images left alone), custom paper and ink; follows the system theme (option). Never changes the file. | S | 1 |
| OF03-F033 | Trim margins | Detect content bounds per page or use a uniform crop; for small screens. | S | 2 |
| OF03-F034 | Auto-scroll | Adjustable continuous scroll for reading; stops on any input. | C | 3 |
| OF03-F035 | Split view | Two panes on one document (text and its references) or two documents side by side with optional synchronised scrolling. | S | 2 |
| OF03-F036 | Compare documents | Text-level comparison of two PDFs: changed words marked on both pages, list of differences; pixel overlay mode for drawings. | C | 3 |
| OF03-F037 | Magnifier loupe | Hold `Z`: a lens at 2–8× under the pointer rendered at full resolution. | C | 3 |
| OF03-F038 | Measure | Distance, perimeter and area tools with a scale ratio (from the viewport dictionary or typed, "1:100"); snap to path points. | C | 3 |
| OF03-F039 | Reflow reading (EPUB) | Font family, size 8–40, line height, margins, justification, hyphenation (G-17), one or two columns, paged or scrolled; publisher styles on or off. | M | 2 |
| OF03-F040 | Reflow for PDF | Optional text reflow of tagged or well-structured PDFs into a single column for narrow windows; falls back with a notice. | C | 3 |
| OF03-F041 | Comics view | Fit height, fit width, original; spread handling; manga right-to-left; guided panel view is not planned. | S | 2 |
| OF03-F042 | Full screen and distraction-free | `F11`; chrome auto-hides; page turn by click zones or keys. | M | 1 |
| OF03-F043 | Remember view | Page, zoom, layout, rotation, sidebar and theme are remembered per document (by content hash, so a moved file keeps its state). | M | 1 |
| OF03-F044 | Colour management | Output intents and ICC-based colour spaces converted to the display profile (needs FUSE-WP-26); overprint preview toggle; without the gap closed, sRGB is assumed and a chip says so. | S | 3 |
| OF03-F045 | Multimedia and 3D content | Rich media, sound, video and 3D annotations are shown as inert placeholders with "Save embedded media…". Nothing plays inside the viewer. | M | 1 |

### 4.3 Text, search and copy

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F046 | Text selection | Across lines, columns and pages using the helper's text layer (glyph boxes in reading order); word and line snapping; rectangular selection with `Alt+drag`. | M | 1 |
| OF03-F047 | Copy | Plain text (with hyphenation joined and ligatures expanded, option), rich text, or as an image of the region (`Ctrl+Shift+C`). Copy honours F003. | M | 1 |
| OF03-F048 | Find | `Ctrl+F`: incremental, case, whole word, regex (FORGE regex, step-limited), diacritic folding, across line breaks and hyphenation. Hits highlighted on pages, in the scroll bar and thumbnails. | M | 1 |
| OF03-F049 | Results list | Sidebar panel with every hit, its page and a context line; grouped by outline section; streaming while the scan runs. | M | 1 |
| OF03-F050 | Search many documents | Search across all open tabs or the whole library (4.11) using the library text index. | S | 2 |
| OF03-F051 | Look up | Context menu: Search library, Copy, Define (local dictionary service if installed), Translate and Ask (local model, 4.9), Search the web (emits an intent to IN01; never automatic). | S | 2 |
| OF03-F052 | Text layer inspector | Toggle to show the extracted text boxes over the page (helps judge OCR and redaction). | S | 2 |
| OF03-F053 | Table copy | Select a region and copy as tab-separated cells using ruling lines and alignment heuristics. | C | 3 |
| OF03-F054 | Citation copy | Copies selection with document title, author and page label appended in a chosen style (plain, BibTeX key from metadata). | C | 3 |
| OF03-F055 | Missing text warning | Pages with no text layer, or with fonts lacking a Unicode mapping, are flagged; the OCR banner (F137) is offered. | M | 1 |
| OF03-F056 | Word and page counts | Per document and per selection. | C | 2 |
| OF03-F057 | Snapshot tool | Drag a rectangle; copy or save it as PNG at a chosen DPI or as a one-page PDF. | S | 2 |
| OF03-F058 | Export text | Whole document or page range to plain text or Markdown (headings from tags or the outline). | S | 2 |

### 4.4 Annotations

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F060 | Text markup | Highlight (`H`), underline (`U`), strike-out (`K`), squiggly; on the current selection or as a sticky tool. Stored as standard PDF markup annotations with quad points. | M | 1 |
| OF03-F061 | Sticky notes | Note icon with a pop-up; text is plain with basic rich text (bold, italic) written to both `Contents` and `RC`. | M | 1 |
| OF03-F062 | Comments on markup | Any annotation carries a comment, author, created and modified times, subject. | M | 1 |
| OF03-F063 | Free text | Typewriter text on the page and call-outs with a leader line; font from the standard set, size, colour, alignment, border, fill. | M | 2 |
| OF03-F064 | Ink | Freehand pen with pressure (width varies), highlighter pen, eraser; smoothing; strokes stored as PDF ink annotations with a rendered appearance stream. | M | 2 |
| OF03-F065 | Shapes | Line, arrow, rectangle, ellipse, polygon, polyline, cloud; stroke width, dash, colour, fill, opacity, line endings. | S | 2 |
| OF03-F066 | Stamps | Built-in (Approved, Draft, Confidential, Received with date), custom from an image or a PDF page; dynamic text fields (date, name). | S | 2 |
| OF03-F067 | Image and signature image | Place a PNG, JPEG or SVG-subset image as a stamp annotation; a drawn or scanned "wet" signature image is kept in the app store. Clearly separate from digital signatures (4.6). | S | 2 |
| OF03-F068 | File attachment annotation | Attach a file at a point on the page. | C | 3 |
| OF03-F069 | Link creation | Draw a link rectangle to a page, a named destination or a URL. | C | 3 |
| OF03-F070 | Replies and review state | Threaded replies (`IRT`), state (accepted, rejected, completed, none), per author. | S | 2 |
| OF03-F071 | Annotations panel | List grouped by page, type, author or colour; filter and search; click jumps; multi-select to delete, recolour, change author, export. | M | 2 |
| OF03-F072 | Properties and defaults | Inspector for the selected annotation; "Make default for this tool"; ten colour presets from tokens plus custom. | M | 2 |
| OF03-F073 | Edit, move, resize | Handles on every annotation; arrow keys nudge by 1 pt, with Shift by 10 pt; copy and paste between pages and documents; z-order. | M | 2 |
| OF03-F074 | Undo and redo | Unlimited per document for annotation, form and page operations. | M | 1 |
| OF03-F075 | Where annotations live | Two modes per document: **in the file** (standard PDF annotations, saved by incremental update) or **sidecar** (kept in the app store keyed by content hash; the file is never written). Default: sidecar until the user saves. EPUB, DjVu and comics always use the sidecar. | M | 1 |
| OF03-F076 | Autosave of annotations | Sidecar changes are saved at once. In-file changes are journalled and written on `Ctrl+S`; a crash loses nothing. | M | 1 |
| OF03-F077 | Flatten | Burn chosen annotations into page content so they cannot be moved or removed as annotations. | S | 2 |
| OF03-F078 | Import and export annotations | XFDF and FDF in and out; export a summary as Markdown, HTML or a PDF appendix (text, page, colour, comment); send highlights to a note in OF02. | S | 2 |
| OF03-F079 | EPUB highlights | Anchored by EPUB CFI ranges so they survive font and layout changes; exported in the W3C Web Annotation JSON form. | S | 2 |
| OF03-F080 | Author identity | Author name comes from a setting, not from the system account, and can be blank. Times can be written in UTC without zone (option) to avoid leaking the time zone. | M | 2 |
| OF03-F081 | Hidden annotations shown | Annotations flagged hidden, no-view or outside the page box are listed in the panel with a warning mark; nothing in the file can hide from the user. | M | 2 |
| OF03-F082 | Pen-first mode | With a pen, the active ink tool draws without selecting a tool; finger scrolls. | S | 2 |

### 4.5 Forms

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F083 | AcroForm fields | Text (single, multi-line, comb, password), check box, radio group, combo, list (multi-select), push button (inert unless it resets the form), signature field, barcode field (display only). Drawn as real FUSE widgets over the page. | M | 2 |
| OF03-F084 | Field navigation | `Tab` and `Shift+Tab` follow the document's tab order, else row order; a form bar shows "Field 3 of 41", required fields left, [Highlight Fields]. | M | 2 |
| OF03-F085 | Formatting and validation without scripts | The common built-in format, keystroke, validate and calculate actions (number, percent, date, time, special, range, simple sum/product/average/min/max) are recognised by pattern and re-implemented natively. Any other script is not run; the field is marked "has a script that was not run". | M | 2 |
| OF03-F086 | Appearance generation | Field appearance streams are regenerated with the right font and auto-size so other viewers show the same values; `NeedAppearances` is cleared. | M | 2 |
| OF03-F087 | Save filled forms | Incremental save keeps existing signatures valid where the certification level allows form filling; otherwise the user is told the signature will show as modified. | M | 2 |
| OF03-F088 | Import and export form data | FDF, XFDF, JSON and CSV (one row per document). | S | 3 |
| OF03-F089 | Reset and clear | Reset to defaults; clear all; undoable. | M | 2 |
| OF03-F090 | Fill from saved identity | Optional: offer values (name, address, e-mail) from a profile kept by CO03's "me" card; never automatic; each use is one click per field or "fill all matched" with a preview. | C | 3 |
| OF03-F091 | Flat forms | For forms with no fields, a "Fill and Sign" tool places typewriter text, ticks, crosses and dots, with alignment guides snapping to detected lines and boxes. | S | 2 |
| OF03-F092 | XFA forms | Static XFA: the AcroForm fallback is used. Dynamic XFA is not supported; a clear message is shown and the XML data packet can be exported. | S | 3 |
| OF03-F093 | Submit actions | Submit-form actions are never performed. "Export data…" is offered instead and the target URL is shown. | M | 2 |
| OF03-F094 | Form field list | Inspector list of all fields with name, type, value, flags (required, read-only, hidden); hidden fields are shown and marked. | S | 3 |

### 4.6 Digital signatures

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F095 | Verify on open | Every signature is checked when the document opens: byte-range digest, CMS structure, signer certificate chain, document changes after signing. A banner summarises: valid, valid but changed after signing (what changed), unknown signer, invalid. | M | 2 |
| OF03-F096 | Formats | `adbe.pkcs7.detached`, `adbe.pkcs7.sha1` (verify only, flagged weak), `ETSI.CAdES.detached` (PAdES B-B, B-T, B-LT, B-LTA), document time-stamps (`ETSI.RFC3161`). Algorithms: RSA PKCS#1 v1.5 and PSS, ECDSA P-256/384/521, Ed25519; SHA-256/384/512. SHA-1 and MD5 digests verify with a "weak" verdict. | M | 2 |
| OF03-F097 | Trust | Chains are built to anchors in the trust store managed by SE04 (system and user anchors, per-purpose). The viewer has no trust store of its own. | M | 2 |
| OF03-F098 | Revocation, offline first | Embedded revocation data (DSS, VRI, CMS attributes) is used. With no embedded data the verdict says "revocation not checked". Online OCSP or CRL fetch is a per-use button, needs `net.user`, shows the endpoint, and is off by default. | M | 2 |
| OF03-F099 | Byte-range coverage | A signature that does not cover the whole file as it was at signing, or whose byte range has gaps beyond the signature value, is reported as "covers only part of the document" and never shown as plainly valid. | M | 2 |
| OF03-F100 | Changes after signing | Later incremental updates are classified: allowed (form fill, annotations, further signatures, per the DocMDP and FieldMDP levels) or not allowed (page content, new pages). "View signed version" opens the exact bytes that were signed. | M | 2 |
| OF03-F101 | Signature panel | Sidebar list of signatures and time-stamps: signer, time and its source (signer's clock or time-stamp authority), reason, location, certificate details, chain, algorithm, coverage, verdict with reasons in plain words. | M | 2 |
| OF03-F102 | Shadow-attack checks | Detect content hidden or replaced through incremental updates, overlapping objects, or form fields painted over text; such documents get a warning even when the signature verifies. | S | 3 |
| OF03-F103 | Sign | Sign into an existing field or draw a new one. The key is chosen from SE04 (software keys, hardware tokens). The helper computes the byte-range digest; the private-key operation happens in SE04 after a shell prompt naming the document, its hash and the key. The key never enters this app. | M | 3 |
| OF03-F104 | Signature appearance | Visible (name, date, reason, location, optional image, optional logo) or invisible; templates saved in the app. The appearance says nothing the signature does not bind. | S | 3 |
| OF03-F105 | Certify | First signature may certify with a permission level: no changes, form filling, form filling and annotations. | S | 3 |
| OF03-F106 | Time-stamp | Optional RFC 3161 time-stamp from an authority the user configured; needs `net.user`; the request holds only a hash. Without it the signing time is marked "signer's clock". | S | 3 |
| OF03-F107 | Long-term validation data | Add DSS with certificates and revocation data, and a document time-stamp (PAdES B-LT, B-LTA), when the data is available. | C | 3 |
| OF03-F108 | Sign safely | Before signing: the document is checked for content that could change its look later (scripts, external references, optional content tied to events, dynamic forms); findings are listed and "Sign a sanitised copy" is offered. | M | 3 |
| OF03-F109 | Detached signatures on any file | For non-PDF files, hand off to SE05 (detached OpenPGP or minisign-style signatures). | C | 3 |
| OF03-F110 | Signature audit | Each signing act writes a record to `svc.audit`: time, document hash, key id. The document's name is not recorded. | M | 3 |

### 4.7 Redaction

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F111 | Mark for redaction | Mark text selections, rectangles, whole pages or search hits. Marks are outlined, reviewable and removable until applied. | M | 2 |
| OF03-F112 | Search and mark | Mark every hit of a phrase or regex; built-in patterns for e-mail addresses, phone numbers, dates, payment-card numbers (with check digit), IBANs, and number formats the user defines. Each hit can be unticked. | M | 2 |
| OF03-F113 | Apply: text | Glyphs inside the region are deleted from the content stream, not covered: the show-text operators are rewritten, remaining glyphs repositioned so layout does not move, and text in the same string outside the region is kept. | M | 2 |
| OF03-F114 | Apply: images | Pixels under the region are overwritten in the decoded image and the image is re-encoded; soft masks and alternates are treated the same; the original stream is dropped. | M | 2 |
| OF03-F115 | Apply: vectors and others | Paths wholly inside are deleted; paths crossing the edge are clipped; shadings and form XObjects are split or rasterised for the affected area; annotations, form field values, links, bookmarks and structure-tree text that intersect or quote the redacted text are removed. | M | 2 |
| OF03-F116 | Fill and label | Redacted areas are filled (black, white or a chosen colour) with an optional overlay code ("s 47F", "Privacy") and optional repeat. | S | 2 |
| OF03-F117 | Full rewrite | Applying redaction always writes a new file from scratch: no incremental history, no unused objects, no previous versions, object streams rebuilt, the document ID renewed. Existing signatures are removed and the user is told. | M | 2 |
| OF03-F118 | Hidden data removal | With redaction, the sanitise set of 4.10 runs by default: metadata, XMP, embedded files, scripts, hidden layers, thumbnails, private application data, comments (option), off-page content, text hidden under images or drawn invisible. | M | 2 |
| OF03-F119 | Verification pass | After writing, the helper reopens the output in a fresh sandbox and checks: (a) no redacted string is found in extracted text, raw decompressed streams, metadata or names; (b) every redacted region renders as the fill only; (c) no object from the source that was meant to go is byte-identical in the output. A report is shown and can be saved. | M | 2 |
| OF03-F120 | Width leak control | Option to replace redacted text with fixed-width fill so the length of names cannot be read from the gap; warns that surrounding layout will shift. | S | 3 |
| OF03-F121 | Rasterise mode | "Maximum safety": affected pages (or all) are replaced by images at a chosen DPI with no text layer; optional OCR afterwards on the redacted image. | S | 2 |
| OF03-F122 | Never destructive in place | Redaction cannot overwrite the source: the result is a new file with a suggested "‑redacted" name. The app says the original still exists and offers a hand-off to SE05 for secure erase. | M | 2 |

### 4.8 Organising pages

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F123 | Organise view | Thumbnail grid of all pages, large; multi-select; drag to reorder; works across two documents side by side. | M | 2 |
| OF03-F124 | Rotate | 90° left or right, 180°, for selected pages; written to the page dictionary. | M | 2 |
| OF03-F125 | Delete and extract | Delete pages (undoable until save); extract to a new document, one file per page, or by ranges. | M | 2 |
| OF03-F126 | Insert | Pages from another PDF, blank pages, images (each becomes a page), scans (`scan` intent to UT06), any supported format converted to PDF first. | M | 2 |
| OF03-F127 | Merge | Drop files onto the grid or use Merge…: order list, per-file page ranges, bookmarks merged under file names, name clashes in forms and destinations resolved by prefix. | M | 2 |
| OF03-F128 | Split | By page count, by top-level bookmarks, by size limit, by blank separator pages, by page ranges typed as `1-3,7,10-`. | S | 2 |
| OF03-F129 | Crop and page boxes | Set crop box by drag or numbers; apply to ranges; "remove hidden cropped content" really deletes outside content. Edit media, bleed, trim boxes (advanced). | S | 3 |
| OF03-F130 | Page labels | Edit numbering ranges (roman, arabic, letters, prefix, start). | C | 3 |
| OF03-F131 | Outline editing | Add, rename, move, nest and delete bookmarks; set destination from the current view; generate from tagged headings. | S | 3 |
| OF03-F132 | N-up and booklet | Impose 2, 4, 6, 8, 9, 16 pages per sheet; booklet order with creep; as print option or as a new PDF. | S | 3 |
| OF03-F133 | Headers, footers, numbering, watermark | Add text or image across ranges with position, font, opacity; Bates numbering with prefix and digits. | C | 3 |
| OF03-F134 | Resize and scale pages | Scale content to a paper size; add margins. | C | 3 |
| OF03-F135 | Optimise | Downsample and recompress images, subset fonts, remove unused objects, object streams, linearise; shows size before and after; never applied silently. | S | 3 |
| OF03-F136 | Lineage note | Page operations on signed documents warn that signatures will not survive; the output records, in the sidecar only, which source hashes each page came from. | S | 3 |

### 4.9 OCR, speech and local AI

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F137 | OCR | Recognise scanned pages through the OCR model class of `svc.model` (FORGE-WP-31, Proposed). Needs `ai.local`. Page images go to the model service over shared memory; text and boxes come back. | S | 3 |
| OF03-F138 | Searchable output | Results are kept in the sidecar (file untouched) or written as an invisible text layer into a copy; language selection; per-page confidence; low-confidence words listed for review. | S | 3 |
| OF03-F139 | Pre-processing | Deskew, despeckle, auto-rotate, contrast; preview before applying; optional replacement of page images with the cleaned ones. | C | 3 |
| OF03-F140 | Read aloud hand-off | "Read aloud" sends text in reading order to AX01's speech interface in sentence units; AX01 reports the spoken range and the viewer highlights and scrolls to it. Controls: play, pause, speed, voice (AX01's), from here, selection only. | S | 2 |
| OF03-F141 | Reading order | Order comes from tags when present, else from layout analysis (columns, headers and footers skipped by repetition detection); the user can exclude regions per document. | S | 2 |
| OF03-F142 | Summarise and ask | Optional: summary of a selection, chapter or document, and questions answered with page citations, through `svc.model`. Off by default. | C | 3 |
| OF03-F143 | Translate selection | Optional local translation in a popover. | C | 3 |
| OF03-F144 | Auto-outline | Optional: propose bookmarks from detected headings (font size and numbering heuristics first; model only if enabled). | C | 3 |
| OF03-F145 | Model containment | Document text given to a model is data: model output never triggers actions, opens links or fills forms. | M | 3 |
| OF03-F146 | Table and figure extraction | Export a detected table as CSV to OF05 or a figure as an image. | C | 3 |

### 4.10 Metadata and sanitising

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F147 | Properties | Title, author, subject, keywords, creator, producer, dates, PDF version, page sizes, fonts (embedded or not, subset), security, tagged, linearised, file size, hashes (SHA-256, BLAKE3), provenance tag. | M | 1 |
| OF03-F148 | XMP view | Raw XMP packet in a read-only tree; mismatch between the Info dictionary and XMP is flagged. | S | 2 |
| OF03-F149 | Edit metadata | Edit the common fields; both Info and XMP are updated together. | S | 2 |
| OF03-F150 | Hidden-data report | "Inspect document" lists: metadata, XMP history, embedded files, scripts, actions, hidden layers, hidden annotations and fields, text outside the page or under images, cropped image data, previous versions in incremental saves, thumbnails, private application data, unused objects, per-image EXIF, tracking-style external references. Each item can be viewed. | M | 2 |
| OF03-F151 | Sanitise | Remove any ticked classes from F150 and write a clean copy by full rewrite; verification pass as in F119. | M | 2 |
| OF03-F152 | Strip on share | The share sheet offers "Remove metadata and hidden data" as a default-on option when sharing a PDF. | S | 2 |
| OF03-F153 | Previous versions | Shows each incremental-save revision of a PDF as its own read-only view and can compare two. | S | 3 |
| OF03-F154 | EPUB metadata | View and edit title, creators, language, identifiers, series, cover; stored in the library record, written to the file only on request. | S | 3 |
| OF03-F155 | Tracking defence | No external resource is ever fetched on open, view or print: remote images, fonts, XObject references, "phone home" form actions and URL actions on open are inert and listed in F150. | M | 1 |

### 4.11 Library and reading progress

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F156 | Bookshelf | A library view of documents the user added: cover grid or table. Adding stores a remembered portal grant and a record; files are not copied unless "Copy into library" is chosen. | S | 2 |
| OF03-F157 | Watched folders | Granted folders are scanned for supported files and kept in step with changes. | S | 2 |
| OF03-F158 | Metadata extraction | Title, authors, series and number, publisher, language, year, identifiers (ISBN, DOI), subjects, cover: from OPF, PDF Info/XMP, `ComicInfo.xml`, file name patterns. Editable; edits live in the library record. | S | 2 |
| OF03-F159 | Covers | From the declared cover, else the first page; rendered by the helper; cached in the app cache. | S | 2 |
| OF03-F160 | Collections and tags | User collections (a document can be in many), tags, series grouping, shelves "Reading", "Unread", "Finished", "Recently added". | S | 2 |
| OF03-F161 | Sort and filter | By title, author, series, added, last read, progress, format, size, language; filter chips; type-ahead. | S | 2 |
| OF03-F162 | Reading progress | Last position (page or EPUB CFI), percentage, time read, finished date; shown as a bar on the cover. Updated on page turns, saved at most once per 5 s. | M | 2 |
| OF03-F163 | Continue reading | The start view lists the last five documents with progress; one click resumes. | M | 2 |
| OF03-F164 | Library search | Full-text index of library documents (option per folder): SQLite FTS5 over text extracted in the helper; search hits open at the page. | S | 3 |
| OF03-F165 | Reading statistics | Local only: pages and minutes per day, per book; can be turned off and erased. | C | 3 |
| OF03-F166 | Sidecar portability | Annotations, bookmarks and progress for a document can be exported as one JSON file (`<name>.annot.json`) and imported on another device; OF02's sync can carry the folder. No built-in sync. | S | 3 |
| OF03-F167 | Missing files | Records whose file is gone show "missing" with [Locate…]; state is kept by content hash so a re-added file reconnects. | S | 2 |
| OF03-F168 | Duplicates | Same content hash listed once; same work in several formats can be grouped by the user. | C | 3 |
| OF03-F169 | Metadata lookup | Not offered online. Import of metadata from a local file (OPF, BibTeX, CSV) only. | C | 3 |
| OF03-F170 | Bibliography export | Selected records as BibTeX, RIS or CSL-JSON for OF04's citations. | C | 3 |
| OF03-F171 | Private library option | Per collection: exclude from recent lists, system search, thumbnails and statistics. | S | 3 |
| OF03-F172 | Remove from library | Removes the record and, by choice, the sidecar data; never deletes the file unless asked, then through the trash. | S | 2 |

### 4.12 Hostile-document handling

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F173 | One sandbox per document | Each open document gets its own `render-helper` process in Capsicum capability mode. It holds: a read-only descriptor of the file, a shared-memory region for tiles, and an IPC pipe. It has no directory handle, no socket, no clock beyond monotonic, no keyring, no other document. | M | 1 |
| OF03-F174 | No ambient anything | The helper starts with an empty capability list in the manifest; it cannot open paths, create sockets, start processes, or read the environment. Fonts it needs are passed as read-only descriptors by the app host on request by name hash. | M | 1 |
| OF03-F175 | No scripts, by construction | The PDF engine is built with no JavaScript interpreter. Document, page, field and annotation scripts are parsed only far enough to list them in the hidden-data report. EPUB scripts and PostScript beyond the page description are not run in the viewer. | M | 1 |
| OF03-F176 | No automatic actions | Open actions, additional actions, launch, URI, submit, import, named "print" and rendition actions do nothing. Only "go to" inside the document is performed, and only on a user click. | M | 1 |
| OF03-F177 | Resource limits | Per helper: memory cap (`RLIMIT_DATA`), CPU time per page (default 10 s, then the page shows "too complex" with a Retry at lower quality), recursion and object-count limits, image dimensions (default 16 384 px a side, 256 megapixels), decompression ratio guard (default 1 000:1 and 2 GiB per stream). | M | 1 |
| OF03-F178 | Helper output is untrusted | Every size, count, offset and string from the helper is validated in the app host before use (FUSE-WP-45 contract). Tiles are plain RGBA of a requested size. Text is length-capped and normalised. | M | 1 |
| OF03-F179 | Crash containment | A helper crash shows "This document made the renderer stop" on the affected tab with [Reload] and [Report details]; other tabs are unaffected. Three crashes on one page mark the page as skipped. | M | 1 |
| OF03-F180 | Write path is separate | Saving, redaction, signing and page operations run in a second helper instance that additionally holds a write-only descriptor to a new temporary file. The app host renames it into place through the portal. The reader instance can never write. | M | 2 |
| OF03-F181 | Untrusted provenance default | Files tagged as downloaded or received (IN01, CO01, CO05, IN03) open with: forms read-only, links showing targets but needing two clicks, embedded files not savable until "Trust this document" is clicked. A `--warn` chip says "From <origin>". | M | 2 |
| OF03-F182 | Disposable viewing | "Open in a disposable sandbox" (DESIGN_SYSTEM.md section 5): no sidecar, no recent entry, no cache on disk, no thumbnail offered; everything is gone on close. | S | 2 |
| OF03-F183 | Convert to trusted PDF | Rebuild an untrusted document as a new PDF from its rendered pages plus re-extracted text (rasterise and OCR-free text overlay from the text layer). The output has none of the original objects. | S | 3 |
| OF03-F184 | Password handling | Passwords are passed to the helper over IPC, held in `secure_buffer`, and never logged. Saved passwords live in `svc.keyring` under the file hash. | M | 1 |
| OF03-F185 | Font safety | Embedded fonts are parsed inside the helper only. System fonts come through the host. Hinting bytecode is not executed (auto-hinting or none). | M | 1 |
| OF03-F186 | Thumbnailer isolation | The thumbnail provider registered with `svc.thumbnail` is the same helper with the same limits and a 2 s budget; it renders page 1 only and reads no sidecar. | M | 2 |
| OF03-F187 | Security panel | `View > Document Safety` shows for the open document: sandbox state, limits hit, scripts found (count, not run), actions found, external references found, embedded files, encryption, signature state, provenance. | M | 2 |
| OF03-F188 | Network honesty | `net.none` by default. The only network uses are the user-triggered OCSP/CRL fetch and time-stamp request; they run in a third short-lived `net-helper` that sees only the request bytes, and the `[net]` badge shows while it runs. | M | 2 |

### 4.13 Print, export, share and general

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF03-F189 | Print | `Ctrl+P` through the print portal (G-16): ranges, odd/even, reverse, copies, scale (fit, shrink, actual, custom), auto-rotate and centre, n-up, booklet, poster tiling, with or without annotations and form values, as image (for stubborn files), greyscale. PDF goes to the spooler as PDF after a sanitising rewrite. | M | 2 |
| OF03-F190 | Save and save as | `Ctrl+S` writes in-file annotations and form values by incremental update (keeps signatures) or full rewrite (option "optimise on save"); atomic rename. Read-only handles lead to Save As. | M | 1 |
| OF03-F191 | Export pages as images | PNG, JPEG, WebP, TIFF; DPI 36–1200; ranges; transparent background option. | S | 2 |
| OF03-F192 | Export to other formats | PDF from EPUB, DjVu, XPS, comics and PostScript; PDF/A-2b attempt with a list of what blocked it; SVG per page (vector where possible). | S | 3 |
| OF03-F193 | Encrypt on save | Set or remove passwords with AES-256 (revision 6) only; certificate encryption to recipients from SE04. Older algorithms are read-only. | S | 3 |
| OF03-F194 | Share | `share` intent with the file, selected pages as a new PDF, a snapshot image, or selected text; sanitise option (F152). | S | 2 |
| OF03-F195 | Command palette | `Ctrl+K`: commands, outline entries, pages ("p 120"), open tabs, library titles, settings. | M | 1 |
| OF03-F196 | Session restore | Open tabs, positions and panels return after restart or crash. Disposable tabs never return. | M | 1 |
| OF03-F197 | Open in other apps | Hand-offs: OF04 (import as editable text), OF08 (page as vector drawing), ME01 (extracted images), OF01 (extracted text), OF02 (make a note linked to this page), CO01 (attach), UT06 (print queue), SE05 (encrypt, erase). | S | 2 |
| OF03-F198 | Command line | `document-viewer [file] [--page n] [--dest name] [--find text] [--present] [--disposable]`; arguments resolved to handles by the launcher. | S | 1 |
| OF03-F199 | Touch and e-reader ergonomics | Tap zones (left, right, centre for chrome), swipe paging, double-tap to fit a column, brightness-independent themes. | S | 2 |
| OF03-F200 | Keyboard reading | Space and Shift+Space page with overlap marker; `J`/`K` lines; `N`/`P` pages; `G` go to page; Vim-like keys are a preset. | M | 1 |
| OF03-F201 | Continuous background save of state | Position and sidecar are saved without prompts; closing never asks unless in-file changes are unsaved. | M | 1 |
| OF03-F202 | Help and first run | A short tour document (a PDF in the bundle) shows sandbox, annotations and redaction basics. | S | 1 |
| OF03-F203 | Thumbnail and preview provider | Provides `svc.thumbnail` renderers and the quick-preview pane for SY02 for all supported types. | M | 2 |
| OF03-F204 | Search provider | Offers library titles and, by option, text to `svc.search`; never for private collections or disposable tabs. | S | 3 |
| OF03-F205 | Accessibility of content | Exposes tagged PDF structure (headings, lists, tables, figures with alt text, reading order, language) and EPUB semantics to the a11y tree; untagged pages expose text lines in detected order with a notice. | M | 2 |

## 5. GUI

### 5.1 Window map

```
Main window
├─ Header bar: menu, title, search/palette, [net] (only during OCSP/time-stamp), lock (encrypted), provenance chip
├─ Tab strip
├─ Sidebar (F9): Thumbnails | Outline | Bookmarks | Annotations | Search | Attachments | Layers | Signatures
├─ Toolbar: page field, zoom, layout, tools (select, hand, highlight, note, ink, shape, text, stamp, sign, redact)
├─ Page area (page view; one or two panes) with banner stack and find bar
├─ Inspector (F4): Properties | Annotation | Form fields | Document Safety
├─ Full views: Library · Organise Pages · Presentation · Presenter · Compare
├─ Sheets: Password · Merge · Split · Extract · Insert · Crop · Print options · Export · Sanitise · Redaction apply
│          and report · Sign · Signature details · OCR · Metadata edit · Encrypt · Library add
├─ Popovers: zoom, layout, colour presets, note pop-up, link preview, footnote, reading settings (EPUB), TTS bar
└─ Settings tab
Drawn by the shell: file portal, print dialog, share sheet, key-use prompt (SE04), capability prompts
```

### 5.2 Main window

Shown at 1360×840 with a signed, untrusted PDF open.

```
+------------------------------------------------------------------------------------------+
| [=] Document Viewer — contract.pdf   [ Search  Ctrl+K ]  (⚠ From: mail)  [🔒][▁][▢][✕]  | (1)
+------------------------------------------------------------------------------------------+
| contract.pdf × | paper.pdf | Dune.epub                                              [+]  | (2)
+------------------------------------------------------------------------------------------+
| [◀][ 3 ]/12[▶] | [−][ 110% v][+] [Fit v] [▯▯ v] | [I][✋][H][🗨][✎][▭][T][⎙][✍][█] | [⋯]    | (3)
+------------+-----------------------------------------------------------+-----------------+
| [▦][≡][🔖] | ✓ Signed by A. Citizen · valid · not changed since signing [Details]    (4) | Properties      |
| [✎][🔍][📎]| ! Opened with limits: forms read-only, links held. [Trust this document]    | Title  Contract |
|  +------+  +-----------------------------------------------------------+ Author —        |
|  |  1   |  |                                                           | Pages  12       |
|  +------+  |        +---------------------------------------+          | PDF    1.7      |
|  +------+  |        |  3. TERM                              |          | Size   412 KiB  |
|  |  2   |  |        |  This agreement starts on ▓▓▓▓▓▓▓▓▓▓  |          | SHA-256 9f2c…   |
|  +------+  |        |  and runs for [ 24      ] months.     |          |-----------------|
|  +======+  |        |  ~~~~~ highlighted text ~~~~~    🗨    |          | Document Safety |
|  |> 3   |  |        |                                       |          | Sandbox  on     |
|  +======+  |        |  Signature: [ A. Citizen ✓ ]          |          | Scripts  2 found|
|  +------+  |        +---------------------------------------+          |          not run|
|  |  4   |  |                          (5)                              | Actions  1 held |
|     (6)    | Find [ months      ] [Aa][W][.*]  1 of 4 [<][>] [List] (7)| Ext. refs 0     |
+------------+-----------------------------------------------------------+-----------------+
| Page 3 of 12 (label "3") · 210 × 297 mm · 1 selected · Annotations: in file ● unsaved     | (8)
+------------------------------------------------------------------------------------------+
```

1. Header bar (`headerBar`): provenance chip and lock chip are `toolButton`s opening details.
2. Tabs (`tabView`, FUSE-WP-31); a format icon per tab; disposable tabs have a dashed outline.
3. Toolbar (`toolbar`, FUSE-WP-29): `spinBox`-like page field accepting labels, zoom `comboBox`, fit and layout menus, tool `toolButton`s (radio group), overflow.
4. Banner stack (`banner`): signature verdict, provenance limits, OCR offer, damaged-file report, password, "has scripts that were not run".
5. Page area (`pageView`, FUSE-WP-45) with overlay layer for annotations (ink through canvas input) and real widgets (`lineEdit`, `checkBox`, `comboBox`) over form fields.
6. Sidebar (`sidebar` with `tabBar` icons): thumbnails `gridView`, outline `treeView`, others `listView`.
7. Find bar.
8. Status bar (`statusBar`): page and label, page size, selection, annotation storage mode with unsaved dot, background jobs (rendering, search, OCR).

States: start view when no document is open ("Continue reading" row, [Open…], drop target); loading (first page tiles progressive, sidebar fills); password needed (sheet); damaged (banner + per-page error cards); helper crashed (card with [Reload]); read-only; narrow.

### 5.3 Other views and dialogs

**Library** (full view, 1360×840).

```
+------------------------------------------------------------------------------------------+
| Library      [ Search library ]   [Grid v] Sort [Last read v]        [+ Add…] [Folders…] |
+--------------+---------------------------------------------------------------------------+
| SHELVES      | Continue reading                                                          |
|  Reading   4 |  +-------+  +-------+  +-------+                                          |
|  Unread   31 |  | cover |  | cover |  | cover |                                          |
|  Finished 12 |  |▓▓▓░░░░|  |▓▓▓▓▓▓░|  |▓░░░░░░|   progress bars                          |
| COLLECTIONS  |  Dune 42%   PBSD 88%   Capsicum 9%                                        |
|  Papers   58 | All (104)                                                                 |
|  Manuals   9 |  +-------+  +-------+  +-------+  +-------+  +-------+  +-------+         |
|  Comics   22 |  | cover |  | cover |  | cover |  | PDF   |  | DjVu  |  | ! miss|         |
| FORMAT       |  +-------+  +-------+  +-------+  +-------+  +-------+  +-------+         |
|  PDF EPUB …  |  …                                                                        |
+--------------+---------------------------------------------------------------------------+
```

`gridView` (covers) or `tableView`; `sidebar`. States: empty ("Add books or a folder" + [Add…]), scanning, missing files.

**Organise pages** (full view).

```
+------------------------------------------------------------------------------------------+
| Organise — contract.pdf    [Rotate ↺][↻] [Delete] [Extract…] [Insert v] [Split…] [Merge…]|
+------------------------------------------------------------------------------------------+
|  +-----+  +-----+  +=====+  +=====+  +-----+  +-----+   |  appendix.pdf (drag pages in)  |
|  |  1  |  |  2  |  |> 3  |  |> 4  |  |  5  |  |  6  |   |  +-----+  +-----+  +-----+     |
|  +-----+  +-----+  +=====+  +=====+  +-----+  +-----+   |  |  1  |  |  2  |  |  3  |     |
|  +-----+  +-----+   …        ┆ drop marker              |  +-----+  +-----+  +-----+     |
+------------------------------------------------------------------------------------------+
| 2 of 12 selected · ! Saving will remove 1 signature          [Cancel] [Save As New File…]|
+------------------------------------------------------------------------------------------+
```

`gridView` with multi-select and drag reorder (FUSE-WP-28, FUSE-WP-33).

**Redaction apply and report** (sheet, 640×520).

```
+----------------------------------------------------------------+
| Remove the marked content for good?                            |
| 14 areas on 6 pages · 9 text, 3 image, 2 vector                |
| Fill [■ Black v]   Overlay text [ s 47F      ]                 |
| [x] Remove metadata and hidden data (11 items) [Review…]       |
| [ ] Fixed-width fill (hides length of removed words)           |
| [ ] Maximum safety: turn affected pages into images            |
| i The original file stays as it is. 1 signature will be lost.  |
|                         [Cancel] [Redact and Save As New File…]|
+----------------------------------------------------------------+
| Verification of contract-redacted.pdf                          |
|  ✓ None of 9 removed strings found in text, streams, metadata  |
|  ✓ 14 of 14 areas render as fill only                          |
|  ✓ 0 source objects marked for removal remain                  |
|  ! 2 pages have text close to a redacted area: check visually  |
|                                   [Save Report…] [Open Result] |
+----------------------------------------------------------------+
```

The lower half replaces the upper after the write finishes. A failed check is `--danger`, keeps the output in a temporary place and offers only [Discard] and [Show Problem].

**Signature details** (sheet, 600×480).

```
+--------------------------------------------------------------+
| Signature 1 of 1                                   ✓ Valid   |
| Signer      A. Citizen <a@example.org>                       |
| Signed      2026-11-10 09:14 UTC (time-stamp authority)      |
| Covers      whole document as signed (revision 2 of 3)       |
| After it    form values changed: allowed by certification    |
| Chain       A. Citizen ← Example CA ← Example Root (trusted  |
|             for documents in Passwords and Keys)             |
| Revocation  embedded OCSP response, good at signing time     |
| Algorithm   ECDSA P-256, SHA-256                             |
| [View Signed Version] [Certificate…] [Check Online…] [Close] |
+--------------------------------------------------------------+
```

`propertyGrid` in a `sheet`. [Check Online…] states the endpoint and that it uses the network before doing anything.

**Sign sheet** (560×420): field or drawn rectangle preview (`image`), key `comboBox` (from SE04), reason and location `lineEdit`s, appearance template, [ ] certify with level `comboBox`, [ ] add time-stamp (endpoint shown), pre-sign findings list (F108), [Cancel] [Sign…]. [Sign…] leads to the shell's key-use prompt.

**Merge sheet** (600×460): `listView` of files with drag order, a range `lineEdit` per row ("all", "1-3,7"), [x] keep bookmarks, [x] add a bookmark per file, total pages, [Add Files…] [Merge to New File…].

**EPUB reading settings popover**: font `comboBox`, size `slider`, line height `slider`, margins `slider`, theme swatches, [x] justify, [x] hyphenate, [x] publisher styles, layout ( ) paged ( ) scrolled, columns ( ) 1 ( ) 2.

**Read-aloud bar** (inline above the status bar): [▶/⏸] [⏮ sentence] [⏭] speed `slider`, voice name (from AX01), "Reading page 14", [x] follow.

**Document safety inspector**: `propertyGrid` rows as in callout area of 5.2 plus [Inspect Document…] [Sanitise…] [Convert to Trusted PDF…].

### 5.4 Menus

```
File    Open… Ctrl+O · Open in Disposable Sandbox… · Open Recent ▸ · Library Ctrl+Shift+L
        Save Ctrl+S · Save As… Ctrl+Shift+S · Save a Sanitised Copy… · Export ▸ (Images, Text, PDF, PDF/A, SVG, Annotations)
        Print… Ctrl+P · Share… · Properties Ctrl+I · Document Safety · Close Tab Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Ctrl+C · Copy as Image Ctrl+Shift+C · Select All Ctrl+A
        Find Ctrl+F · Find Next Ctrl+G · Find Previous Ctrl+Shift+G · Search Library…
        Snapshot Tool · Edit Metadata… · Preferences Ctrl+,
View    Sidebar F9 ▸ (Thumbnails, Outline, Bookmarks, Annotations, Search, Attachments, Layers, Signatures)
        Inspector F4 · Zoom ▸ (In Ctrl++, Out Ctrl+-, Actual Ctrl+0, Fit Page Ctrl+1, Fit Width Ctrl+2, Fit Content Ctrl+3)
        Layout ▸ (Single, Continuous, Two-Up, Book, Horizontal, Right-to-Left) · Rotate View Ctrl+R / Ctrl+Shift+R
        Reading Theme ▸ · Trim Margins · Split View · Compare With… · Text Layer · Layers
        Presentation F5 · Full Screen F11 · Read Aloud Ctrl+Shift+A
Go      Previous Page PgUp · Next Page PgDn · First Ctrl+Home · Last Ctrl+End · Go to Page… Ctrl+L
        Back Alt+Left · Forward Alt+Right · Add Bookmark Ctrl+D · Bookmarks ▸
Annotate Highlight H · Underline U · Strike Out K · Note N · Ink D · Shape ▸ · Text T · Stamp ▸ · Image…
        Annotations Panel · Flatten… · Import… · Export Summary… · Send Highlights to Notes · Storage ▸ (In File, Sidecar)
Forms   Highlight Fields · Next Field Tab · Reset Form · Fill and Sign Tool · Import Data… · Export Data… · Field List
Protect Sign… · Certify… · Signatures Panel · Mark for Redaction R · Search and Mark… · Apply Redactions…
        Inspect Document… · Sanitise… · Encrypt… · Convert to Trusted PDF…
Pages   Organise Pages · Rotate ▸ · Delete… · Extract… · Insert ▸ · Merge… · Split…
        Crop… · Page Labels… · Edit Outline · Optimise… · Recognise Text (OCR)…
Help    Document Viewer Help F1 · Keyboard Reference · How Redaction Works · How the Sandbox Works · About
```

"Organise Pages" has no default key; `Ctrl+Shift+P` stays the command palette.

### 5.5 Keyboard and pointer

- Reading keys: Space / Shift+Space, arrows, PgUp / PgDn, Home / End, `J` `K` `N` `P` `G`, `+` `-`, `Z` hold for the loupe, `B` blank in presentation, `Esc` leaves a tool.
- Tools are single letters when no text field has focus (5.4). Pressing the letter twice makes the tool sticky.
- Pointer: drag selects text (select tool) or pans (hand tool, or middle button, or Space held); double click selects a word, triple a line; `Ctrl+wheel` zooms at the pointer; `Shift+wheel` scrolls sideways; `Alt+drag` selects a rectangle; `Ctrl+drag` marquee zoom.
- Context menu, text selection: Copy, Highlight, Underline, Strike Out, Add Note, Mark for Redaction, Search Library, Look Up, Read Aloud from Here, Send to Notes.
- Context menu, page: Add Bookmark Here, Rotate, Extract Page, Snapshot, Properties of Page, Copy Link to Page.
- Context menu, annotation: Edit Comment, Reply, Set Status, Properties, Copy, Delete, Flatten, Bring Forward, Send Back.
- Context menu, thumbnail: Rotate, Delete, Extract, Insert Before/After, Print Page, Export as Image.
- Context menu, library item: Open, Open in New Window, Add to Collection, Edit Details, Mark Finished, Reveal in Files, Export Sidecar, Remove.
- Drag and drop: files onto the window open; onto the organise grid insert; thumbnails out to SY02 extract pages as a PDF; selected text out as `text/plain`; an image region out as PNG; a document onto a collection adds it.
- Pen: draws with the active ink tool; barrel button erases; hover shows the tool cursor. Touch: swipe pages, pinch zoom, long press for the context menu.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start view: Continue reading, recent, [Open…], [Library]. |
| Loading | First page within budget; grey tiles never shown without a scaled preview once any tile exists; "Rendering" job in status bar. |
| Password needed | Sheet with secret field, attempts unlimited, [x] remember in keyring. Wrong password says so; nothing is shown. |
| Error | Unsupported or DRM-locked: full-pane message with the reason and [Open With…]. Damaged: repaired with a report or per-page cards. |
| Helper stopped | Card on the tab; other tabs unaffected (F179). |
| Offline | Normal. [Check Online…] and time-stamp show "No network" from the helper's failure; verification falls back to embedded data. |
| Locked (encrypted document) | Lock chip; contents excluded from thumbnails and system search unless the password is saved. |
| Read-only | Annotation tools still work in sidecar mode; Save becomes Save As. |
| Untrusted | Limits banner (F181) until trusted. |
| Narrow (< 640) | Sidebar and inspector become overlays; toolbar collapses into the overflow; two-up turns into single page. |

### 5.7 Widgets

| Region | Widget | Exists today (V3) | Needs |
|---|---|---|---|
| Page area | `pageView` | no | FUSE-WP-45 (text-layer contract reviewed against this spec) |
| Ink, shapes, measure, loupe | canvas input and overlay | no (G-12) | FUSE-WP-43, FUSE-WP-08 (pen) |
| Form fields | `lineEdit`, `checkBox`, `radioButton`, `comboBox`, `listView` | yes | Placement over pages (FUSE-WP-45) |
| Thumbnails, covers, organise grid | `gridView` | yes, single select | FUSE-WP-28 (multi-select, drag reorder), FUSE-WP-33 |
| Outline, annotations, results | `treeView`, `listView` | limited | FUSE-WP-28 |
| EPUB reflow | pages rendered by the helper into `pageView` | — | Helper-side layout (this spec, W16) |
| Presentation | second window on another output | no | FUSE-WP-07, FUSE-WP-11 |
| Header, toolbar, status, banners, sheets, popovers, palette | chrome kit | no | FUSE-WP-29, FUSE-WP-30 |
| Tabs | `tabView` | basic | FUSE-WP-31 |
| Properties | `propertyGrid` | yes | — |
| Colour management | — | no | FUSE-WP-26 |

`OF03-GAP-1`: `pageView`'s `PageSource` must support variable page counts after a relayout (EPUB font change) and anchors that are not page numbers (CFI). `OF03-GAP-2`: a keyring operation "sign this digest with key K after a shell prompt" exposed to apps (PLATFORM.md section 8 lists `svc.keyring` for secrets only). `OF03-GAP-3`: a system font service that hands a helper read-only font descriptors by name.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Tabs, tools, annotation model, sidecar store, library, form values, verdict presentation, hand-offs |
| Views and styles | FUSE UI | app host | All windows |
| `render-helper` | C++23 wrapper around the document engines | one per open document; capability mode; no capabilities | Parse, render tiles, text layer, outline, links, annotations and fields as records, object inspection, signature byte ranges and CMS parsing, script and action listing |
| `write-helper` (same binary, write role) | C++23 | short-lived, one per save; adds one write-only descriptor | Incremental save, full rewrite, redaction, sanitise, page operations, appearance generation, signature container embedding, verification pass (in a fresh instance) |
| `convert-helper` | C++23 wrapper | short-lived; capability mode | PostScript to PDF; other formats to PDF; image pre-processing for OCR |
| `net-helper` | C++23 | short-lived; only process with a socket | OCSP, CRL and RFC 3161 requests to one shown endpoint |
| Signature verifier | C++23 (in `render-helper` for parsing; chain building asks SE04) | helper + SE04 | CMS and certificate parsing in the sandbox; trust decisions in SE04's service |
| Library index | SQLite FTS5 | app host | Records, progress, text index |
| Model client | ForgeScript | app host | OCR and optional AI through `svc.model` |

```
 SY02/CO01/IN01 ──open intent + handle──▶ app host (ForgeScript + FUSE UI: pageView, overlays, forms)
                                           │  validates every record from helpers
       read-only fd, shm tiles, IPC        ├──▶ render-helper  #1 (doc A)   no caps, no net, no fs
                                           ├──▶ render-helper  #2 (doc B)
       + write-only fd to a temp file      ├──▶ write-helper   (save / redact / sign container) ─▶ verify instance
       request bytes only                  ├──▶ net-helper     (OCSP, CRL, time-stamp; user-triggered)
       digest to sign, after shell prompt  ├──▶ SE04 key service ── returns signature bytes
       page images (shm)                   └──▶ svc.model (OCR)        AX01 speech interface ◀── text, ranges
```

Document engines inside the helpers (third-party, each named with licence and reason; all C, all confined; see section 13 for the plan to replace or prove them):

| Library | Licence | Used for | Why not own code |
|---|---|---|---|
| MuPDF (built without its JavaScript engine and without network code) | AGPL-3.0-or-later (compatible with this project's licence) | PDF, XPS, EPUB, CBZ, FB2 parsing, rendering, text extraction, PDF writing | A full PDF imaging model is many person-years |
| jbig2dec, OpenJPEG, libjpeg-turbo, zlib, FreeType, HarfBuzz (as bundled by MuPDF) | AGPL-3.0, BSD-2-Clause, IJG/BSD, zlib, FTL, MIT | Image filters, fonts | Standard codecs |
| DjVuLibre | GPL-2.0-or-later | DjVu | Format is niche; own decoder later if wanted |
| libarchive | BSD-2-Clause (in the HardenedBSD base system) | CBR (RAR), CB7, CBT | Already in base |
| Ghostscript (only inside `convert-helper`) | AGPL-3.0-or-later | PostScript interpretation | PostScript is a full language; conversion only, never in the viewer path |

All must be vendored at pinned versions and build offline. Their merge is gated by fuzzing, not by the safe-subset check; that is why they live only in processes with no capabilities.

### 6.2 Data model

Sidecar and library store: `~/.local/share/org.pbsd.document-viewer/library.sqlite` (schema version in `meta`); covers and tile cache in `~/.cache/…`.

| Table | Fields |
|---|---|
| `document` | `hash: blob(32) pk` (BLAKE3 of the file), `grant_token: text`, `display_path: text`, `format: u8`, `size: u64`, `title`, `authors: json`, `series`, `series_index: f32`, `language`, `publisher`, `year: i16`, `identifiers: json`, `page_count: u32`, `added: i64`, `missing: bool`, `private: bool`, `provenance: text`, `trusted: bool` |
| `progress` | `hash`, `position: text` (page index or CFI), `fraction: f32`, `last_read: i64`, `seconds_read: u32`, `finished: i64?` |
| `view_state` | `hash`, `zoom: f32`, `fit: u8`, `layout: u8`, `rotation: u16`, `theme: u8`, `sidebar: u8`, `reflow: json` |
| `annotation` (sidecar mode) | `id: uuid`, `hash`, `page: u32`, `kind: u8`, `rects: blob` (quads, f32), `ink: blob`, `colour: u32`, `opacity: f32`, `border: json`, `contents: text`, `rich: text`, `author`, `created`, `modified`, `reply_to: uuid?`, `state: u8`, `anchor_text: text` (for re-anchoring), `cfi: text?` |
| `bookmark` | `id`, `hash`, `position`, `title`, `order: u32` |
| `ocr_text` | `hash`, `page`, `lang`, `words: blob` (text + box + confidence), `model: text` |
| `collection`, `collection_item`, `tag`, `document_tag` | names and joins |
| `text_fts` | FTS5: `hash`, `page`, `text` |
| `saved_password_ref` | `hash`, `keyring_item: text` |
| `journal` | `hash`, `seq`, `op: json` (pending in-file edits: annotations, form values, page ops) |
| `stats_day` | `day`, `hash`, `pages: u32`, `seconds: u32` |

Helper IPC records (schema-compiled, G-08): `DocInfo`, `PageInfo{w,h,rotation,label}`, `TileRequest{page,scale,rect,appearance}`, `TextLayer{glyph boxes, unicode, flags}`, `OutlineNode`, `LinkRecord{rect, kind, target}`, `AnnotRecord`, `FieldRecord`, `SignatureRecord{byte_range, cms, field, revision}`, `HiddenItem{class, object, summary}`, `EditOp` (for the write role), `VerifyReport`.

Migration: forward only, with a copy of the previous database. The cache can be deleted at any time.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| PDF 1.0–2.0 | yes | yes | MuPDF + own C++23 layers (redaction verifier, sanitiser, signature container, appearance generator) | `render-helper` / `write-helper` |
| CMS, X.509, OCSP, CRL, RFC 3161 tokens | yes | CMS, TSQ | own C++23 ASN.1 DER reader and writer (strict, no BER indefinite forms); chain and trust by SE04 | `render-helper` (parse); SE04 (trust); `net-helper` (transport only) |
| EPUB 2/3, FB2 | yes | no | MuPDF HTML/CSS layout; own OPF, nav and CFI code | `render-helper` |
| MOBI/KF8 (no DRM) | yes | no | own code (Proposed) | `render-helper` |
| DjVu | yes | no | DjVuLibre | `render-helper` |
| CBZ, CBR, CB7, CBT | yes | CBZ (export) | libarchive; images by MuPDF codecs | `render-helper` |
| XPS / OpenXPS | yes | no | MuPDF | `render-helper` |
| PostScript, EPS | convert | no | Ghostscript in safer mode | `convert-helper` |
| TIFF, PNG, JPEG, WebP (export, insert) | yes | yes | MuPDF codecs; own WebP not planned (libwebp, BSD-3-Clause, if wanted) | helper |
| XFDF, FDF | yes | yes | own code | `render-helper` |
| Web Annotation JSON, sidecar JSON | yes | yes | ForgeScript JSON | app host (own schema, validated) |
| BibTeX, RIS, CSL-JSON | no | yes | ForgeScript | app host |
| Speech hand-off | — | — | AX01 interface (Proposed in AX01) | app host |

### 6.4 Integration

- Intents handled: `open`, `view`, `print` (a file sent for printing opens the print sheet), `search`. Emitted: `open` (URLs, embedded files, hand-offs), `share`, `print`, `scan` (UT06), `compose` (CO01 with attachment).
- MIME: `application/pdf`, `application/epub+zip`, `image/vnd.djvu`, `application/vnd.comicbook+zip`, `application/vnd.comicbook-rar`, `application/x-cb7`, `application/x-cbt`, `application/vnd.ms-xpsdocument`, `application/oxps`, `application/postscript`, `image/x-eps`, `application/x-fictionbook+xml`, `application/x-mobipocket-ebook`, `image/tiff`.
- Services: `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.notify`, `svc.keyring`, `svc.thumbnail` (as a provider too), `svc.search` (provider, optional), `svc.audit`, `svc.a11y`, `svc.backup` (library and sidecar included; cache excluded), `svc.model` (optional).
- Services provided: thumbnailer and quick preview for the MIME types above; `org.pbsd.document-viewer.Convert(handle, target_mime) -> handle` so OF04, UT06 and CO01 can get a PDF or page images without linking a PDF engine.
- Other apps: SE04 (keys, trust store, signing), AX01 (speech), OF02 (page links, highlights), OF04 (PDF import, bibliography), UT06 (scan, print), SE05 (erase, encrypt), ME05 and UT06 share the OCR model class.
- Verified in repos: only that FUSE has no page view or canvas today (`widgets.hpp`, whole file) and that FORGE lists OCR as a model class to be built (FORGE_BUILD_PLAN.md FORGE-R-090). The third-party libraries were not inspected here; their capabilities are stated from general knowledge and are Unverified in this plan.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Library database, sidecar, journal, cache |
| `fs.portal` | Open, save, export, watched folders |
| `net.none` | Default for the app and for every render and write helper |
| `net.user` | Only for the user-triggered OCSP, CRL and time-stamp requests; passed to `net-helper` alone; off until first use; each endpoint logged |
| `proc.helper:render-helper`, `proc.helper:convert-helper`, `proc.helper:net-helper` | Section 6.1 |
| `dev.print` | Printing |
| `ai.local`, `svc.model` | OCR and optional summaries; off by default |
| `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.notify`, `svc.keyring`, `svc.thumbnail`, `svc.search`, `svc.audit`, `svc.a11y`, `svc.backup` | Section 6.4 |

Not requested: `proc.exec`, camera, microphone, screen, location, USB, GPU compute, `bg.*`, `net.host`, `fs.path`.

Assets: the user's documents; signing keys (never in this app); the trust decision shown to the user; what the user reads (library, progress, statistics); redacted secrets.

| Threat | Mitigation |
|---|---|
| Memory-corruption exploit in a document engine | Engine runs with no capabilities: no files, no network, no processes; per-document process; resource limits; W^X and ASLR from the base system; host validates all output; fuzzing corpus per format |
| Script execution (PDF JavaScript, EPUB script, PostScript) | No interpreter is built in; PostScript only in `convert-helper`, safer mode, no descriptors but input and output |
| Phone-home on open (tracking, IP disclosure, NTLM-style leaks) | No sockets in helpers; no external references followed; actions inert (F155, F176) |
| Decompression and complexity bombs | Limits (F177); per-page failure, not app failure |
| UI spoofing by the document (fake signature panels, fake dialogs drawn on the page) | Verdicts appear only in app chrome (banner, sidebar) never on the page; presentation mode cannot draw chrome-like overlays outside the page box |
| Signature forgery tricks (byte-range gaps, incremental-save shadow content, certification bypass) | F099, F100, F102; "View signed version"; verdict wording never says "valid" for partial coverage |
| Malicious certificate or CMS data | Strict DER parser in the sandbox; trust decided by SE04 |
| Key theft through the viewer | Private-key operations happen in SE04 after a shell prompt showing the document hash; the viewer handles only the digest and the returned signature |
| Signing something other than what was shown | The digest is computed over the bytes the write helper produced from the displayed revision; pre-sign checks (F108); the shell prompt shows the hash that the signature panel shows afterwards |
| Failed redaction (text under black boxes, old revisions, metadata, image remnants) | Content removal, full rewrite, hidden-data removal, independent verification pass, rasterise mode (4.7) |
| Privacy of reading habits | All state local; private collections; disposable mode; statistics optional; nothing offered to system search unless enabled |
| Hostile embedded files | Never auto-opened; saved with provenance tag; untrusted documents hold them back (F181) |
| Helper returns hostile records to exploit the host | Schema-checked IPC, clamped sizes and counts (FUSE-WP-45), ForgeScript bounds checks |
| Malicious thumbnail request path | Same sandbox and limits; 2 s budget (F186) |

Stored: library records, sidecars, journal, view state, optional statistics, optional saved-password references. Leaves the machine: nothing by default; on explicit request only, an OCSP/CRL query (reveals the certificate serial to its issuer) or a time-stamp request (reveals a hash to the authority). Both facts are stated in the confirmation.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `view.default_layout` | enum single, continuous, two_up, book | continuous | |
| `view.default_fit` | enum page, width, content, actual | width | |
| `view.theme` | enum document, sepia, dark, system, custom | document | |
| `view.smooth_scroll` | bool | true | Honours reduced motion |
| `view.remember_per_document` | bool | true | |
| `render.tile_cache_bytes` | int | 268435456 | |
| `render.helper_memory_bytes` | int | 1073741824 | Per document |
| `render.page_cpu_seconds` | int | 10 | |
| `render.max_image_megapixels` | int | 256 | |
| `text.copy_join_hyphens` | bool | true | |
| `annot.storage_default` | enum sidecar, in_file | sidecar | |
| `annot.author` | string | "" | Blank means none written |
| `annot.utc_times` | bool | true | |
| `annot.default_highlight` | colour token | `--warn` at 40% | |
| `forms.highlight_fields` | bool | true | |
| `forms.honour_restrictions` | bool | true | F003 |
| `sign.default_key` | key reference | none | From SE04 |
| `sign.timestamp_url` | string | "" | Empty disables time-stamps |
| `sign.allow_online_revocation` | enum never, ask | ask | |
| `sign.weak_algorithms` | enum warn, reject | warn | SHA-1, RSA under 2048 bits |
| `redact.fill` | colour | black | |
| `redact.sanitise_by_default` | bool | true | |
| `redact.fixed_width` | bool | false | |
| `safety.untrusted_limits` | bool | true | F181 |
| `safety.disposable_for_untrusted` | bool | false | Open untrusted files disposably |
| `library.watch_folders` | list of grant tokens | [] | |
| `library.index_text` | bool | false | |
| `library.statistics` | bool | false | |
| `library.offer_to_system_search` | bool | false | |
| `epub.font_family`, `epub.font_size`, `epub.line_height`, `epub.margin`, `epub.justify`, `epub.hyphenate`, `epub.publisher_styles` | mixed | system serif, 16, 1.5, 48, true, true, true | Reflow defaults |
| `ocr.languages` | list | [system locale] | |
| `ocr.write_mode` | enum sidecar, copy | sidecar | |
| `ai.enabled` | bool | false | Summaries, translation |
| `recent.enabled` | bool | true | |
| `keys.preset` | enum default, vim | default | |

## 9. Accessibility and languages

- Tagged structure is mapped to a11y roles (F205). For untagged documents the reading-order analysis of F141 feeds the tree, with a notice that order is guessed.
- Text ranges, caret browsing (`F7` toggles a caret in the page text) and selection by keyboard work in every format with a text layer.
- Form fields are native widgets, so labels (`TU` tooltips), required state and errors reach assistive tools.
- Every annotation is in the a11y tree with type, author and comment; annotating by keyboard: select text with the caret, press the tool letter.
- Signature and safety verdicts are words plus icons, announced when the document opens.
- Reading themes, reflow and font controls serve low vision; high-contrast mode remaps page colours.
- Right-to-left documents: page order and two-up side follow the viewer preference in the file; text selection follows the bidi run order (needs G-03 on the host side for the UI only; page glyphs come from the helper).
- Scanned documents without text announce "image only, n pages" and offer OCR.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame (start view) | ≤ 200 ms |
| Open to first page, 10 MiB PDF | ≤ 300 ms including helper start (helper pre-spawn pool of 1) |
| Open to first page, linearised 1 GiB PDF | ≤ 500 ms |
| Page render, text page at 150 DPI | ≤ 30 ms in the helper; complex map page ≤ 1 s, progressive |
| Scroll and zoom frame | ≤ 4 ms in the host (FUSE-WP-45) |
| Search, 1 000 pages | first hit ≤ 200 ms; full scan ≤ 3 s |
| Signature verification, 5 signatures | ≤ 300 ms without network |
| Redaction with verification, 100 pages | ≤ 10 s |
| Merge 10 files of 100 pages | ≤ 2 s |
| Helper memory | ≤ 1 GiB cap; typical 60–150 MiB per document |
| Host memory | ≤ 120 MiB plus tile cache (256 MiB default) |
| Library, 20 000 records | list frame ≤ 4 ms; open view ≤ 150 ms |
| Idle | zero frames, zero wake-ups; helpers of background tabs are suspended after 60 s and killed after 10 min (state kept for instant respawn) |

## 11. Testing and proof

Nothing has run. All rows are plans.

- Unit (ForgeScript): annotation model, re-anchoring, form format patterns, range parser (`1-3,7,10-`), verdict logic from signature records, library queries.
- Golden renders: reference pages per format compared with stored images (tolerance stated); chrome in each state; redaction report; signature banners.
- Scripted UI through the a11y tree: tasks T1–T14.
- Fuzz: every format entry point of the helper (PDF objects, xref, each filter, fonts, CMS/X.509 DER, XFDF, EPUB OPF/XHTML/CSS, DjVu chunks, archives, `ComicInfo.xml`), the host-side record validator, sidecar JSON import.
- Differential: rendering against a second engine on a public corpus; text extraction against the same; signature verdicts against a reference validator on a signed-document corpus, including published attack samples (byte-range, shadow, certification).
- Redaction tests: a corpus where the secret appears as text, in images, in vector outlines, in metadata, in bookmarks, in form values, in previous revisions, in object streams, in ToUnicode maps; after redaction a brute-force scanner (all streams decompressed, every encoding) must not find it.
- Confinement tests: helper syscalls outside the capability-mode set fail; no socket, no `open` by path; write role cannot read anything but its input; `net-helper` cannot read the document.
- Limit tests: bombs for each limit of F177; crash injection in the helper.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Host-side record validator | Every size, index and count checked before use | T2 | PRISM (FUSE-WP-45 job) | — |
| DER reader, CMS and X.509 parser (own C++23) | No runtime errors; accepts only canonical DER | T2 | PRISM plus fuzz | Full RFC conformance is test-based |
| Byte-range and revision classifier | A verdict "covers whole document" implies the ranges cover all bytes but the signature value | T1 | PRISM proof on the range arithmetic | — |
| Redaction verifier | If it reports clean, no removed string occurs in any decoded stream of the output | T2 for the scanner; T3 overall | PRISM on the scanner; corpus tests | Depends on the third-party decoder for stream decoding |
| Sanitiser and full rewrite | Output contains only objects reachable from the new catalogue | T2 | PRISM on the graph walk | — |
| Document engines (MuPDF, DjVuLibre, Ghostscript, codecs) | Memory safety | T4 | Fuzzing and sandboxing only | Third-party C outside the safe subset |
| Helper confinement | No capability beyond the passed descriptors | T3 | Capsicum tests on HardenedBSD (G-06) | Kernel enforcement, not proved here |
| Annotation appearance generator | Bounded output, valid content stream | T2 | PRISM plus round-trip tests | — |
| App logic | — | T4 | Tests | Interpreted |

## 12. Build plan

Phase 1, usable core (read PDF safely):

| ID | Work item | Depends on |
|---|---|---|
| OF03-W01 | Bundle, manifest, actions, window, tabs, sidebar, status | G-02, G-07, FUSE-WP-29, FUSE-WP-30, FUSE-WP-31 |
| OF03-W02 | Vendor and build the PDF engine offline, without JavaScript and network code; licence review | FUSE-WP-02 practice, G-18 |
| OF03-W03 | `render-helper`: capability-mode start, descriptors, limits, IPC schema, tile and text records | FORGE-WP-27, FORGE-WP-28, G-06, G-08 |
| OF03-W04 | Page view integration: layouts, zoom, progressive tiles, links, outline, thumbnails | FUSE-WP-45, W03 |
| OF03-W05 | Text selection, copy, find, results list | W04 |
| OF03-W06 | Passwords, damaged files, format detection, properties, tracking defence | W03, `svc.keyring` |
| OF03-W07 | Sidecar store; highlights, notes, bookmarks; view state; session restore; recent | W04 |
| OF03-W08 | Save (incremental) with the write role; journal | W03 |
| OF03-W09 | Fuzzing harnesses and limit tests for PDF | W03 |
| OF03-W10 | Font hand-off to helpers | OF03-GAP-3 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| OF03-W11 | Full annotation set, panel, properties, ink and shapes, XFDF | G-12 (FUSE-WP-43), FUSE-WP-08, W07 |
| OF03-W12 | Forms: widgets over pages, native format patterns, appearance generation | W08 |
| OF03-W13 | Signature verification: DER/CMS parser, byte-range classifier, SE04 trust queries, panel and banner | SE04, W03 |
| OF03-W14 | Hidden-data inspection, sanitise, full rewrite, verification pass | W08 |
| OF03-W15 | Redaction: marking, content removal for text, images, vectors; report | W14 |
| OF03-W16 | EPUB (reflow, CFI, reading settings), DjVu, comics | W03, OF03-GAP-1, G-17 |
| OF03-W17 | Organise pages, merge, split, insert, extract | W08, FUSE-WP-28, FUSE-WP-33 |
| OF03-W18 | Library, covers, progress, collections, watched folders | W07 |
| OF03-W19 | Print and export images and text; share; thumbnail and preview provider | G-16, `svc.thumbnail` |
| OF03-W20 | Untrusted-provenance limits, disposable mode, safety panel | SY02 provenance tags, W06 |
| OF03-W21 | Presentation and presenter view; read-aloud hand-off | FUSE-WP-07, FUSE-WP-11, AX01 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF03-W22 | Signing, certifying, appearance templates, pre-sign checks | OF03-GAP-2, SE04, SH03, W13 |
| OF03-W23 | `net-helper`: OCSP, CRL, time-stamps; long-term validation data | W22, FORGE-WP-21 |
| OF03-W24 | OCR and pre-processing; optional AI features | G-15, FORGE-WP-31, AI02 |
| OF03-W25 | XPS, PostScript (`convert-helper`), FB2, MOBI, TIFF; Convert service for other apps | W03 |
| OF03-W26 | Crop, labels, outline editing, imposition, watermark, optimise, encrypt on save, PDF/A attempt | W17 |
| OF03-W27 | Compare, measure, loupe, reflow for PDF, colour management | FUSE-WP-26 |
| OF03-W28 | Library text index, statistics, sidecar export, bibliography export, shadow-attack checks, convert to trusted PDF | W18, W14 |

## 13. Open questions and risks

1. The document engines are large third-party C code bases. The plan confines them rather than proving them. A staged replacement with own memory-safe C++23 code (object parser and xref first, then filters, then the content interpreter) is possible but is not scheduled here; it needs an owner decision.
2. Licences: MuPDF and Ghostscript are AGPL-3.0-or-later, DjVuLibre GPL-2.0-or-later; they fit an AGPL-3.0-or-later bundle, but the bundle as a whole then cannot be relicensed. RAR 5 support through libarchive is partial (Unverified here).
3. EPUB layout through the PDF engine's HTML and CSS support is limited. A better result needs either the web content engine (G-14) in a no-script, no-network mode or an own reflow layout on FUSE-WP-36. Decision pending; `OF03-GAP-1` applies either way.
4. Signing depends on SE04 offering a digest-signing operation with a shell prompt (OF03-GAP-2) and on hardware-token support there.
5. Redaction of text in complex fonts can shift neighbouring glyphs; the verifier flags "text close to a redacted area", but a human check remains necessary. The app says so.
6. Honouring owner restrictions (F003) is a policy choice; the default follows common practice, the override is one click.
7. Helper-per-document memory cost with many tabs; suspension and respawn (section 10) reduce it but add latency on return.
8. OCR depends on a model class that does not exist yet (FORGE-WP-31) and on which models may ship (owner decision Q11 in FORGE_BUILD_PLAN.md).
9. Unverified: none of the third-party libraries was read in this session; build flags named here (no JavaScript, safer mode) must be confirmed against their sources when vendored.
