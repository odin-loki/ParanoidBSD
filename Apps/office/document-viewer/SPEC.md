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
