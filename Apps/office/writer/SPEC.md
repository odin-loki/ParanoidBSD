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
| OF04-F004 | Save | `Ctrl+S`, atomic (temporary file, `fsync`, rename). Saving to a foreign format shows what will be lost (F212) and offers "Keep a copy in ODT". | M | 1 |
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
| OF04-F028 | Word completion | Suggests words already in the document (min length 8, setting); off by default. | C | 3 |
| OF04-F029 | Drag and drop of text | Move or copy (`Ctrl`) selections inside and between documents. | M | 1 |
| OF04-F030 | Direct cursor | Click anywhere on a blank page area to start typing there; inserts paragraphs and tabs or alignment as needed (option). | C | 3 |
| OF04-F031 | Hidden text and conditional text | Character attribute "hidden"; paragraphs and sections hidden by a condition on a field or variable. | S | 3 |
| OF04-F032 | Hyperlinks | `Ctrl+K`(when a text view has focus the palette moves to `Ctrl+Shift+P`): to URL, document, heading, bookmark, e-mail. Links never open without `Ctrl+click`, and show the real target. | M | 1 |
| OF04-F033 | Bookmarks | Named anchors over a point or range; listed in the navigator; targets for links and references. | M | 2 |
| OF04-F034 | Sorting and case | Sort paragraphs or table rows (up to 3 keys, locale collation, numeric); change case: upper, lower, title, sentence, toggle, small caps attribute. | S | 2 |

### 4.3 Character and paragraph formatting

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F035 | Fonts | Family, style, size (1–999 pt, half-point steps), separate Western, Asian and complex-script fonts, language per run. Missing fonts are substituted and listed. | M | 1 |
| OF04-F036 | Character effects | Bold, italic, underline (9 styles, colour), overline, strike-through (single, double), superscript and subscript (auto or by offset), small caps, all caps, outline-free set: no shadow or emboss effects. | M | 1 |
| OF04-F037 | Colour and highlight | Text colour, highlight, character background; palette from theme tokens, document colours, custom; "automatic" follows background. | M | 1 |
| OF04-F038 | Spacing and position | Letter spacing (expand, condense), pair kerning, scale width, baseline shift, rotation 90/270 for table headers. | S | 2 |
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
| OF04-F055 | Conditional styles | A paragraph style can vary by context (in table header, in footnote, at list level n). | C | 3 |
| OF04-F056 | Load and manage styles | Import styles from another document or template (choose families, overwrite or not); rename; show usage count; find unused. | S | 2 |
| OF04-F057 | Style-first formatting bar | The toolbar's first control is the paragraph style box; direct-formatting buttons are present but styles are one key away. | M | 1 |
| OF04-F058 | Spotlight | Colour-code text by paragraph or character style and mark direct formatting in the margin, to find inconsistencies. | S | 3 |
| OF04-F059 | Themes | Document theme: heading and body fonts, a 12-colour palette, spacing scale; switching the theme restyles all dependent styles. | S | 3 |
| OF04-F060 | Default styles shipped | Body Text, Headings 1–10, Title, Subtitle, Quotations, Preformatted Text, List families, Caption, Footnote, Endnote, Header, Footer, Contents 1–10, Index, Bibliography, Table Contents, Table Heading, Frame Contents; character styles Emphasis, Strong, Source Text, Internet Link, Footnote Anchor, Page Number. | M | 1 |

### 4.5 Lists and numbering

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F061 | Bullets and numbering | Toggle with `Shift+F12` and `F12`; 10 levels; Tab and Shift+Tab change level; Enter on an empty item ends the list. | M | 1 |
| OF04-F062 | List styles | Per level: number format (1, a, A, i, I, native numerals of the language), prefix and suffix, show sublevels (1.2.3), start value, bullet character or image, character style, alignment, indent, tab or space after. | M | 1 |
| OF04-F063 | Heading numbering | One dialog ties heading styles to an outline list ("1", "1.1", "Appendix A"). | M | 2 |
| OF04-F064 | Continue, restart, set value | Per paragraph; continue a previous list across other content. | M | 2 |
| OF04-F065 | List reorder | `Ctrl+Alt+Up/Down` moves an item with or without its sub-items; promote and demote with sub-items. | S | 2 |
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
| OF04-F079 | Page background and borders | Per page style; area fills limited to colour, gradient (two-stop) and image. | S | 2 |
| OF04-F080 | Text grid | Lines-only or lines-and-characters grid for East Asian layout. | C | 3 |
| OF04-F081 | Envelopes and labels | Wizards: envelope formats and printer feed orientation; label sheets by brand-neutral dimensions table and custom; synchronised label content. | S | 3 |
| OF04-F082 | Booklet and n-up | At print time through the print pipeline (G-16). | S | 3 |
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
| OF04-F092 | Sort | By up to three columns or rows. | S | 2 |
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
| OF04-F109 | Gallery | Local folders of reusable images and shapes the user granted; nothing online. | C | 3 |
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
| OF04-F132 | Side-by-side review | Two documents in synchronised scroll. | C | 3 |
| OF04-F133 | Protect review | Lock tracking on with a password so changes cannot be made untracked. | S | 3 |
| OF04-F134 | Change summary | Generates a table of changes (page, author, type, text) as a new document. | C | 3 |

### 4.11 Mail merge and templates

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F135 | Data sources | CSV, ODS and XLSX sheets (through OF05's converter interface), vCard files, a CO03 contact group (by intent, after a prompt), SQLite files (read-only, parsed in the helper). No live network databases; OF09 can export a table. | S | 3 |
| OF04-F136 | Merge fields | Insert from the data source panel; address block and greeting line builders with locale rules; conditional fields ("if empty, skip line"); next-record field for labels. | S | 3 |
| OF04-F137 | Filter and sort | Choose records by tick, range, filter expression; sort by columns. | S | 3 |
| OF04-F138 | Preview | Step through records in place before producing anything. | S | 3 |
| OF04-F139 | Output | One combined document, one file per record (name pattern from fields), print, PDF per record, or hand the set to CO01 as drafts (never sent by Writer). | S | 3 |
| OF04-F140 | Templates | `.ott` files in the bundle, the user template folder, and granted folders; gallery with categories, preview, default per category; "Save as template"; documents remember their template and offer to update styles when it changes. | M | 1 |
| OF04-F141 | Shipped templates | Blank, letter (several locales), report, memo, minutes, CV, thesis, book, article with two columns, invoice, envelope and label sets, fax-free. | S | 2 |
| OF04-F142 | Template safety | Templates are documents: same sandboxed parsing, no macros run from templates unless signed and trusted. | M | 1 |
| OF04-F143 | Forms | Text, check box, drop-down, date fields in a document; "forms-only" protection; export as fillable PDF. | C | 3 |

### 4.12 Equations

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F144 | Equation objects | Inline or display; stored as MathML (Presentation) inside the ODF with the source text as an annotation; numbered with a right-aligned sequence field. | S | 2 |
| OF04-F145 | Input | `Alt+=` opens an inline editor: type the LaTeX subset that MathScript reads (Verified: `MATHSCRIPT:docs/LATEX_SUBSET.md`), or a linear "a/b", "x^2" form, or pick from a symbol palette. Input outside the subset is rejected with a position, never guessed. | S | 2 |
| OF04-F146 | MathScript hand-off | The source is parsed and exported to MathML by MathScript's notation module (Verified export names `latex`, `mathml`: `MATHSCRIPT:docs/API.md:203`) in a helper process. Writer does not evaluate. "Open in Notebook" sends the expression to AI04. | S | 2 |
| OF04-F147 | Maths layout | A MathML box layout (fractions, scripts, radicals, stretchy delimiters, matrices, alignment) using the font's OpenType MATH table; Proposed (`OF04-GAP-2`). Baseline aligns with the text line. | S | 2 |
| OF04-F148 | Equation tools | Convert between inline and display; numbering and cross-reference; copy as LaTeX or MathML; alt text generated from the expression ("x squared plus…") editable. | S | 3 |
| OF04-F149 | Import of foreign equations | OMML (DOCX) and MathType-free: OMML is converted to MathML in the helper; equation images stay images. | S | 3 |

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
| OF04-F157 | Optical margin alignment | Hanging punctuation and hyphens by a per-font protrusion table. | C | 3 |
| OF04-F158 | Typographic quotes and spaces | Language rules for quotes, apostrophes, narrow no-break spaces before punctuation (French), dashes. | S | 2 |
| OF04-F159 | Font embedding | Embed used fonts in ODT and PDF (subset), honouring the font's embedding permission bits; report fonts that forbid it. | S | 2 |
| OF04-F160 | Word count goals and readability | Target counts per document or section; readability indices computed locally. | C | 3 |
| OF04-F161 | Local AI help | Optional (`ai.local`): rewrite selection, summarise, translate, suggest headings; always shown as tracked changes the user accepts. | C | 3 |

### 4.14 Long documents

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F162 | Navigator | `F5`: tree of headings, tables, frames, images, objects, bookmarks, sections, links, references, indexes, comments, fields, footnotes; search; drag headings to move chapters; promote and demote with sub-headings. | M | 1 |
| OF04-F163 | Outline folding | Fold body text under headings in the document view; folded parts are skipped by print only when asked. | S | 2 |
| OF04-F164 | Master documents | `.odm` with linked sub-documents (each a portal grant); shared styles from the master; continuous page numbers, notes, captions, cross-references across files; export as one ODT. | S | 3 |
| OF04-F165 | Chapter numbering and running heads | Chapter fields in headers; "first/last heading on page" fields. | S | 2 |
| OF04-F166 | Go to | `Ctrl+G`: page, heading, bookmark, table, figure, comment, footnote, change. | M | 1 |
| OF04-F167 | Split and combine | Split a document at a heading level into files and a master; insert a document at the caret. | C | 3 |

### 4.15 Accessibility checker

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF04-F168 | Checks | Missing alt text; heading levels skipped; fake headings (bold large text) and fake lists; tables without header rows or with merged cells; low contrast text (< 4.5:1); missing document title and language; links whose text is the URL; blinking or tiny text; empty paragraphs used for spacing; manual numbering; floating text with unclear reading order; footnotes typed by hand; colour-only meaning (heuristic). | M | 2 |
| OF04-F169 | Fix flow | Sidebar list by severity; each item jumps to the place and offers a fix (add alt text, apply heading style, set header row); "ignore" with a reason stored in the document. | M | 2 |
| OF04-F170 | Check before export | Runs before tagged PDF and EPUB export; export proceeds with a summary of open issues. | S | 2 |
