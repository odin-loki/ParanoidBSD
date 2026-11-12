<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Writer

By Odin Loch

| | |
|---|---|
| ID | `OF04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.writer` |
| Category | office |
| Priority | P1 |
| Backing repos | FUSE (rich text FUSE-WP-35 and page layout FUSE-WP-36: Proposed, gap G-10), FORGE (ForgeScript, signing, text library: Verified), MathScript (LaTeX subset reader and MathML export: Verified; maths layout: Proposed) |
| Status | Planned. Nothing built. |

## 1. Purpose

A word processor for letters, reports, theses and books. It is built on styles and a real page layout engine, keeps OpenDocument Text as its native format, and exchanges files with DOCX, RTF, HTML, Markdown, PDF and EPUB under a stated fidelity plan. Documents from elsewhere are parsed in a sandbox; macros are signed, sandboxed and off by default. It replaces the word processor the Plasma plan would have taken from outside (PLATFORM.md V20).

## 2. Scope

In scope: everything in section 4, and the precise requirements on the rich-text and layout engine (section 6.5).

Out of scope:

| Item | Owner |
|---|---|
| Vector drawing editor (Writer embeds and hands off) | OF08 Draw |
| Spreadsheets and charts (embedded as objects, edited there) | OF05 Sheets |
| Slides | OF06 Slides |
| Raster image editing beyond crop and simple adjustments | CR01 Image Editor |
| PDF viewing, annotation, redaction, signing of PDFs | OF03 Document Viewer |
| Notes and knowledge base | OF02 Notes |
| Plain text and code | OF01 Text Editor |
| Symbolic maths evaluation | OF07 Calculator, AI04 Notebook (Writer only typesets equations) |
| Key and certificate management | SE04 Passwords and Keys |
| Print queues | UT06 Printers and Scanners |
| Contacts database (a mail-merge source) | CO03 Contacts |
| Sending mail | CO01 Mail |
| Real-time multi-user editing, cloud storage | Not planned |
| Desktop-publishing features: linked colour separations, imposition marks | Not planned |

## 3. Users and top tasks

Personas:

- **Letter writer**: short documents from templates, prints or exports PDF.
- **Report author**: headings, tables, figures, contents, cross-references, review with colleagues.
- **Academic**: citations, bibliography, footnotes, equations, long documents.
- **Editor / reviewer**: tracked changes, comments, compare versions.
- **Clerk**: mail merge to letters, labels and envelopes.
- **Exchange user**: receives DOCX, must return DOCX that looks the same.

| # | Task | Target steps |
|---|---|---|
| T1 | Start a letter from a template | 2 |
| T2 | Apply a heading style | 1 (`Ctrl+Alt+1`) |
| T3 | Insert a table and fill it | 2, then Tab between cells |
| T4 | Insert an image with text wrapped around it | 2 (drop, choose wrap) |
| T5 | Insert and update a table of contents | 2; update 1 (`F9` on it) |
| T6 | Add a footnote | 1 (`Ctrl+Alt+F`) |
| T7 | Cite a source and build the bibliography | 3 |
| T8 | Cross-reference "Figure 3" so it renumbers | 3 |
| T9 | Turn on change tracking and review changes | 1; accept or reject 1 each |
| T10 | Compare two versions | 3 |
| T11 | Export a tagged PDF | 2 |
| T12 | Open a DOCX, edit, save back as DOCX with a fidelity report | open, edit, `Ctrl+S`, confirm format |
| T13 | Mail-merge 200 letters from a contact list | 6 |
| T14 | Remove personal data and sign the document | 4 |
| T15 | Recover after a crash | 0 |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Documents and files

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F001 | New document | `Ctrl+N` blank from the default template; `Ctrl+Shift+N` opens the template gallery. | M | 1 |
| OF04-F002 | Open | Portal or intent. The file is parsed by the `convert-helper` in a sandbox and arrives as engine records; the app host never parses document bytes. | M | 1 |
| OF04-F003 | Native format | ODF 1.3 Text (`.odt`), extended conformance; templates `.ott`; flat XML `.fodt` (useful under version control); master documents `.odm`. | M | 1 |
| OF04-F004 | Save | `Ctrl+S`, atomic (temporary file, `fsync`, rename). Saving to a foreign format shows what will be lost (F180) and offers "Keep a copy in ODT". | M | 1 |
| OF04-F005 | Autosave and recovery | Every engine operation is journalled to the app data directory within 1 s; after a crash the journal is replayed onto the last saved state. Untitled documents persist until discarded. | M | 1 |
| OF04-F006 | Versions | "Save version" stores a named version (ODF `Versions`) inside the file or, by default, as local history in the app store (zstd, 50 versions or 1 GiB); ZFS snapshot versions listed through `svc.backup`. Any version opens read-only or in Compare. | S | 2 |
| OF04-F007 | Tabs and windows | Documents in tabs; a second window or split pane on the same document (two places at once). | M | 1 |
| OF04-F008 | Recent and pinned | Last 50 with thumbnails; pin; clearable. | M | 1 |
| OF04-F009 | External change | A file changed on disk: reload if unmodified, else a banner with Compare, Keep mine, Reload. | M | 1 |
| OF04-F010 | Document properties | Title, subject, keywords, author, description, language, custom properties, statistics; see 4.18 for hygiene. | M | 1 |
| OF04-F011 | Read-only and protected | Open read-only; protect sections, forms-only editing, or track-changes-only with a password (states plainly that this is not encryption). | S | 2 |
| OF04-F012 | Untrusted documents | Files with an untrusted provenance tag open in "Protected view": read-only, no fields updated, no external links resolved, macros unavailable, until [Enable Editing]. | M | 1 |
| OF04-F013 | Session restore | Open documents, positions, zoom, panels. | M | 1 |
| OF04-F014 | Large documents | 5 000 pages, 2 million words, 10 000 images linked or embedded: stays in the budgets of section 10. | M | 2 |
| OF04-F015 | Document statistics | Words, characters, paragraphs, pages, lines, tables, images; for selection and whole document; live in the status bar; excludes or includes notes. | M | 1 |
| OF04-F016 | Command line | `writer [file] [--new template] [--convert-to pdf|docx|… --out handle] [--print]`; conversion runs headless in the helper. | S | 2 |

### 4.2 Text editing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F017 | Typing and caret | Caret with affinity; bidi-aware movement (visual or logical, setting); by grapheme, word, sentence, paragraph, line, page; click below the last line continues the document. | M | 1 |
| OF04-F018 | Selection | Drag, Shift+move, double (word), triple (sentence), quadruple (paragraph); multiple ranges with `Ctrl`; block selection with `Alt+drag`; select all text of one style. | M | 1 |
| OF04-F019 | Undo and redo | Unlimited, grouped by typing bursts and by command; built on invertible engine operations; survives save; undo history list. | M | 1 |
| OF04-F020 | Clipboard | Offers on copy: ODF fragment, HTML, RTF, plain text, PNG for objects. Paste picks the richest safe offer; foreign rich text is converted in the helper. | M | 1 |
| OF04-F021 | Paste special | `Ctrl+Shift+V`: unformatted, keep source formatting, match destination styles, as picture, as link to source (for Sheets ranges). Default per source in settings. | M | 1 |
| OF04-F022 | Find and replace | `Ctrl+F`, `Ctrl+H`: case, whole word, regex (FORGE regex), in selection, backwards, including notes, comments, headers. Search by format: character or paragraph attributes, style; replace with text and/or format or style. | M | 1 |
| OF04-F023 | Find all and navigate | Results list in the sidebar; "Select all matches" for bulk formatting. | S | 2 |
| OF04-F024 | Special characters | Dialog with Unicode blocks, search by name, recent; hand-off to AX04's picker; shortcuts for non-breaking space (`Ctrl+Shift+Space`), no-break hyphen, soft hyphen (`Ctrl+-`), zero-width joiners, en and em dash. | M | 1 |
| OF04-F025 | Formatting marks | `Ctrl+F10` shows paragraph ends, spaces, tabs, breaks, anchors, hidden text, field shading, object boundaries. | M | 1 |
| OF04-F026 | Autocorrect | Replacement table, capitalise sentences, fix two initial capitals, ordinal suffixes, fractions, smart quotes and dashes per language, URL recognition; every rule can be turned off; each correction is one undo step back to what was typed. | S | 2 |
| OF04-F027 | AutoText | Named reusable blocks (text with formatting, tables, images), inserted by name + `F3`. | S | 2 |
| OF04-F029 | Drag and drop of text | Move or copy (`Ctrl`) selections inside and between documents. | M | 1 |
| OF04-F031 | Hidden text and conditional text | Character attribute "hidden"; paragraphs and sections hidden by a condition on a field or variable. | S | 3 |
| OF04-F032 | Hyperlinks | `Ctrl+Alt+K`: to URL, document, heading, bookmark, e-mail. Links never open without `Ctrl+click`, and show the real target. | M | 1 |
| OF04-F033 | Bookmarks | Named anchors over a point or range; listed in the navigator; targets for links and references. | M | 2 |
| OF04-F034 | Sorting and case | Sort paragraphs or table rows (up to 3 keys, locale collation, numeric); change case: upper, lower, title, sentence, toggle, small caps attribute. | S | 2 |

### 4.3 Character and paragraph formatting

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F035 | Fonts | Family, style, size (1–999 pt, half-point steps), separate Western, Asian and complex-script fonts, language per run. Missing fonts are substituted and listed. | M | 1 |
| OF04-F036 | Character effects | Bold, italic, underline (9 styles, colour), overline, strike-through (single, double), superscript and subscript (auto or by offset), small caps, all caps. Shadow, outline and emboss effects are import-preserved only. | M | 1 |
| OF04-F037 | Colour and highlight | Text colour, highlight, character background; palette from theme tokens, document colours, custom; "automatic" follows background. | M | 1 |
| OF04-F039 | OpenType features | Standard and discretionary ligatures, contextual alternates, old-style and lining figures, proportional and tabular figures, fractions, stylistic sets 1–20, true small caps, swashes, language-specific forms. | S | 2 |
| OF04-F040 | Alignment and indents | Left, centre, right, justified, justify last line; before, after, first-line and hanging indents; mirrored for right-to-left; ruler handles. | M | 1 |
| OF04-F041 | Line and paragraph spacing | Single, 1.15, 1.5, double, proportional, at least, exact, leading; space above and below; "no space between paragraphs of the same style"; register-true (baseline grid). | M | 1 |
| OF04-F042 | Tabs | Left, right, centre, decimal (custom decimal character), leader characters; default tab distance; set on the ruler. | M | 1 |
| OF04-F043 | Text flow | Widow and orphan control (lines), keep with next, keep lines together, page or column break before or after with optional page style and page number restart. | M | 1 |
| OF04-F044 | Hyphenation per paragraph | On or off, minimum word length, characters before and after the break, maximum consecutive hyphens, no hyphenation of capitals or last word, hyphenation zone. | M | 2 |
| OF04-F045 | Borders, shading, padding | Per side line style, width, colour; padding; background colour; merge borders of adjacent paragraphs. | M | 2 |
| OF04-F046 | Drop caps | Letters or whole word, 2–9 lines, distance to text, character style. | S | 2 |
| OF04-F047 | Outline level and numbering link | Paragraph outline level 1–10 (feeds contents and navigator); attach to a list style; restart or skip numbering; line numbering inclusion. | M | 1 |
| OF04-F048 | Clone formatting | Paintbrush: copy character and paragraph formatting, single use or sticky. | S | 1 |
| OF04-F049 | Clear direct formatting | `Ctrl+M`; the inspector shows direct formatting as overrides on top of the style, each removable. | M | 1 |
| OF04-F050 | Asian and complex typography | Ruby text, emphasis marks, vertical text in frames and pages, kinsoku rules, hanging punctuation, kashida justification, combined characters. | S | 3 |

### 4.4 Styles

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F051 | Style families | Paragraph, character, page, list, table, frame (graphic) styles; each with a name, parent, and for paragraph styles a "next style". | M | 1 |
| OF04-F052 | Inheritance | A style stores only differences from its parent; the inspector shows where each value comes from (style, parent, default, direct). | M | 1 |
| OF04-F053 | Styles panel | Sidebar list per family: all, used, custom, hierarchical; live preview in each row; filter; apply by double click or Enter; `Ctrl+Alt+0…9` for body and headings. | M | 1 |
| OF04-F054 | Create and modify | New from selection; update style to match selection; modify dialog with all attributes; delete with replacement style. | M | 1 |
| OF04-F057 | Style-first formatting bar | The toolbar's first control is the paragraph style box; direct-formatting buttons are present but styles are one key away. | M | 1 |
| OF04-F059 | Themes | Document theme: heading and body fonts, a 12-colour palette, spacing scale; switching the theme restyles all dependent styles. | S | 3 |
| OF04-F060 | Default styles shipped | Body Text, Headings 1–10, Title, Subtitle, Quotations, Preformatted Text, List families, Caption, Footnote, Endnote, Header, Footer, Contents 1–10, Index, Bibliography, Table Contents, Table Heading, Frame Contents; character styles Emphasis, Strong, Source Text, Internet Link, Footnote Anchor, Page Number. | M | 1 |

### 4.5 Lists and numbering

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F061 | Bullets and numbering | Toggle with `Shift+F12` and `F12`; 10 levels; Tab and Shift+Tab change level; Enter on an empty item ends the list. | M | 1 |
| OF04-F062 | List styles | Per level: number format (1, a, A, i, I, native numerals of the language), prefix and suffix, show sublevels (1.2.3), start value, bullet character or image, character style, alignment, indent, tab or space after. | M | 1 |
| OF04-F063 | Heading numbering | One dialog ties heading styles to an outline list ("1", "1.1", "Appendix A"). | M | 2 |
| OF04-F064 | Continue, restart, set value | Per paragraph; continue a previous list across other content. | M | 2 |
| OF04-F066 | Line numbering | Margin line numbers: every n lines, restart per page, count blank lines, character style, position. | S | 3 |
| OF04-F067 | Legal and multi-list documents | Several independent numbered sequences (clauses, schedules) with their own list styles and cross-references to numbers. | S | 3 |

### 4.6 Page layout

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F068 | Page styles | Size (ISO, US, envelopes, custom up to 5 m), orientation, margins (mirrored, gutter), background (colour, image), borders, layout (right and left, mirrored, right only), numbering format, next page style, first-page variant. | M | 1 |
| OF04-F069 | Sections | Named ranges with their own column count, indents, background, write protection, hide condition, link to another file, and footnote or endnote collection point. | M | 2 |
| OF04-F070 | Columns | 1–20 columns per page style, section or frame; equal or custom widths; gutter; separator line; balanced content; column breaks. | M | 2 |
| OF04-F071 | Headers and footers | Per page style; same or different on left, right and first pages; height fixed or auto; spacing; any content including tables, images and fields. | M | 1 |
| OF04-F072 | Page numbers | Field with format; "page x of y"; restart at a break; offset; chapter-prefixed ("3-12"). | M | 1 |
| OF04-F073 | Footnotes and endnotes | Auto or custom marks; numbering per page, chapter or document; numerals or symbols; separator line; continuation notices ("continued on next page"); notes split across pages; endnotes at end of section or document; convert between the two. | M | 2 |
| OF04-F074 | Breaks | Line, column, page (with style and number), section; manual breaks shown and deletable. | M | 1 |
| OF04-F075 | Views | Print layout (pages), web layout (continuous, wraps to window), draft (no page furniture), outline (headings with fold and drag), read mode, focus mode; side-by-side and book preview. | M | 1 |
| OF04-F076 | Zoom | 20–600%; fit width, whole page, n pages across. | M | 1 |
| OF04-F077 | Rulers and guides | Horizontal and vertical rulers with units (mm, cm, in, pt, pc, ch, lines); margin and column handles; text boundaries; snap grid for frames. | M | 1 |
| OF04-F078 | Watermark | Text or image behind the body on chosen page styles; stored as a standard header-anchored shape. | S | 2 |
| OF04-F081 | Envelopes and labels | Wizards: envelope formats and printer feed orientation; label sheets by brand-neutral dimensions table and custom; synchronised label content. | S | 3 |
| OF04-F083 | Print preview | Exactly the page boxes that will be printed; one, two or many pages; shows non-printing areas of the chosen printer. | M | 2 |

### 4.7 Tables

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F084 | Insert and draw | `Ctrl+F12` or grid picker; rows × columns up to 4 096 × 256; convert text to table and back (separator choice). | M | 1 |
| OF04-F085 | Editing | Tab moves and adds rows; insert and delete rows, columns, cells; merge and split cells (horizontal and vertical); split table; merge tables. | M | 1 |
| OF04-F086 | Sizing | Drag borders; exact widths; distribute evenly; optimal width and height; table width fixed, relative (percent) or automatic; alignment and indents. | M | 1 |
| OF04-F087 | Cell formatting | Borders per edge (style, width, colour), padding, background, vertical alignment, text direction, wrap, shrink-to-fit off by design. | M | 1 |
| OF04-F088 | Table styles | Whole-table style with first and last row and column, banding, corner cells; shipped set; make from selection. | S | 2 |
| OF04-F089 | Page breaking | Allow table to split; allow rows to break across pages; repeat n heading rows; keep with next paragraph. | M | 2 |
| OF04-F090 | Nested tables | Tables in cells to depth 8. | S | 2 |
| OF04-F091 | Formulas in cells | `=sum(<A1:A4>)`, arithmetic, `mean`, `min`, `max`, `round`, `if`; number formats (locale, currency, percent, date); number recognition on entry (option). | S | 2 |
| OF04-F093 | Captions and numbering | Caption above or below with a sequence field ("Table 1"); auto-caption option per object type. | M | 2 |
| OF04-F094 | Paste from Sheets | As table (native), as linked object (updates on request), or as picture. | S | 2 |
| OF04-F095 | Accessibility properties | Header row marking, summary and title text used by tagged PDF and screen readers. | M | 2 |
| OF04-F096 | Wide and long tables | 100 000 rows stay within the frame budget (only visible rows are laid out; row heights cached). | S | 3 |

### 4.8 Images, frames and drawings

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F097 | Insert image | From file (portal), clipboard, drop, scan (`scan` intent), screenshot (ME05). Formats decoded by the system image service: PNG, JPEG, WebP, GIF, TIFF, SVG subset, AVIF; EMF/WMF and EPS converted in the helper. | M | 1 |
| OF04-F098 | Embed or link | Embedded by default. Linked images hold a portal grant; missing links show a placeholder; "break links" embeds. Links to network locations are never fetched on open. | M | 2 |
| OF04-F099 | Anchoring | To page, paragraph, character, or as character (inline). The anchor is visible and draggable. | M | 1 |
| OF04-F100 | Wrapping | None, before, after, parallel (both sides), through, optimal (larger side), in background; contour wrap with an editable polygon; first-paragraph-only; spacing per side. | M | 2 |
| OF04-F101 | Position and size | Relative to page, margin, paragraph, character, line; mirrored on even pages; keep ratio; size as percent of page or paragraph width; follow text flow. | M | 2 |
| OF04-F102 | Image adjustments | Crop (also to shape), rotate any angle, flip, brightness, contrast, gamma, transparency, greyscale and black-and-white modes; compress (resample to a DPI, re-encode); replace image keeping layout. Original kept until compressed. | S | 2 |
| OF04-F103 | Alt text and decorative flag | Title, description, "decorative"; checked by the accessibility checker. | M | 1 |
| OF04-F104 | Text frames | Floating boxes with their own columns, borders, background, auto-size; linked frames (text flows from one to the next). | S | 2 |
| OF04-F105 | Basic shapes in place | Line, arrow, rectangle, ellipse, text box, simple connector, call-out, drawn directly; grouping, alignment, distribution, z-order. Full editing by hand-off. | S | 2 |
| OF04-F106 | Drawing hand-off | "Edit in Draw" opens the drawing object in OF08 by intent with a handle to the embedded ODF Drawing sub-document; saving there updates the object and its replacement image. | S | 2 |
| OF04-F107 | Embedded objects | Sheets ranges and charts (OF05), slides (OF06), formulas (4.12). Shown through their stored replacement image; edited by hand-off. OLE objects from foreign files are kept as opaque blobs with their preview and never activated. | S | 3 |
| OF04-F108 | Captions | For images, frames, tables, objects; category sequences; numbering by chapter. | M | 2 |
| OF04-F110 | Image metadata | EXIF, XMP and thumbnails inside inserted images are stripped on insert by default (setting); see 4.18. | M | 1 |

### 4.9 Fields, references, contents, index, bibliography

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F111 | Document fields | Page number, page count, date and time (fixed or updating), title, subject, author, file name, chapter name and number, statistics, template name. | M | 1 |
| OF04-F112 | Variables | Set, show, user fields, number-range (sequence) fields, input fields, input lists (drop-down), placeholders, conditional text, hidden text and paragraph. Expressions use a small formula language, not scripts. | S | 2 |
| OF04-F113 | Field handling | Shaded on screen (`Ctrl+F8`); toggle names and values (`Ctrl+F9`); update selected (`F9`) or all; fields in protected view are not updated; "convert to text". | M | 1 |
| OF04-F114 | Cross-references | To headings, numbered paragraphs, bookmarks, footnotes, endnotes, captions (figures, tables, user sequences), set-reference marks. Formats: page, chapter, reference text, "above/below", number, number with context, caption category and number, caption text. Each is a live field and a link. | M | 2 |
| OF04-F115 | Table of contents | From outline levels, chosen styles and index marks; levels 1–10; entry structure per level (number, text, tab with leader, page, hyperlink); styles per level; protected against manual edits (toggle); several per document; scope: document or chapter. | M | 2 |
| OF04-F116 | Other tables | Lists of figures, tables, objects by caption category; user-defined indexes from marked entries. | M | 2 |
| OF04-F117 | Alphabetical index | Entries with two sub-keys, main-entry flag, range entries, "see" and "see also"; concordance file (word list) for auto-marking; combine identical entries, page ranges ("12–15", "12f."), locale collation, alphabetical separators, columns. | S | 3 |
| OF04-F118 | Bibliography database | Per-document reference store (kept inside the ODT as CSL-JSON) plus a user library in the app store; import BibTeX, RIS, CSL-JSON, EndNote XML; export the same. Fields follow CSL types. | S | 3 |
| OF04-F119 | Citations | Insert citation (`Ctrl+Alt+B`): search the library, add locator (page, chapter), prefix, suffix, suppress author; multiple sources per citation; stored as fields. | S | 3 |
| OF04-F120 | Citation styles | A CSL 1.0.2 processor (own code, Proposed) with shipped styles: author–date, numeric, note, and common named styles; user `.csl` files loaded as data. Footnote-style citations create notes. | S | 3 |
| OF04-F121 | Bibliography | Generated list in the chosen style; sorted and formatted by the processor; updates with citations; convert to static text. | S | 3 |
| OF04-F122 | Reference integrity | Broken references are shown as "Error: Reference source not found" in `--danger`, listed in the navigator with fix actions. | M | 2 |
| OF04-F123 | Update policy | Fields, contents and indexes update on demand, before print and export, and optionally on open for trusted documents; never for untrusted ones. | M | 2 |

### 4.10 Review

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F124 | Track changes | `Ctrl+Shift+C` records insertions, deletions, moves, attribute changes, paragraph-format changes, table row and column changes, with author and time. | M | 2 |
| OF04-F125 | Display of changes | Inline colours per author with strike-through, or balloons in the margin; "final", "final with markup", "original", "original with markup"; change bars at the page edge. | M | 2 |
| OF04-F126 | Accept and reject | One at a time (`Alt+]` accept, `Alt+[` reject, then move to next), in selection, by author, by date, by type, all. | M | 2 |
| OF04-F127 | Manage changes panel | Sidebar list with filters (author, date, type, text), comments on changes, jump to change. | M | 2 |
| OF04-F128 | Comments | `Ctrl+Alt+C` on a point or range; threaded replies; resolve and reopen; author and time; rich text limited to bold, italic, lists, links; margin display with connector lines; print in margin or as a list. | M | 2 |
| OF04-F129 | Reviewer identity | Name and initials from a Writer setting (may be blank or a pseudonym); optional removal of times; colours assigned per author automatically. | M | 2 |
| OF04-F130 | Compare documents | Choose a second document or version: differences become tracked changes in a new document; granularity by character, word or paragraph; ignore formatting, case, whitespace, moved text detection. | S | 3 |
| OF04-F131 | Merge documents | Combine tracked changes from several reviewers' copies of the same original; conflicts shown as overlapping changes to resolve. | S | 3 |

### 4.11 Mail merge and templates

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F135 | Data sources | CSV, ODS and XLSX sheets (through OF05's converter interface), vCard files, a CO03 contact group (by intent, after a prompt), SQLite files (read-only, parsed in the helper). No live network databases; OF09 can export a table. | S | 3 |
| OF04-F136 | Merge fields | Insert from the data source panel; address block and greeting line builders with locale rules; conditional fields ("if empty, skip line"); next-record field for labels. | S | 3 |
| OF04-F137 | Filter and sort | Choose records by tick, range, filter expression; sort by columns. | S | 3 |
| OF04-F138 | Preview | Step through records in place before producing anything. | S | 3 |
| OF04-F139 | Output | One combined document, one file per record (name pattern from fields), print, PDF per record, or hand the set to CO01 as drafts (never sent by Writer). | S | 3 |
| OF04-F140 | Templates | `.ott` files in the bundle, the user template folder, and granted folders; gallery with categories, preview, default per category; "Save as template"; documents remember their template and offer to update styles when it changes. | M | 1 |
| OF04-F141 | Shipped templates | Blank, letter (several locales), report, memo, minutes, CV, thesis, book, article with two columns, invoice, envelope and label sets. | S | 2 |
| OF04-F142 | Template safety | Templates are documents: same sandboxed parsing, no macros run from templates unless signed and trusted. | M | 1 |

### 4.12 Equations

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F144 | Equation objects | Inline or display; stored as MathML (Presentation) inside the ODF with the source text as an annotation; numbered with a right-aligned sequence field. | S | 2 |
| OF04-F145 | Input | `Alt+=` opens an inline editor: type the LaTeX subset that MathScript reads (Verified: `MATHSCRIPT:docs/LATEX_SUBSET.md`), or a linear "a/b", "x^2" form, or pick from a symbol palette. Input outside the subset is rejected with a position, never guessed. | S | 2 |
| OF04-F146 | MathScript hand-off | The source is parsed and exported to MathML by MathScript's notation module (Verified export names `latex`, `mathml`: `MATHSCRIPT:docs/API.md:203`) in a helper process. Writer does not evaluate. "Open in Notebook" sends the expression to AI04. | S | 2 |
| OF04-F147 | Maths layout | A MathML box layout (fractions, scripts, radicals, stretchy delimiters, matrices, alignment) using the font's OpenType MATH table; Proposed (`OF04-GAP-2`). Baseline aligns with the text line. | S | 2 |
| OF04-F149 | Import of foreign equations | OMML (DOCX) is converted to MathML in the helper; equation images stay images. | S | 3 |

### 4.13 Language tools and typography

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F150 | Spell check | As you type and on demand (`F7`), through the spelling service (G-17); language per run; suggestions, ignore, add; per-document exclusions. | M | 2 |
| OF04-F151 | Grammar check | Local rule-based checker through the same service (agreement, repeated words, punctuation spacing, common confusions) per language; optional local-model checking with `ai.local`, off by default. No text leaves the machine. | S | 3 |
| OF04-F152 | Thesaurus | `Ctrl+F7`, from local data in the language service. | S | 3 |
| OF04-F153 | Language assignment | Per character run, paragraph, style, document default; "none" to skip checking; detection suggestion for pasted text. | M | 2 |
| OF04-F154 | Hyphenation | Pattern-based per language (G-17), exceptions list, manual soft hyphens, interactive hyphenation dialog. | M | 2 |
| OF04-F155 | Justification quality | Paragraph-level line breaking (total-fit, see 6.5 R-L07) with penalties for hyphens and rivers; word-spacing limits (min, optimum, max), optional letter-spacing and glyph-scaling limits (±2%); last-line rules. Greedy mode for compatibility with foreign documents. | S | 2 |
| OF04-F156 | Ligatures and kerning | On by default where the font and language allow; suppressed across morpheme boundaries by a zero-width non-joiner the user can insert. | M | 2 |
| OF04-F158 | Typographic quotes and spaces | Language rules for quotes, apostrophes, narrow no-break spaces before punctuation (French), dashes. | S | 2 |
| OF04-F159 | Font embedding | Embed used fonts in ODT and PDF (subset), honouring the font's embedding permission bits; report fonts that forbid it. | S | 2 |
| OF04-F161 | Local AI help | Optional (`ai.local`): rewrite selection, summarise, translate, suggest headings; always shown as tracked changes the user accepts. | C | 3 |

### 4.14 Long documents

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F162 | Navigator | `F5`: tree of headings, tables, frames, images, objects, bookmarks, sections, links, references, indexes, comments, fields, footnotes; search; drag headings to move chapters; promote and demote with sub-headings. | M | 1 |
| OF04-F163 | Outline folding | Fold body text under headings in the document view; folded parts are skipped by print only when asked. | S | 2 |
| OF04-F164 | Master documents | `.odm` with linked sub-documents (each a portal grant); shared styles from the master; continuous page numbers, notes, captions, cross-references across files; export as one ODT. | S | 3 |
| OF04-F165 | Chapter numbering and running heads | Chapter fields in headers; "first/last heading on page" fields. | S | 2 |
| OF04-F166 | Go to | `Ctrl+G`: page, heading, bookmark, table, figure, comment, footnote, change. | M | 1 |

### 4.15 Accessibility checker

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F168 | Checks | Missing alt text; heading levels skipped; fake headings (bold large text) and fake lists; tables without header rows or with merged cells; low contrast text (< 4.5:1); missing document title and language; links whose text is the URL; blinking or tiny text; empty paragraphs used for spacing; manual numbering; floating text with unclear reading order; footnotes typed by hand; colour-only meaning (heuristic). | M | 2 |
| OF04-F169 | Fix flow | Sidebar list by severity; each item jumps to the place and offers a fix (add alt text, apply heading style, set header row); "ignore" with a reason stored in the document. | M | 2 |
| OF04-F170 | Check before export | Runs before tagged PDF and EPUB export; export proceeds with a summary of open issues. | S | 2 |

### 4.16 Import and export

Fidelity classes used below: **A** round trip without visible change; **B** same pages and text, minor differences listed; **C** content kept, layout approximate; **D** text and structure only. The plan per format is in section 6.3.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F171 | ODF | Read ODF 1.0–1.3 and 1.4 where published; write ODF 1.3 extended. Unknown elements and foreign namespaces are preserved through a round trip. Target class A. | M | 1 |
| OF04-F172 | DOCX import | OOXML WordprocessingML (transitional and strict): styles, numbering, sections, headers and footers, tables, images, drawing shapes (basic), text boxes, fields, contents, footnotes, comments, tracked changes, bookmarks, content controls (as fields), OMML maths, themes, compatibility settings that affect layout. Target class B. | M | 2 |
| OF04-F173 | DOCX export | Writes transitional OOXML that validates; maps styles, lists, sections, tables, anchored objects, fields, notes, comments, changes, maths (OMML). Target class B. | M | 2 |
| OF04-F174 | DOC (binary) import | Word 97–2003 binary: text, styles, tables, images, fields, notes, sections. Read only. Target class C. | S | 3 |
| OF04-F175 | RTF | Read RTF 1.9 (text, styles, tables, images, lists, fields, notes, sections) and write it. Target class C. Embedded objects in RTF are discarded with a notice. | S | 2 |
| OF04-F176 | HTML | Import through the sanitising converter (no scripts, no remote fetch; local images only if in the same granted folder): headings, lists, tables, links, images, inline styles to nearest styles. Export clean HTML5 with a class per style and an optional single-file mode. Class D in, C out. | S | 2 |
| OF04-F177 | Markdown | Import CommonMark with tables, footnotes and task lists to styles; export with headings, emphasis, lists, tables, links, images (to a sibling folder), footnotes; unsupported layout is dropped and listed. Class D. | S | 2 |
| OF04-F178 | Plain text | Import with encoding detection (FORGE text library); export with encoding, line-ending and wrap choices. | M | 1 |
| OF04-F179 | PDF export | PDF 1.7 or 2.0; tagged (PDF/UA-1 target) with headings, lists, tables, figures with alt text, language, reading order, artefacts for headers and footers; bookmarks from headings; links; PDF/A-2b and 3b; font subsetting; image compression choices; page ranges; comments as annotations or in margin; initial view. Output comes from the same page boxes as the screen. | M | 2 |
| OF04-F180 | Fidelity report | On every foreign import and export: a list of features that were approximated, dropped or preserved-but-not-shown, each with a location link. The report can be saved. Before a lossy save the user chooses: save anyway, save as ODT too, cancel. | M | 2 |
| OF04-F181 | PDF import | Hand-off: OF03's Convert service extracts text, structure and images; Writer builds an editable document (class D for untagged, C for tagged). The result is marked "reconstructed". | C | 3 |
| OF04-F182 | EPUB export | EPUB 3.3 reflowable: chapters split at a heading level, navigation document, styles as CSS, images, footnotes as pop-up notes, cover, metadata, language, accessibility metadata; optional fixed layout from page boxes. | S | 3 |
| OF04-F187 | Preserve unknown content | Foreign constructs with no Writer equivalent (SmartArt, OLE, content-control bindings, custom XML parts) are stored as opaque parts with a preview and written back unchanged when saving to the same format, unless the user edited around them. | M | 2 |
| OF04-F188 | Font substitution table | Metric-compatible substitutes for common proprietary fonts; per-document mapping shown and editable; substitution is listed in the fidelity report. | M | 2 |

### 4.17 Macros

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F189 | Off by default | Macro support is disabled until turned on in settings. Documents never run code on open, save, print or any other event unless the user enabled a trusted macro for that event. | M | 3 |
| OF04-F190 | Language | ForgeScript only, against a `writer::` API: document, selection, ranges, styles, tables, fields, find and replace, properties. Interpreted; no JIT (bytecode only, Verified: `FORGE:script/src/runtime.cpp:729-730`). | S | 3 |
| OF04-F191 | Sandbox | A macro runs in its own engine instance with an empty capability set: no file, network, process, environment or reflection access. It can touch only the document it was started on, through the API. Budgets: time 10 s, memory 256 MiB, operation count; all edits are one undo step. | M | 3 |
| OF04-F192 | Extra grants by prompt | A macro that needs a file (mail-merge data, export target) asks through the portal like any app; the handle is scoped to that run. No network grant exists for macros. | S | 3 |
| OF04-F193 | Signing and trust | Macros are signed (Ed25519 over BLAKE3, detached, Verified mechanism: `FORGE:docs/scripting.md`). Sources: user macros signed with the user's key from SE04; document macros run only if signed by a key the user marked trusted for macros. Unsigned or changed macros can be read, never run. | M | 3 |
| OF04-F194 | Foreign macros | VBA and other embedded code in DOCX/DOC, and Basic or script macros in ODF files, are never run. They are listed in the safety panel, can be viewed as text, are dropped on save by default, or preserved as inert parts on request. | M | 2 |
| OF04-F195 | Recorder and editor | Record actions into readable ForgeScript; edit in a `codeView` dialog with API completion from a static list; run, step, stop; output panel. | C | 3 |
| OF04-F196 | Macro management | Library list, shortcuts, toolbar buttons, import and export; every run and every trust change is written to `svc.audit`. | S | 3 |

### 4.18 Metadata hygiene, signing, encryption

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F197 | Minimal metadata by default | New documents store no author name, no editing time, no revision count, no printer name, no template path, no generator build string beyond "PBSD Writer" and the ODF version, unless the user turns each on. | M | 1 |
| OF04-F198 | Inspect document | A report of everything that could reveal more than the visible text: properties and custom properties, author names in comments and changes, tracked changes, hidden text and sections, comments, versions, embedded file names and paths, link targets, image metadata, cropped image areas, embedded fonts' names, field history, previous-save remnants, macros, unknown preserved parts, printer and template settings, document ID values (`rsid` values from DOCX). | M | 2 |
| OF04-F199 | Clean | Remove any ticked classes; accept or reject all changes; flatten cropped images (crop for real); rename authors to "Author"; strip `rsid` values. One undoable action. | M | 2 |
| OF04-F200 | Clean on export and share | PDF export, DOCX export and the share sheet offer "Remove personal data" (default on for share); export never includes hidden content unless ticked. | M | 2 |
| OF04-F201 | Save is a full rewrite | Saving always writes a fresh package: no leftover streams from earlier saves, deterministic zip order and timestamps (fixed epoch), so two saves of the same content are byte-identical. | M | 1 |
| OF04-F202 | Digital signatures in ODF | Sign and verify document signatures (`META-INF/documentsignatures.xml`, XML-DSig with XAdES-B, and OpenPGP-based signatures of ODF 1.3) with keys and trust from SE04. The private-key operation happens in SE04 after a shell prompt. | S | 3 |
| OF04-F203 | Signature status | Banner and status-bar badge: valid, valid but document changed, unknown signer, invalid; what the signature covers; signing forbids later edits unless the signature is removed (stated). | S | 3 |
| OF04-F206 | Encryption with a password | ODF 1.3 package encryption written with AES-256-GCM and Argon2id key derivation where the reader supports it, else AES-256-CBC with PBKDF2-SHA-256 for compatibility (the dialog names which and why). Wrong password reveals nothing. | S | 2 |
| OF04-F207 | Encryption to keys | OpenPGP-based ODF encryption to recipients chosen from SE04; the user's own key is always included unless unticked with a warning. | S | 3 |
| OF04-F208 | Reading foreign encryption | Open password-protected DOCX (agile encryption, AES) and legacy ODF (Blowfish, AES-CBC); write DOCX encryption on export (agile, AES-256). | S | 3 |
| OF04-F209 | Secret handling | Passwords go through secret fields to the helper; keys never enter the script engine; plaintext recovery journals for encrypted documents are sealed with a key from `svc.keyring`. | M | 2 |
| OF04-F210 | Safety panel | Per document: provenance, protected view state, macros found (not run), external links (not fetched), embedded objects (inert), signatures, encryption, preserved unknown parts. | M | 2 |

### 4.19 General

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F211 | Command palette | `Ctrl+K` when no text view has focus, `Ctrl+Shift+P` always: commands, styles (apply by name), headings (jump), templates, settings, help. | M | 1 |
| OF04-F212 | Print | `Ctrl+P` through the print portal (G-16): ranges, selection, copies, collate, duplex, pages per sheet, booklet, comments placement, hidden text and placeholders options, tracked-change display. Spool format PDF from the page boxes. | M | 2 |
| OF04-F213 | Share | `share` intent with ODT, PDF or DOCX; clean option (F200). | S | 2 |
| OF04-F214 | Customisable toolbars and shortcuts | Toolbars are data; add, remove, reorder actions; all shortcuts rebindable; notebook-style tabbed toolbar is not planned. | S | 3 |
| OF04-F215 | Inspector | `F4`: context panels for character, paragraph, page, table, image, frame, field, style provenance. | M | 1 |
| OF04-F216 | No network | `net.none`. No online templates, fonts, clip art, grammar service, help or update check. Linked remote content in documents is listed, never fetched. | M | 1 |
| OF04-F217 | Help | In-app help pages and a sample document demonstrating styles and layout. | S | 1 |

## 5. GUI

### 5.1 Window map

```
Main window
├─ Header bar: menu, title, search/palette, signature and lock chips, provenance chip
├─ Tab strip
├─ Toolbars: Standard · Formatting (style box first) · context bars (Table, Image, Review, Header/Footer, Equation)
├─ Rulers (horizontal, vertical)
├─ Sidebar (F9): Navigator | Styles | Find results | Changes | Comments list | Accessibility | Data sources
├─ Document area: page view (print, web, draft, outline, read), comment margin, change bars, banners, find bar
├─ Inspector (F4): Character | Paragraph | Page | Table | Image/Frame | Field | Safety
├─ Status bar
├─ Dialogs: Style (per family) · Character · Paragraph · Page Style · Section · Columns · Table Properties ·
│          Image/Frame Properties · Fields · Cross-reference · Contents and Index · Bibliography entry · Citation ·
│          Footnote options · Heading Numbering · List Style · Special Character · AutoCorrect · Mail Merge wizard ·
│          Compare · Merge · Templates gallery · Export PDF · Export EPUB · Fidelity report · Inspect and Clean ·
│          Sign · Encrypt · Macro manager · Envelope · Labels · Word Count · Go To
├─ Popovers: style quick-apply, colour, table grid, wrap, zoom, language, equation editor, comment bubble
└─ Print preview (full view), Settings tab
Drawn by the shell: file portal, print dialog, share sheet, key-use prompt, capability prompts
```

### 5.2 Main window

Shown at 1400×860, print layout, sidebar on Navigator, inspector on Paragraph.

```
+----------------------------------------------------------------------------------------------+
| [=] Writer — report.odt              [ Search or command  Ctrl+Shift+P ]   [✓sig] [▁][▢][✕]  | (1)
+----------------------------------------------------------------------------------------------+
| report.odt ● | letter.odt | Untitled 1                                                  [+]  | (2)
+----------------------------------------------------------------------------------------------+
| [New v][Open][Save][PDF][Print] [↶][↷] [✂][⧉][📋][🖌] [Table v][Image][Link][Note][¶] [Review]| (3)
| [Heading 2        v] [Serif      v] [14 v] [B][I][U][S] [A v][▮ v] [≡][≡][≡][≡] [•][1.][⇤][⇥] | (4)
+------------+-----L-----1-----2-----3-----4-----5-----6-----7-----8---R-----+-----------------+
| Navigator  |   +--------------------------------------------------------+   | Paragraph       |
| [search  ] |   | Report on capabilities                          3      |   | Style Heading 2 |
| v Headings |   |--------------------------------------------------------|   |  └ Heading      |
|   1 Intro  |   | 2.1 Rights only narrow                                 |   | Align  [≡ left] |
|  >2 Model  |   | A handle carries rights. A copy may hold fewer, never  | 🗨 | Indent 0 / 0    |
|    2.1 Ri… |   | more.¹ See Figure 3 on page 4.                         | AK | Above  12 pt    |
|    2.2 Li… |   |  +-----------+  Text wraps around the anchored image   |   | Below   6 pt    |
|   3 Proof  |   |  |  image    |  and keeps to the baseline grid.        |   | Line   1.15     |
| > Tables 4 |   |  +-----------+  ▍inserted text▍ d̶e̶l̶e̶t̶e̶d̶              |   | [x] Keep w/ next|
| > Images 6 |   |  Figure 3: Lineage tree                                |   | Outline lvl 2   |
| > Comments3|   |--------------------------------------------------------|   | Direct format:  |
|            |   | ¹ INVARIANTS.md, SI-1.                                 |   |  spacing ✕      |
|    (5)     |   +--------------------------------------------------------+(6)|       (7)       |
+------------+----------------------------------------------------------------+-----------------+
| Page 3 of 41 · 12 408 words · Heading 2 · English (AU) · Track: ON · Insert · ▭▭ ▯ ▤ · 100%  | (8)
+----------------------------------------------------------------------------------------------+
```

1. Header bar (`headerBar`): signature chip (valid, changed, none), lock chip when encrypted, provenance chip in protected view. No `[net]` badge exists for this app.
2. Tabs (`tabView`).
3. Standard toolbar (`toolbar`); context toolbars replace the right part when a table, image, header or equation has focus.
4. Formatting bar: paragraph style `comboBox` with previews first, then font, size, direct formatting `toolButton`s.
5. Sidebar (`sidebar`): Navigator `treeView` with drag reorder; other panels are lists.
6. Document area (`richTextEdit` in paged mode over `pageView`; see 5.7): rulers, page boxes, comment margin with bubbles, change marks, anchors.
7. Inspector (`inspector`, `propertyGrid`): shows style inheritance and direct-format overrides with remove buttons.
8. Status bar (`statusBar`): page, words, paragraph style, language, tracking state, insert mode, view buttons, zoom `slider`.

States: empty (template gallery as start view); loading (first page shown, "Laying out pages 120 of 800" in the status bar, page count shows "…"); protected view banner; conversion running (progress sheet with cancel); read-only; error; narrow.

### 5.3 Other views and dialogs

**Paragraph style dialog** (720×560).

```
+--------------------------------------------------------------------------+
| Paragraph Style: Heading 2                                               |
| [Organiser][Indents][Alignment][Text Flow][Font][Effects][Position]      |
| [Tabs][Borders][Area][Outline & List][Drop Caps][Condition]              |
+--------------------------------------------------------------------------+
| Name        [ Heading 2            ]     Preview                         |
| Inherit from[ Heading             v]     +----------------------------+  |
| Next style  [ Body Text           v]     | 2.1 Rights only narrow     |  |
| Category    [ Text styles         v]     | Body text follows here…    |  |
| Contains (differences from parent):      +----------------------------+  |
|   14 pt · bold · space above 12 pt · outline level 2 · keep with next    |
| [ ] Update automatically from direct formatting                          |
|                          [Reset to Parent] [Cancel] [Apply] [OK]         |
+--------------------------------------------------------------------------+
```

`dialog` with `tabBar`; each tab is a form of `lineEdit`, `comboBox`, `spinBox`, `checkBox`; preview is a read-only rich text view.

**Manage changes** (sidebar panel, 320 wide) and review bar.

```
+----------------------------------+
| Changes (27)   [Filter v]        |
| Show [Final with markup    v]    |
+----------------------------------+
| > AK · 11 Nov · Insert           |
|   "and keeps to the baseline"    |
|   [✓ Accept] [✕ Reject]          |
|   MB · 10 Nov · Delete           |
|   "deleted"                      |
|   AK · 10 Nov · Format: bold     |
|   AK · 09 Nov · Move (2 parts)   |
+----------------------------------+
| [Accept All] [Reject All] [Next] |
+----------------------------------+
```

**Fidelity report** (sheet, 680×480).

```
+----------------------------------------------------------------------+
| Saving "report" as Word (.docx)                                      |
| 3 things will change, 2 will be kept but not editable in Word        |
+----------------------------------------------------------------------+
| ! Approximated  Linked text frames (p. 7) → separate text boxes [Go] |
| ! Approximated  Register-true spacing → exact line spacing      [Go] |
| ! Dropped       Conditional style "Body/in table" → plain style [Go] |
| i Kept          Bibliography in CSL data → custom XML part           |
| i Substituted   Font "Serif Pro" → metric-compatible substitute      |
+----------------------------------------------------------------------+
| [x] Also keep an ODT copy beside it     [Save Report…]               |
|                               [Cancel] [Save as ODT instead] [Save]  |
+----------------------------------------------------------------------+
```

**Cross-reference dialog** (640×420): left `listView` of types (Headings, Numbered paragraphs, Figures, Tables, Bookmarks, Footnotes, Endnotes), middle `listView` of targets with a filter, right `listView` of formats (Page, Chapter, Reference text, Above/Below, Number, Category and number, Caption text), [x] insert as hyperlink, [Insert] [Close]; stays open for repeated inserts.

**Contents and index dialog** (760×560): tabs Type, Entries, Styles, Columns, Background; the Entries tab has a structure line editor `[#][E][T………][P][link]` built from `tagInput` tokens, and a live preview pane.

**Mail merge wizard** (720×520): steps 1 Source (file or contacts group, table picker, record count), 2 Fields (drag into the document; address block builder), 3 Filter and sort (`tableView` with ticks), 4 Preview (record stepper `[◀] 12 of 200 [▶]`), 5 Output (one document, files by pattern, print, PDF, mail drafts) with a summary.

**Inspect and clean** (sheet, 640×520).

```
+------------------------------------------------------------------+
| What this document reveals                                       |
| [x] Author names in 14 comments and 27 changes     [View]        |
| [x] 27 tracked changes (will be accepted)          [View]        |
| [x] 2 hidden paragraphs                            [View]        |
| [x] Properties: title, 3 custom fields             [View]        |
| [x] Metadata inside 6 images (camera, GPS in 1)    [View]        |
| [x] Cropped-away parts of 2 images                 [View]        |
| [ ] 1 unknown preserved part from the DOCX source  [View]        |
| [x] 1 macro (not signed, never run)                [View]        |
| i Links: 3 external targets (kept; not fetched)                  |
|                         [Cancel] [Clean a Copy…] [Clean This]    |
+------------------------------------------------------------------+
```

**Export PDF dialog** (640×520): General (range, PDF version, PDF/A, tagged, bookmarks, comments), Images (lossless, JPEG quality, downsample DPI), Fonts (subset, embed all), Initial view, Security (password, AES-256), Clean (personal data, hidden content), accessibility check summary, [Export].

**Equation popover** (inline, 520 wide): source `lineEdit` in monospace, live preview above it, error line with position, symbol palette `gridView`, [Display] [Inline] toggle, [Done].

### 5.4 Menus

```
File    New Ctrl+N · New from Template Ctrl+Shift+N · Open… Ctrl+O · Recent ▸ · Close Ctrl+W
        Save Ctrl+S · Save As… Ctrl+Shift+S · Save a Copy… · Save Version… · Versions… · Revert
        Export ▸ (PDF…, EPUB…, DOCX…, HTML…, Markdown…, Text…) · Share… · Print Preview · Print… Ctrl+P
        Properties… · Inspect and Clean… · Sign… · Encrypt… · Templates ▸ · Mail Merge… · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut · Copy · Paste Ctrl+V · Paste Special Ctrl+Shift+V · Select All Ctrl+A
        Find Ctrl+F · Replace Ctrl+H · Go To Ctrl+G · Track Changes ▸ (Record Ctrl+Shift+C, Accept Alt+], Reject Alt+[,
        Next, Previous, Manage) · Comment Ctrl+Alt+C · Compare… · Merge… · Fields… · Links… · Settings Ctrl+,
View    Print Layout · Web Layout · Draft · Outline · Read Mode · Focus Mode · Full Screen F11
        Sidebar F9 · Navigator F5 · Styles Ctrl+Alt+S · Inspector F4 · Rulers · Text Boundaries
        Formatting Marks Ctrl+F10 · Field Shading Ctrl+F8 · Field Names Ctrl+F9 · Comments · Changes Display ▸
        Zoom ▸ · Split Window
Insert  Page Break Ctrl+Enter · Break… · Image… · Table… Ctrl+F12 · Text Frame · Shape ▸ · Drawing (Draw) · Chart (Sheets)
        Equation Alt+= · Special Character… · Hyperlink Ctrl+Alt+K · Bookmark… · Cross-reference… · Footnote Ctrl+Alt+F
        Endnote Ctrl+Alt+D · Caption… · Field ▸ · Header ▸ · Footer ▸ · Page Number · Contents and Index ▸
        Citation Ctrl+Alt+B · Bibliography · Section… · Envelope… · Labels… · Document… · Watermark…
Format  Text ▸ (Bold Ctrl+B, Italic Ctrl+I, Underline Ctrl+U, Superscript Ctrl+Shift+=, Subscript Ctrl+=, Case ▸) · Character… · Paragraph…
        Lists ▸ (Bullets Shift+F12, Numbering F12, Promote, Demote, Restart) · Heading Numbering… · Columns… · Page Style…
        Align ▸ (Left Ctrl+L, Centre Ctrl+E, Right Ctrl+R, Justify Ctrl+J) · Line Spacing ▸ · Clear Direct Formatting Ctrl+M
        Clone Formatting · Image ▸ · Frame and Object ▸ · Wrap ▸ · Anchor ▸ · Arrange ▸ · Theme…
Styles  Body Ctrl+Alt+0 · Heading 1–9 Ctrl+Alt+1…9 · New from Selection · Update Selected Style · Edit Style… · Manage… · Load…
Table   Insert ▸ · Delete ▸ · Select ▸ · Merge Cells · Split Cells · Split Table · Size ▸ · Heading Rows Repeat
        Convert ▸ · Sort… · Formula F2 · Number Format… · Table Style ▸ · Properties…
Tools   Spelling F7 · Grammar · Thesaurus Ctrl+F7 · Language ▸ · Hyphenation… · Word Count… · AutoCorrect… · AutoText…
        Update ▸ (All F9, Fields, Contents and Indexes, Links) · Accessibility Check… · Bibliography Library…
        Line Numbering… · Footnote Settings… · Macros ▸ (disabled until enabled) · Document Safety · Customise…
Help    Writer Help F1 · Keyboard Reference · Format Fidelity Notes · About
```

`F11` stays full screen, so the Styles panel is `Ctrl+Alt+S`. `F2` edits a formula only inside a table cell and renames elsewhere, as reserved. Print preview has no default key.

### 5.5 Keyboard and pointer

- Movement: `Ctrl+Left/Right` word, `Ctrl+Up/Down` paragraph, `Home/End` line, `Ctrl+Home/End` document, `PgUp/PgDn` screen, `Ctrl+PgUp/PgDn` header and footer, `Alt+PgUp/PgDn` previous and next page; in tables `Tab`, `Shift+Tab`, `Alt+arrows` resize, `Alt+Insert` then arrow inserts.
- Structure: `Tab` and `Shift+Tab` at the start of a heading or list item change level; `Ctrl+Alt+Up/Down` move paragraphs; `Ctrl+Enter` page break; `Shift+Enter` line break; `Ctrl+Shift+Enter` column break.
- Context menu, text: Cut, Copy, Paste, Paste Special, spelling and grammar suggestions, Character, Paragraph, Style ▸, List ▸, Insert Comment, Hyperlink, Synonyms ▸, Accept or Reject Change.
- Context menu, table: Insert ▸, Delete ▸, Merge, Split, Size ▸, Properties, Caption, Number Format.
- Context menu, image or frame: Crop, Wrap ▸, Anchor ▸, Arrange ▸, Align ▸, Alt Text, Caption, Replace, Compress, Edit in ▸ (Image Editor, Draw), Save Image, Properties.
- Context menu, field: Update, Edit Field, Toggle Field Name, Convert to Text. Navigator item: Go To, Promote, Demote, Move Up, Move Down, Delete, Rename.
- Pointer: drag margins and tab stops on rulers; double click on a ruler opens Paragraph; double click in the header or footer area enters it; drag table borders; drag handles of frames with snap lines; `Ctrl+wheel` zoom; `Ctrl+click` follows a link; drag the anchor icon to re-anchor.
- Drag and drop: files (images insert, documents open or, with `Ctrl`, insert as text; data files start mail merge); text between windows; headings in the Navigator; styles from the Styles panel onto paragraphs; table ranges from OF05; drawings from OF08.
- Pen and touch: pinch zoom, two-finger scroll, long press for the context menu; ink annotations are comments of type "ink" stored as drawing shapes.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start view: recent documents and the template gallery; [Blank Document]. |
| Loading | Converter progress, then first page at once; background pagination with a live page count; editing allowed immediately. |
| Protected view | Banner "This file came from <origin>. Editing, fields, links and macros are off." [Enable Editing]. |
| Error | Conversion failure: message with the helper's reason and [Open as Text] [Report Details]. Save failure: banner, journal keeps everything. |
| Offline | Normal; the app has no network use. |
| Locked | Encrypted document awaiting password or key: nothing shown but the prompt. |
| Read-only | Lock chip; typing shows a toast with [Edit a Copy]. Signed documents are read-only until the signature is removed. |
| Recovered | After a crash: banner "Recovered to 14:02:31" with [Compare with Saved]. |
| Missing fonts | Banner listing substitutions with [Details]. |
| Narrow (< 640) | Web layout is forced; sidebar and inspector become sheets; toolbars collapse to one row with overflow. |

### 5.7 Widgets

| Region | Widget | Exists today (V3) | Needs |
|---|---|---|---|
| Document area | `richTextEdit` in paged mode | no (G-10) | FUSE-WP-35 and FUSE-WP-36 meeting section 6.5 |
| Page presentation, print preview | `pageView` | no | FUSE-WP-45 fed by `PageBox`es |
| Rulers | `ruler` | no | `OF04-GAP-1` (ruler widget with tab stops, indents, margin and column handles) |
| Comment margin, change bars | decoration and side-note lane of the rich text widget | no | section 6.5 R-E14 |
| Equations | maths box layout | no | `OF04-GAP-2` |
| Shapes in place | canvas overlay | no (G-12) | FUSE-WP-43, FUSE-WP-23 |
| Navigator, styles, changes | `treeView`, `listView` | limited | FUSE-WP-28 |
| Mail-merge records | `tableView` | single select, no ticks | FUSE-WP-28 |
| Macro editor | `codeView` | `codeEdit` only | FUSE-WP-37 |
| Toolbars, header, status, sheets, popovers, palette | chrome kit | no | FUSE-WP-29, FUSE-WP-30 |
| Dialog forms | `lineEdit`, `comboBox`, `spinBox`, `checkBox`, `radioButton`, `colorPicker`, `fontPicker`, `tabBar`, `propertyGrid` | yes | FUSE-WP-32 for product-quality pickers |
| Compare | `diffView` is text only | no | Compare is built on tracked changes in the rich text engine, not on `diffView` |

`OF04-GAP-3`: a spelling, grammar, hyphenation and thesaurus service interface (G-17 names the service; no interface exists). `OF04-GAP-4`: digest signing through SE04 with a shell prompt (same as OF03-GAP-2).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Commands, style management, fields and reference resolution, contents and index generation, CSL processing control, review logic, mail merge, checker, UI glue |
| Views and styles | FUSE UI | app host | Windows, dialogs, panels |
| Rich text engine and page layout | C++23 (FUSE-WP-35, FUSE-WP-36) | app host | Document model, operations, layout, painting; platform code |
| `convert-helper` | C++23 | helper per conversion; capability mode; input descriptor and output pipe only | Readers and writers for ODF, DOCX, DOC, RTF, HTML, Markdown, EPUB; zip and XML parsing; package encryption and decryption with a passed key or password; signature byte preparation |
| `export-helper` (same binary, write role) | C++23 | helper; write-only descriptor | PDF writer (tagging, fonts, images), EPUB writer |
| `math-helper` | C++23 (links MathScript) | helper, no capabilities | LaTeX-subset parsing and MathML export |
| CSL processor | C++23 (own, Proposed) | in `convert-helper` binary, separate role | Citation and bibliography formatting from CSL styles |
| Macro engine | ForgeScript, second engine instance, empty capabilities | app host | Runs trusted macros against `writer::` |
| Recovery journal | ForgeScript + engine operation codec | app host | Append-only operation log per document |

```
 SH04 portals ─handles─▶ app host ───────────────────────────────────────────────────────────┐
                          │ ForgeScript logic ── macro engine (no caps)                      │
                          │ RichDoc + operations ─▶ page layout ─▶ PageBoxes ─▶ screen/print │
                          │        ▲ engine records (schema-checked)      │                  │
                          ├────────┴── convert-helper (ODF/DOCX/RTF/HTML…; no net, no fs)    │
                          ├── export-helper ◀── PageBoxes + structure tree ──▶ PDF / EPUB    │
                          ├── math-helper (MathScript: LaTeX subset → MathML)                │
                          ├── SE04 (sign digest, unwrap key) · spelling service (G-17)       │
                          └── svc.settings / keyring / audit / intent / print / backup ──────┘
```

### 6.2 Data model

The document in memory is the engine's `RichDoc` (FUSE-WP-35) extended as section 6.5 requires. Writer-level entities:

| Entity | Fields |
|---|---|
| `Story` | `id: u32`, `kind: u8` (body, header, footer, footnote, endnote, frame, cell, comment), `text: piece tree`, `char_attrs: interval tree`, `para_ids: vector<u64>` |
| `Paragraph` | `id: u64`, `style: StyleId`, `direct: ParaAttrs` (sparse), `list: ListRef?` (`list_id`, `level: u8`, `restart: bool`, `start: u32?`), `outline_level: u8`, `break_before: BreakSpec?` (`kind`, `page_style`, `page_number`) |
| `CharRun` (interval) | `style: StyleId?`, `direct: CharAttrs` (sparse: font ids for three script groups, `size_hpt: u16`, weight, slant, underline, strike, colour, highlight, `lang`, features, spacing, position, hidden) |
| `Style` | `id`, `family: u8` (para, char, page, list, table, frame), `name`, `parent: StyleId?`, `next: StyleId?`, `attrs` (sparse), `auto: bool`, `hidden: bool` |
| `PageStyle` | `size`, `margins`, `orientation`, `columns`, `header: HFSpec`, `footer: HFSpec`, `first_variant`, `mirrored`, `numbering`, `next`, `footnote_area`, `background`, `grid` |
| `Section` | `id`, `name`, `range: AnchorPair`, `columns`, `protected: bool`, `condition: Expr?`, `link: LinkRef?`, `notes_at_end: bool` |
| `Table` | `id`, `style`, `width_spec`, `columns: vector<ColSpec>`, `rows: vector<Row>` (`height_spec`, `header: bool`, `can_split`), `cells` (`row_span`, `col_span`, `story`, `attrs`, `formula?`, `value?`) |
| `Anchor` | `id: u64`, `story`, `offset` (stable across edits), `kind` (bookmark, reference mark, comment start/end, change start/end, index mark, object) |
| `Object` | `id`, `kind` (image, frame, shape, formula, embedded), `anchor`, `anchor_type`, `wrap: WrapSpec`, `pos: PosSpec`, `size: SizeSpec`, `z: i32`, `alt_title`, `alt_desc`, `decorative: bool`, `payload: BlobRef`, `caption: StoryRef?` |
| `Field` | `id`, `kind: u16`, `anchor`, `params: json`, `cached_text`, `fixed: bool`, `dirty: bool` |
| `Note` | `id`, `kind` (foot, end), `anchor`, `mark: text?`, `story` |
| `Change` | `id`, `kind` (insert, delete, move-from, move-to, char-attr, para-attr, table-row, table-col), `range: AnchorPair`, `author_id`, `time?`, `old_attrs?`, `comment?`, `pair_id?` |
| `Comment` | `id`, `range`, `author_id`, `time?`, `story`, `parent?`, `resolved: bool` |
| `BibSource` | CSL-JSON item with `id` |
| `Blob` | `hash: blake3`, `mime`, `bytes` or linked `grant_token` |
| `OpaquePart` | `origin_format`, `path`, `bytes`, `preview: BlobRef?`, `anchor?` |

On disk: ODT package (`mimetype`, `content.xml`, `styles.xml`, `meta.xml`, `settings.xml`, `META-INF/manifest.xml`, `Pictures/`, `Object n/`, optional `META-INF/documentsignatures.xml`, optional `Versions/`). Writer-specific data that ODF has no element for (CSL items, accessibility ignore reasons, opaque parts) goes into namespaced extension elements and extra package parts, declared in the manifest. App store (`~/.local/share/org.pbsd.writer/`): `recovery/<doc-id>.journal` (operation records with chained BLAKE3), `history/` versions, `library.sqlite` (bibliography library, AutoText, recent, author identity, font map), `templates/`, `macros/`. Schema versions in each store; forward-only migration with a copy kept.

### 6.3 File formats and protocols

| Format | Read | Write | Library or own code | Parser runs in | Fidelity target and plan |
|---|---|---|---|---|---|
| ODT, OTT, FODT, ODM (ODF 1.3) | yes | yes | own C++23 (zip, XML pull parser, ODF mapper) | `convert-helper` | A. Test: ODF conformance suite documents; open-save-open must give an equal model; unknown content preserved |
| DOCX, DOTX (OOXML) | yes | yes | own C++23 | `convert-helper` | B. Test: a corpus rendered to PDF by a reference word processor; compare page count, line breaks per paragraph (target ≥ 95% of paragraphs with identical breaks when fonts match) and object positions (≤ 1 mm); each known gap listed in "Format Fidelity Notes" |
| DOC | yes | no | own C++23 (compound file and piece table reader) | `convert-helper` | C. Text, styles, tables, images, notes |
| RTF | yes | yes | own C++23 | `convert-helper` | C |
| HTML5 | yes | yes | own tokeniser and allow-list mapper (shared design with OF02) | `convert-helper` | D in, C out |
| Markdown | yes | yes | FUSE-WP-42 parser on the read side, own writer | `convert-helper` (foreign files) | D |
| EPUB 3.3 | no | yes | own C++23 | `convert-helper` / `export-helper` | Validated against the EPUB specification's checker rules re-implemented as tests |
| PDF 1.7 / 2.0, PDF/A-2b/3b, PDF/UA-1 | via OF03 | yes | own C++23 writer | `export-helper` | Output equals screen page boxes; tagged structure checked by tests modelled on the PDF/UA failure conditions |
| MathML, OMML | yes | yes | own mapper; MathScript for LaTeX subset | `math-helper`, `convert-helper` | Presentation MathML core |
| CSL 1.0.2 styles, CSL-JSON, BibTeX, RIS, EndNote XML | yes | CSL-JSON, BibTeX, RIS | own C++23 | `convert-helper` (CSL role) | CSL test-suite fixtures as the reference |
| CSV, ODS, XLSX, vCard, SQLite (mail-merge sources) | yes | no | OF05 converter interface; own vCard; SQLite (public domain) read-only | helper | — |
| XML-DSig / XAdES, OpenPGP messages | yes | yes | own C++23 canonicaliser (exclusive C14N) and packet codec; primitives from FORGE-WP-22 | `convert-helper`; private-key operations in SE04 | — |
| ODF and OOXML package encryption | yes | yes | own code on FORGE-WP-22 primitives | `convert-helper` | — |

No third-party office-format library is planned: such libraries are large, not memory-safe, and pull in their own layout assumptions. The cost is that the DOCX mapper is the largest work item of this app.

### 6.4 Integration

- Intents handled: `open`, `edit`, `view`, `new` (text document, from template), `print`, `compose` (new document with given text). Emitted: `open` (links after confirmation, hand-offs), `share`, `print`, `scan`, `edit` to OF08 (drawings), OF05 (charts and ranges), CR01 (images), AI04 (equations).
- MIME: see `manifest.json`; default handler for ODT, DOCX, DOC, RTF.
- Services: `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.notify` (long conversions and merges), `svc.keyring`, `svc.thumbnail` (provider for its document types, through the helper), `svc.search` (optional), `svc.backup`, `svc.audit`, `svc.a11y`, `svc.model` (optional), spelling and language service (G-17).
- Interfaces offered: `org.pbsd.writer.Convert(handle, target_mime, options) -> handle` (headless conversion for OF02, CO01, SY02 previews); thumbnail and quick-preview provider.
- Other apps: OF03 (PDF import service, viewing exported PDFs), OF05 (data sources, embedded ranges and charts), OF08 (drawings), OF06 (shares the engine and the converter core), CO01 (mail-merge drafts, HTML mail composition uses the same rich text engine), CO03 (contacts for merge), SE04 (keys, trust), UT03 (fonts), AX01 (structure for speech), OF02 (document import and ODT export of notes).
- Verified in repos: FUSE has no rich text widget, page layout or ruler (`FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp`, whole file); today's text layout has one font size, one colour and three alignments (FUSE_BUILD_PLAN.md FUSE-WP-18, citing `text.hpp:176-191`); FULL text profile exists but fetches sources at configure time (PLATFORM.md V6). MathScript reads a strict LaTeX subset and exports MathML (paths in 4.12). ForgeScript signing and capability narrowing exist (`FORGE:docs/scripting.md`). Everything else is Proposed.

### 6.5 What the rich-text and layout engine must do (gap G-10)

These are requirements on FUSE-WP-35 (editing engine, "R-E") and FUSE-WP-36 (page layout, "R-L"). FUSE_BUILD_PLAN.md already states the shape; this table fixes what Writer needs. Items marked ＋ are not stated in the build plan as read and must be added there.

| ID | Requirement | Detail |
|---|---|---|
| R-E01 | Stories | Any number of independent text flows (body, each header and footer, each note, frame, cell, comment) sharing one style table and one undo log. |
| R-E02 | Character attributes | Sparse attribute sets on ranges, resolved as direct → character style → paragraph style → parents → defaults, with the source of each value queryable (for the inspector). Three font slots per run by script group. ＋ |
| R-E03 | Paragraph attributes | The set in 4.3 including tab stops, borders that merge between neighbours, drop caps, outline level, list reference, flow controls, hyphenation limits, baseline-grid snap. |
| R-E04 | Style families | Paragraph, character, list, table, frame and page styles with inheritance and "next style"; changing a style relayouts only users of it. |
| R-E05 | Lists | Multi-level list styles, per-paragraph level and restart, numbering computed incrementally across stories and sections; number text exposed to fields and references. |
| R-E06 | Tables | Rows and columns with spans, nested tables, header rows, per-edge borders with conflict resolution, width modes (fixed, relative, auto), cell-range selection, row and column operations as invertible operations. |
| R-E07 | Anchors | Stable positions and ranges that survive edits, undo and copy; typed; used for bookmarks, comments, changes, index marks, objects. Deleting a range reports the anchors inside so the app can decide (keep, collapse, delete). |
| R-E08 | Inline and anchored objects | Objects as characters or anchored to page, paragraph or character, with wrap, position and size specs; hit testing, handles, snap lines; object content supplied by callbacks (image, formula box, embedded preview). |
| R-E09 | Fields | Inline placeholders whose text comes from the app, are atomic for caret movement, shadable, and re-queried on layout when they depend on layout (page number, page count, "above/below", chapter) with a bounded number of passes. |
| R-E10 | Notes | Footnote and endnote anchors bound to note stories; numbering by page, chapter or document. |
| R-E11 | Operations | A closed set of invertible operations with transactions; every user command is one transaction; operations are serialisable (recovery journal) and replayable deterministically. |
| R-E12 | Change tracking ＋ | A mode in which operations produce change records instead of destructive edits: deleted text stays in the story flagged as deleted; inserted text is flagged; attribute changes keep old values; moves are paired. Layout can show any of four views (4.10) without changing the model. Accept and reject are operations. |
| R-E13 | Selection | Multiple ranges, cell rectangles, object selection, caret affinity, visual and logical bidi movement, selection across stories forbidden except whole-document select. |
| R-E14 | Decorations and side notes | Squiggles (spelling, grammar), search hits, change marks, field shading, formatting marks, comment highlights, and a margin lane where the app places comment and change balloons tied to anchors with connector lines. ＋ for the margin lane. |
| R-E15 | Input | IME pre-edit inline, autocorrect hook per committed word, input rules, overwrite mode, drag and drop of ranges. |
| R-E16 | Clipboard fragments | Copy produces a self-contained fragment (text, attributes, used styles, objects) and paste merges styles by a caller-chosen policy. |
| R-E17 | Search | Iterator over text across stories with attribute and style predicates; regex through the host provider. |
| R-E18 | Scale | 2 million words, 100 000 paragraphs, 10 000 objects; edit cost independent of document size; memory ≤ 3× the text size plus objects. |
| R-E19 | Accessibility | Text ranges with attributes, headings, lists, tables with headers, links, notes, comments and changes exposed to the a11y tree. |
| R-E20 | Record interface ＋ | A stable, versioned record stream to build and read a `RichDoc` from a helper process (the "engine records" of 6.1), validated on entry: the app host never trusts converter output. |
| R-L01 | Page model | Page styles with size, margins, mirrored layout, first/left/right variants, header and footer areas that grow, footnote area, columns, background, borders; page sequence driven by "next style" and by breaks carrying a style and number. |
| R-L02 | Sections and columns | Section-level column sets inside a page, balancing, separator lines, column breaks; nested limits stated (no columns inside columns except in frames). |
| R-L03 | Pagination rules | Widows, orphans, keep with next, keep together, break before and after, minimum lines beside drop caps, heading never last on page; deterministic tie-breaks. |
| R-L04 | Tables across pages | Row splitting on or off, repeated header rows, cell content split with borders redrawn, vertical merges across a break, keep-with-next for the table. |
| R-L05 | Footnotes | Space reserved per page, split with continuation notice, separator, per-column notes in multi-column sections, endnote pages; a footnote reference and the start of its note stay on the same page where possible. |
| R-L06 | Anchored objects and wrap | Wrap modes of 4.8 including contour polygons; objects may push their anchor paragraph; a bounded iteration (stated maximum 8 passes) resolves object and text positions, then freezes. |
| R-L07 | Line breaking | Two algorithms: greedy (compatibility) and total-fit per paragraph (Knuth–Plass style with hyphen penalties, looseness, widow avoidance), selectable per document; UAX 14 opportunities; hyphenation provider; justification by word space, then letter space and glyph scaling within limits; kashida and inter-ideograph expansion by script. ＋ for total-fit. |
| R-L08 | Baseline grid | Optional register-true placement per paragraph style. ＋ |
| R-L09 | Vertical and mixed text | Vertical page and frame writing modes, rotated runs, ruby and emphasis marks (Phase 3). ＋ |
| R-L10 | Layout-dependent fields | Two-pass or iterative resolution of page count and references with a fixed-point bound; if not converged in 4 passes, the last values stand and a warning is recorded. |
| R-L11 | Incremental layout | After an edit, relayout from the changed paragraph until a page boundary and line set match the previous layout; background completion; page count estimate until done. |
| R-L12 | Determinism | Same document, fonts and settings give the same page boxes on any machine and thread count; layout uses fixed-point units (1/64 pt or finer) for positions. |
| R-L13 | Output | `PageBox` draw lists for screen and print, plus a parallel structure tree (element type, reading order, alt text, language, artefact flags, link and note relations) from which the PDF writer produces tags. ＋ for the structure tree. |
| R-L14 | Views without pages | Web layout (wrap to width), draft and outline views use the same line breaker without page rules. |
| R-L15 | Compatibility switches | Per-document flags that change specific rules to match foreign documents (space-before at page top, tab behaviour at margins, table border model, line-height model, footnote placement). Each flag is named and tested. ＋ |
| R-L16 | Maths boxes | Inline boxes with baseline, axis height and italic correction from the maths layout (OF04-GAP-2). |

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Recovery journals, history, bibliography library, AutoText, user templates and macros |
| `fs.portal` | Open, save, export, linked images and sub-documents, mail-merge sources, template folders |
| `net.none` | Writer never opens a socket; remote links and images are not fetched |
| `proc.helper:convert-helper`, `proc.helper:math-helper` | Section 6.1 (the export and CSL roles are the same binary as `convert-helper`) |
| `dev.print` | Printing |
| `ai.local`, `svc.model` | Optional writing aids and model-based grammar; off by default |
| `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.notify`, `svc.keyring`, `svc.thumbnail`, `svc.search`, `svc.backup`, `svc.audit`, `svc.a11y`, `svc.spell` | Section 6.4 |

Not requested: `net.host`, `net.user`, `proc.exec`, any device but print, `bg.*`, `fs.path`.

Assets: document content; hidden content the author does not know is there; author identity; signing keys and passwords; the user's trust decisions about macros.

| Threat | Mitigation |
|---|---|
| Malicious document exploits a parser (zip, XML, OOXML, DOC, RTF, fonts, images) | All parsing in `convert-helper` with no capabilities; own memory-safe C++23 code under the subset check; XML entity expansion off; zip bombs bounded (ratio, total size, entry count); helper output validated record by record (R-E20) |
| Macro malware | No foreign macro language is implemented; ForgeScript macros are off by default, signed, sandboxed with empty capabilities, budgeted, audited (4.17) |
| Auto-executing content (fields that run programs, DDE, OLE activation, external data) | No such field types exist; DDE and "include from URL" fields are imported as inert text with a notice; OLE is opaque |
| Remote-content tracking and credential leaks through linked images, templates or frames | Nothing is fetched: links to remote templates and images are listed in the safety panel and stay unresolved |
| Hidden data disclosure (tracked changes, comments, metadata, cropped images, old save remnants) | Minimal metadata, full rewrite on save, Inspect and Clean, clean on share and export (4.18) |
| Spoofed content in protected documents or fake prompts inside a document | Protected-view and signature state shown only in chrome; documents cannot draw outside page boxes |
| Signature wrapping and partial-coverage tricks in XML signatures | Verification checks that references cover every package part listed in the manifest; no ID-based indirection outside the signed set; canonicalisation implemented strictly; uncovered parts make the verdict "partial" |
| Weak encryption on export | Only AES-256 modes written; legacy algorithms read-only; the dialog names the mode |
| Font-based attacks | Document-embedded fonts are parsed only by the sandboxed font path decided in FUSE-WP-17/65; until then embedded fonts are ignored and substituted |

Stored: recovery journals (sealed for encrypted documents), local history, libraries, settings. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `identity.name`, `identity.initials` | string | "" | Used for comments and changes only if set |
| `metadata.store_author` | bool | false | F197 |
| `metadata.strip_image_metadata` | bool | true | On insert |
| `save.default_format` | enum odt, keep_source | odt | Foreign files keep their format on `Ctrl+S` only after the fidelity report is accepted once per document |
| `save.embed_fonts` | bool | false | |
| `save.journal_interval_ms` | int | 1000 | |
| `open.protected_view_for_untrusted` | bool | true | |
| `open.update_fields_on_open` | enum never, trusted, always | trusted | |
| `view.default` | enum print, web, draft | print | |
| `view.units` | enum mm, cm, in, pt, pc | from locale | |
| `view.change_display` | enum inline, balloons | inline | |
| `layout.line_breaker` | enum total_fit, greedy | total_fit | New documents; imported DOCX default to greedy |
| `language.default` | locale | system | |
| `spell.as_you_type` | bool | true | |
| `grammar.enabled` | bool | true | Rule-based |
| `autocorrect.enabled` | bool | true | Each rule also has its own key |
| `fonts.substitution_table` | map | shipped table | |
| `export.pdf.tagged` | bool | true | |
| `export.pdf.version` | enum 1.7, 2.0, A-2b, A-3b | 1.7 | |
| `export.clean_personal_data` | bool | true | |
| `macros.enabled` | bool | false | |
| `macros.trusted_keys` | list of key ids | [] | From SE04 |
| `encryption.kdf` | enum argon2id, pbkdf2 | argon2id | |
| `ai.enabled` | bool | false | |

## 9. Accessibility and languages

- The document exposes headings, paragraphs, lists, tables (with header relations), links, images with alt text, notes, comments and tracked changes to the a11y tree (R-E19). Changes are announced with type and author.
- Every dialog is keyboard complete; ruler actions have dialog equivalents (Paragraph, Page Style, Table Properties).
- The accessibility checker (4.15) and tagged PDF and EPUB export make output accessible, not only the app.
- Right-to-left and mixed text: paragraph direction, mirrored indents and tables, bidi caret; page layouts mirror for right-to-left documents (binding on the right). Needs G-03.
- Complex scripts (Indic, Arabic, Thai, CJK) need shaping, line breaking and justification by script (R-L07); vertical text is Phase 3.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame (start view) | ≤ 300 ms |
| Open a 20-page ODT to first page | ≤ 400 ms including helper start |
| Open a 1 000-page ODT: first page / full pagination | ≤ 1 s / ≤ 5 s in the background |
| Open a 300-page DOCX with 200 images | first page ≤ 2 s |
| Key to paint, 500-page document | ≤ 8 ms (FUSE-WP-36) |
| Style change affecting all body text, 500 pages | first screen ≤ 50 ms; rest in the background ≤ 3 s |
| Scroll frame | ≤ 4 ms |
| Save 500 pages as ODT | ≤ 1.5 s; UI not blocked |
| Export 500 pages to tagged PDF | ≤ 6 s |
| Update contents, index and all fields, 500 pages | ≤ 1 s |
| Memory, 500-page text document | ≤ 250 MiB; images decoded on demand with a 256 MiB cache |
| Journal write | ≤ 1 `fsync` per second; ≤ 2% CPU while typing |
| Idle | zero frames, zero wake-ups (caret blink stops after 30 s) |

## 11. Testing and proof

Nothing has run. All rows are plans.

- Unit (ForgeScript): style resolution display, field evaluation, reference formats, contents and index builders, mail-merge expressions, checker rules, cleaning actions.
- Engine gates (in FUSE): those named in FUSE-WP-35 and FUSE-WP-36 plus gates for each ＋ requirement of 6.5 (change-tracking invariants, total-fit optimality on small paragraphs against a brute-force search, structure-tree completeness, determinism across thread counts).
- Golden renders: a layout corpus (one document per rule in R-L01–R-L16) compared page by page; every dialog; every state.
- Round-trip tests: ODF open-save-open equality on the conformance documents; DOCX and RTF round trips with the fidelity report compared to an expected list.
- Differential layout: DOCX corpus against PDFs produced by a reference word processor (metrics in 6.3); ODF corpus against a second ODF implementation.
- CSL: the public CSL test fixtures. EPUB and PDF/UA: checks re-implemented from the published failure conditions; PDF/A against the same.
- Fuzz: zip reader, XML parser, each format reader, the engine-record validator, RTF tokeniser, DOC compound-file reader, CSL and BibTeX parsers, signature and encryption readers.
- Scripted UI through the a11y tree: tasks T1–T15.
- Security tests: a corpus of documents with macros, DDE fields, remote templates, external images, OLE objects, XML bombs: nothing runs, nothing is fetched (helper has no socket), each is listed in the safety panel. Cleaning tests: after Clean, a scanner finds no author strings, change records, hidden text or image metadata in the saved package.
- Crash tests: kill at random during typing, saving and conversion; recovery loses at most 1 s of input; the saved file is old or new, never mixed.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Engine operations (FUSE-WP-35) | `inverse(op)` restores the pre-state; anchors stay in the document | T1 | PRISM | — |
| Change-tracking mode | Accept-all equals the edited document; reject-all equals the original | T1 (bounded) | PRISM on the operation algebra | Unbounded document sizes |
| Page loop (FUSE-WP-36) | Termination; progress on each iteration; bounded passes for objects and fields | T2 | PRISM | Full layout correctness has no formal spec |
| Total-fit line breaker | Returns a minimum-demerit break set for the cost model | T1 (bounded lengths) | PRISM plus brute-force comparison | — |
| Engine-record validator | Rejects any record stream that would break model invariants | T2 | PRISM plus fuzz | — |
| Zip and XML readers | No runtime errors; bounded resource use | T2 | PRISM plus fuzz | — |
| ODF, OOXML, RTF, DOC mappers | No runtime errors | T2 target, T3 at first | PRISM, fuzz, corpus | Size of the mappers |
| XML canonicalisation and signature coverage | Every manifest part is covered or the verdict is "partial" | T2 | PRISM on the coverage check | Cryptographic primitives proved elsewhere (FORGE-WP-22) |
| PDF writer | Well-formed output; xref offsets correct | T2 | PRISM on offset arithmetic; validator tests | — |
| Macro sandbox | Macro reaches only `writer::` | T3 | Negative tests per capability | ForgeScript VM not proved |
| App logic | — | T4 | Tests | Interpreted |

## 12. Build plan

Phase 1, usable core (write, style, save ODT, print-quality pages for simple documents):

| ID | Work item | Depends on |
|---|---|---|
| OF04-W01 | Bundle, manifest, actions, window, toolbars, sidebar, inspector, status | G-02, G-07, FUSE-WP-29, FUSE-WP-30, FUSE-WP-31 |
| OF04-W02 | Review the FUSE-WP-35/36 headers against section 6.5; file the ＋ items | FUSE-WP-18 header review |
| OF04-W03 | Document host: stories, styles (paragraph, character, page, list), editing commands of 4.2–4.5 | G-10 (FUSE-WP-35), G-03 |
| OF04-W04 | Page view: page styles, headers and footers, breaks, page numbers, rulers, views, zoom | FUSE-WP-36, FUSE-WP-45, OF04-GAP-1 |
| OF04-W05 | `convert-helper` core: zip, XML, engine records, validator | FORGE-WP-27, FORGE-WP-28, G-06, G-08 |
| OF04-W06 | ODF reader and writer (text, styles, lists, pages, simple tables, images); deterministic full-rewrite save | W05 |
| OF04-W07 | Tables (basic), images inline and anchored without wrap, alt text | W03 |
| OF04-W08 | Recovery journal, session restore, protected view, minimal metadata | W03, SY02 provenance |
| OF04-W09 | Find and replace, navigator, fields (document set), statistics, templates | W03 |
| OF04-W10 | Plain text import and export; help; sample document | W05 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| OF04-W11 | Sections, columns, footnotes and endnotes, wrap modes, frames, table page breaking | FUSE-WP-36 |
| OF04-W12 | Cross-references, contents, lists of figures, captions, bookmarks, heading numbering | W09, W11 |
| OF04-W13 | Track changes and comments in the engine and UI | 6.5 R-E12, R-E14 |
| OF04-W14 | DOCX import and export; fidelity report; opaque parts; font substitution | W05, W11, W13 |
| OF04-W15 | PDF export with tagging; print | G-16, FUSE-WP-61, 6.5 R-L13 |
| OF04-W16 | Spelling, hyphenation, total-fit line breaking, OpenType features | G-17, OF04-GAP-3, FUSE-WP-18.6 |
| OF04-W17 | RTF, HTML, Markdown readers and writers | W05 |
| OF04-W18 | Equations: `math-helper`, maths layout, OMML mapping | OF04-GAP-2, MathScript |
| OF04-W19 | Inspect and Clean; safety panel; password encryption | FORGE-WP-22, `svc.keyring` |
| OF04-W20 | Accessibility checker; table and image accessibility properties | W15 |
| OF04-W21 | Shapes in place and hand-offs to OF08, OF05, CR01; versions and local history | G-12, OF08, OF05 |
| OF04-W22 | Convert service, thumbnail provider, command-line conversion | W14, `svc.thumbnail` |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF04-W23 | Bibliography library, CSL processor, citations | W12 |
| OF04-W24 | Alphabetical index, master documents, line numbering, conditional and hidden content | W12 |
| OF04-W25 | Compare and merge documents (through tracked changes) | W13 |
| OF04-W26 | Mail merge, envelopes, labels | OF05 converter, CO03, CO01 |
| OF04-W27 | Signatures (ODF XML-DSig, OpenPGP), key-based encryption, DOCX encryption and signature verification | OF04-GAP-4, SE04, SH03 |
| OF04-W28 | Macros: `writer::` API, sandboxed engine, signing, recorder | FORGE-WP-09, SE04 |
| OF04-W29 | EPUB export; PDF import through OF03; DOC import | OF03 Convert, W15 |
| OF04-W30 | Asian and vertical typography, themes | 6.5 R-L09, G-03 |
| OF04-W31 | Local AI aids and model grammar | G-15, FORGE-WP-30 |

## 13. Open questions and risks

1. G-10 is the critical path. FUSE-WP-35 and FUSE-WP-36 are each rated XL in the build plan; the ＋ items in 6.5 (change tracking in the engine, total-fit breaking, structure tree, compatibility switches, record interface) add to them and must be agreed before the engine headers freeze.
2. DOCX fidelity depends on matching another product's undocumented layout behaviour. The target (class B with a measured corpus) is a goal, not a promise; the compatibility switches of R-L15 will grow over time.
3. Font metrics decide line breaks. Without metric-compatible substitutes for common proprietary fonts, class B is out of reach; which fonts ship is an owner decision.
4. Writing all format code in-house avoids large unsafe libraries but is the largest cost in this spec (W06, W14, W15, W17 and W29).
5. ODF 1.3 encryption with AES-GCM and Argon2id is newer than many readers; the compatibility fallback (CBC, PBKDF2) lacks authentication. The dialog must say which mode is used.
6. A maths layout engine does not exist in MathScript as read (it is a computer-algebra library with LaTeX and MathML notation output); OF04-GAP-2 needs an owner and OpenType MATH support in the FULL text profile.
7. The spelling, grammar, hyphenation and thesaurus service (G-17) has no interface or data set yet; dictionary and pattern licences must allow shipping.
8. Embedded fonts in documents are an attack surface and are ignored until FUSE's font parsing path is decided (FUSE-WP-17, FUSE-WP-65).
9. Unverified: MathScript's notation module was read only at the document level (`docs/LATEX_SUBSET.md`, `docs/API.md`); its fitness as a library call from a helper was not checked. The ODF 1.4 status was not checked.
