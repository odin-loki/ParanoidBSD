<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Fonts and Characters

By Odin Loch

| | |
|---|---|
| ID | `UT03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.fonts` |
| Category | utils |
| Priority | P2 |
| Backing repos | FUSE (text profiles, `fontPicker` widget: Verified in `FUSE:docs/ui/README.md`); FORGE (Unicode data in the text and locale layer: Proposed use) |
| Status | Planned. Nothing built. |

## 1. Purpose

Fonts and Characters manages the fonts on the machine and is the reference for every character in Unicode. Users install, preview, compare and inspect fonts, and find and copy any character. Font files are hostile data, so every font is parsed and rendered in a helper that holds no file, network or device handle. It replaces the Plasma font management module and KCharSelect (PLATFORM.md V20).

## 2. Scope

In scope:

- Font library: per-user and system fonts, install, remove, enable, disable, collections.
- Preview: custom text, waterfall, paragraphs, scripts, colour fonts, variable-font axes.
- OpenType feature inspector, glyph grid, glyph details.
- Character map: Unicode blocks and scripts, search by name, recent, favourites, copy, code points, how to type it.
- Font comparison, metadata and licence view.
- Duplicate, broken and risky font detection; a validation report.
- Export of font subsets and of glyphs as SVG.

Out of scope:

| Item | Owner |
|---|---|
| Choosing the system UI, document and monospace fonts, rendering options (hinting, subpixel) | SY01 Settings |
| The font service that discovers and serves fonts to apps | platform (FUSE-WP-17); this app is its management front end |
| The quick emoji and symbol picker, compose sequences, input methods | AX04 Input Methods and Emoji |
| Designing or editing glyph outlines | not planned |
| Downloading fonts from online catalogues | not planned (no network); IN03 Transfers fetches files if the user wants |
| Installing fonts that arrive in packages | SY05 Software |
| Font choice inside documents | each editor, with `fontPicker` |

## 3. Users and top tasks

Personas:

- **Everyday user**: got a font file, wants to see it and install it; needs a symbol now and then.
- **Designer or writer**: compares faces, explores features and axes, groups fonts by project.
- **Linguist or developer**: needs to know what a code point is, which fonts cover it, and how text will shape.
- **Admin**: installs fonts for all users, removes broken ones, checks licences.

| # | Task | Target steps |
|---|---|---|
| T1 | Preview a font file from Files | 1 (double click) |
| T2 | Install it for the current user | 1 more (Install) |
| T3 | Find a character by name and copy it | 3 (`Ctrl+L`, type, Enter) |
| T4 | Identify a pasted character | 2 (paste, read) |
| T5 | Compare two fonts with the same text | 3 |
| T6 | Try a variable font's weight and width | 2 (select font, drag slider) |
| T7 | See which OpenType features a font has and toggle one | 2 |
| T8 | Disable a font family without deleting it | 2 |
| T9 | Find which installed fonts contain a given character | 2 |
| T10 | Find and remove duplicate fonts | 3 |
| T11 | Read a font's licence and embedding permission | 2 |
| T12 | Export a subset with only Latin and digits | 4 |

## 4. Features

### 4.1 Library

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F001 | Family list | All fonts grouped by family, each row rendered in its own face (by the helper), with style count and badges: variable, colour, monospace, user/system, disabled. | M | 1 |
| UT03-F002 | Grid and list modes | Grid of sample cards or a compact list. Sample text is editable. | M | 1 |
| UT03-F003 | Sources | Sidebar: All, User, System, App-bundled (read-only, listed per app), Disabled, Recently added. | M | 1 |
| UT03-F004 | Filters | By classification (serif, sans, mono, script, display, symbol), script coverage, weight range, width, italic, variable, colour, licence class, has feature tag. | S | 2 |
| UT03-F005 | Search | By family, style, PostScript name, foundry, designer; by "contains character X". | M | 1 |
| UT03-F006 | Install per user | Copies the validated file into the user font directory through the font service and refreshes the catalogue. No restart of apps. | M | 1 |
| UT03-F007 | Install for all users | Same, into the system font directory; needs the admin prompt drawn by SH04. | S | 2 |
| UT03-F008 | Install by drop or open | Drop font files on the window or open them from SY02; a preview window shows first, Install is one click. | M | 1 |
| UT03-F009 | Batch install | Many files or a folder at once with one validation report. | S | 2 |
| UT03-F010 | Remove | Moves the file to trash through `svc.undo-trash`; Undo restores it. System fonts shipped by the OS cannot be removed (they are on the verified dataset) and say why. | M | 1 |
| UT03-F011 | Enable and disable | A disabled font stays on disk but the font service does not offer it. Per family or per style. | M | 1 |
| UT03-F012 | Collections | Named groups; a font can be in many. Enable or disable a whole collection. | S | 2 |
| UT03-F013 | Smart collections | Saved filters ("Monospace with Cyrillic"). | C | 3 |
| UT03-F014 | Favourites | Star a family; a Favourites source. | S | 2 |
| UT03-F015 | Protected fonts | Fonts the system needs (UI, fallback, monospace) cannot be disabled; the row shows a lock and the reason. | M | 1 |
| UT03-F016 | Activation for a session | "Enable until I log out" for trying fonts without changing the library. | C | 3 |
| UT03-F017 | Font collections files | `.ttc` and `.otc` list each face separately. | M | 1 |
| UT03-F018 | Fallback chain view | Shows, for a language or script, which fonts the system will try in order; hands off to SY01 to change it. | S | 3 |
| UT03-F019 | Export and import library state | Collections, favourites and disabled list as JSON. | C | 3 |
| UT03-F020 | Print a specimen | One-page specimen (name, alphabet, waterfall, paragraph, metadata) through `svc.portal.print`. | S | 2 |

### 4.2 Preview

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F030 | Custom text | One field drives every preview. Presets: pangrams per language, alphabet, digits and punctuation, a paragraph, a code sample, "all characters the font has". | M | 1 |
| UT03-F031 | Waterfall | The text at 8, 9, 10, 11, 12, 14, 18, 24, 36, 48, 72, 96 px (list editable). | M | 1 |
| UT03-F032 | Paragraph view | Multi-line text with controls for size, line height, letter spacing, alignment, column width. | S | 2 |
| UT03-F033 | Size and zoom | Size slider 6–400 px; `Ctrl++/-/0`. | M | 1 |
| UT03-F034 | Colours and background | Text and background colour, light/dark swatch, checkerboard for colour fonts. | S | 2 |
| UT03-F035 | Rendering options | Hinting (none, slight, full), antialiasing (grey, subpixel, none), for comparison only; system defaults are in SY01. | C | 3 |
| UT03-F036 | Script and language | Set the OpenType script and language tag so localised forms appear. | S | 2 |
| UT03-F037 | Direction | Left-to-right, right-to-left, auto. | S | 2 |
| UT03-F038 | Missing glyph marking | Characters the font lacks are drawn as a marked box, never by silent fallback; a count is shown. Toggle "allow fallback" to see what users would get. | M | 1 |
| UT03-F039 | Colour fonts | COLR v0/v1, CPAL palettes (palette chooser), bitmap (CBDT, sbix) and SVG-in-OpenType tables. SVG glyphs render through the restricted SVG subset only (FUSE-WP-27). | S | 2 |
| UT03-F040 | Preview without installing | A font file opens in a preview window served by the helper from the passed handle; nothing is installed or cached outside the app cache. | M | 1 |
| UT03-F041 | Copy preview as image or SVG | Copy the rendered line as PNG, or outlines as SVG. | C | 3 |
| UT03-F042 | Baseline and metrics overlay | Lines for baseline, x-height, cap height, ascender, descender, line gap. | S | 2 |

### 4.3 Variable fonts and OpenType features

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F050 | Axes sliders | One slider per axis in `fvar` with tag, name, min, default, max, and a numeric field. Registered axes (wght, wdth, slnt, ital, opsz) first. | M | 2 |
| UT03-F051 | Named instances | Dropdown of named instances; choosing one sets the sliders. | M | 2 |
| UT03-F052 | Optical size link | Option to tie `opsz` to the preview size. | C | 3 |
| UT03-F053 | Copy settings | Copy the current axes as CSS `font-variation-settings` and as a FUSE style line. | S | 2 |
| UT03-F054 | Axis animation | Sweep an axis back and forth; honours reduced motion (off then). | C | 3 |
| UT03-F055 | Feature list | Every GSUB and GPOS feature per script and language, with the registered name ("liga: Standard Ligatures") and, for `ssNN` and `cvNN`, the font's own names. | M | 2 |
| UT03-F056 | Feature toggles | Tick boxes apply features to the preview; defaults shown as in a normal shaper. | M | 2 |
| UT03-F057 | Feature samples | For a selected feature, list the glyphs it changes, before and after. | S | 3 |
| UT03-F058 | Copy features | Copy as CSS `font-feature-settings`. | S | 2 |
| UT03-F059 | Shaping trace | For the preview text show input code points → glyph ids → positions, cluster by cluster, with the lookups that fired. | C | 3 |
| UT03-F060 | Kerning view | Show pair adjustments in the preview as coloured gaps with values; toggle kerning. | C | 3 |

### 4.4 Glyphs

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F070 | Glyph grid | Every glyph in the font in a virtualised grid: by glyph id, by Unicode order, or grouped by block. Cell size adjustable. | M | 1 |
| UT03-F071 | Glyph filter | By name, code point, block, "unencoded only", "has colour layers", "composite". | S | 2 |
| UT03-F072 | Glyph details | Large outline with metrics lines; glyph id, name, code points mapping to it, advance, side bearings, bounding box, contour and point counts, component list, class (base, mark, ligature). | M | 2 |
| UT03-F073 | Outline points | Show on-curve and off-curve points and contour direction. | C | 3 |
| UT03-F074 | Alternates | List glyphs reachable from this one through GSUB features. | S | 3 |
| UT03-F075 | Copy glyph | Copy the character (if encoded), the code point, the glyph name, or the outline as SVG path data. | S | 2 |
| UT03-F076 | Coverage summary | Per Unicode block and per script: covered / total, with a bar; per language "supports / lacks n characters" from CLDR exemplar sets. | S | 2 |
| UT03-F077 | Missing characters list | For a chosen language or text, list what the font lacks. | S | 2 |

### 4.5 Character map

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F080 | Blocks and scripts | Sidebar tree of Unicode blocks and of scripts; the grid shows the characters. Planes 0–16; unassigned code points are hidden by default. | M | 1 |
| UT03-F081 | Search by name | Words of the character name, aliases, Unihan definitions and CLDR annotation keywords ("arrow right", "euro"). Ranked; works offline from a bundled index. | M | 1 |
| UT03-F082 | Search by code point | `U+20AC`, `20AC`, `&#8364;`, `€`, decimal. | M | 1 |
| UT03-F083 | Identify text | Paste or type any text: each code point is listed with name, block, category; grapheme clusters are grouped; invisible and format characters are shown as labelled boxes. | M | 1 |
| UT03-F084 | Character details | Name, aliases, code point, UTF-8/UTF-16/UTF-32 bytes, category, script, bidi class, combining class, decomposition, case mappings, age (Unicode version), East Asian width, emoji properties, block, confusables, related characters. | M | 1 |
| UT03-F085 | Escapes | The character as HTML entity (named and numeric), CSS, C/C++, ForgeScript, JSON, URL-encoded, regex. Each has a copy button. | S | 2 |
| UT03-F086 | How to type it | Compose sequence, the key on current layouts, and the `Ctrl+Shift+U` code-point entry, from AX04's tables over its bus interface. | S | 2 |
| UT03-F087 | Fonts that have it | List of installed fonts containing the character, each rendering it. | M | 2 |
| UT03-F088 | Font for the grid | Draw the grid in a chosen font or "system fallback". Characters the chosen font lacks are dimmed (option: hidden). | M | 1 |
| UT03-F089 | Copy | `Enter` copies the selected character; a copy buffer collects several before copying. | M | 1 |
| UT03-F090 | Recent and favourites | Last 64 copied characters; starred characters; both in the sidebar. | M | 1 |
| UT03-F091 | Variation and emoji sequences | Variation selectors, skin-tone and ZWJ sequences listed under their base character. The quick picker is AX04; this is the reference. | S | 2 |
| UT03-F092 | Confusables and security view | Shows characters that look alike in other scripts, mixed-script state of pasted text, bidi controls and invisible characters. Useful for checking addresses and file names. | S | 2 |
| UT03-F093 | Normalisation | Show NFC, NFD, NFKC, NFKD of the pasted text and whether they differ. | S | 2 |
| UT03-F094 | Unicode version | About page shows the bundled Unicode and CLDR versions. | M | 1 |
| UT03-F095 | Send to app | "Insert" emits the character to the last focused text field through AX04's insertion interface (the user must have invoked the app from that field). | C | 3 |
| UT03-F096 | Share with search | Launcher phrases "char euro", "U+2603" answer from this app's index through `svc.search`. | C | 3 |

### 4.6 Comparison, metadata, licence

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F100 | Side-by-side compare | Two to four fonts, same text and size, stacked or in columns. | S | 2 |
| UT03-F101 | Overlay compare | Two fonts drawn on top of each other in two colours with adjustable opacity; align by baseline. | S | 2 |
| UT03-F102 | Metrics compare | Table of units per em, ascender, descender, x-height, cap height, average width, for the compared fonts. | C | 3 |
| UT03-F103 | Coverage compare | Characters only in A, only in B, in both. | C | 3 |
| UT03-F104 | Version compare | Two versions of the same font: added, removed and changed glyphs (by outline hash). | C | 3 |
| UT03-F105 | Metadata view | All `name` records (family, style, full, PostScript, version, designer, manufacturer, URLs as plain text), `OS/2` (weight, width, vendor id, PANOSE, embedding bits), `head` dates, table list with sizes and checksums. | M | 1 |
| UT03-F106 | Licence view | Licence text and licence URL from the font, shown as plain text (links are not followed). A plain summary: detected licence (OFL, Apache, Ubuntu, proprietary, unknown) by text match against a bundled list. | M | 2 |
| UT03-F107 | Embedding permission | `fsType` decoded: installable, editable, preview and print, restricted, no subsetting, bitmap only. Subset export obeys it. | M | 2 |
| UT03-F108 | File information | Path (or "from Files, not installed"), size, format (TrueType, CFF, CFF2, collection, WOFF, WOFF2), hash, install date, provenance tag, signature state. | M | 1 |
| UT03-F109 | Used by | Which apps bundle their own copy; which documents use it is out of scope. | C | 3 |

### 4.7 Health and safety

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F120 | Validation on open and install | The helper runs structural checks (table directory, offsets, lengths, checksums, required tables, glyph count limits, recursion depth of composites, hint program length) before any rendering. Failing fonts are not installed. | M | 1 |
| UT03-F121 | Sanitised install | What is installed is the helper's re-serialised copy with only known tables, not the original bytes (option to keep unknown tables off by default). The original hash is recorded. | S | 2 |
| UT03-F122 | Hinting programs off | TrueType bytecode is not executed for previews of fonts that are not installed. For installed fonts the system policy (SY01) decides; default is no bytecode interpreter. | M | 1 |
| UT03-F123 | Duplicate detection | Same file hash; same PostScript name and version; same family and style from different files. A resolution view keeps the newest or the system copy. | S | 2 |
| UT03-F124 | Broken font report | Fonts that fail validation or crash the helper are listed with the failing check; "Disable" and "Move to trash" offered. | M | 2 |
| UT03-F125 | Helper crash quarantine | A font whose parsing kills the helper is marked quarantined by hash and is not opened again without an explicit "Try again". | M | 1 |
| UT03-F126 | Resource limits | Helper has memory and CPU-time limits per font (default 256 MiB, 5 s per request); exceeding them fails that font only. | M | 1 |
| UT03-F127 | Name conflicts | Warn when a user font shadows a system family name; show which one wins. | S | 2 |
| UT03-F128 | Library scan | "Check all fonts" job with progress and a report. | S | 2 |
| UT03-F129 | Provenance and audit | Installs, removals and quarantines go to `svc.audit` with the file hash and source. | S | 2 |

### 4.8 Export

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F140 | Subset export | Write a new font with chosen characters (by block, script, language, or typed text), chosen features and, for variable fonts, pinned or limited axes. Formats: TTF/OTF, WOFF2. Refused when `fsType` forbids subsetting. | S | 3 |
| UT03-F141 | Glyphs as SVG | Selected glyphs as one SVG each or one sheet. | C | 3 |
| UT03-F142 | Character table export | Coverage list as text, CSV or JSON. | C | 3 |
| UT03-F143 | Specimen as PDF | The print specimen saved through print-to-PDF (UT06). | C | 3 |
| UT03-F144 | Copy as text list | Copy all characters of a block or a font's coverage as a string. | S | 2 |

### 4.9 Settings, recovery, automation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT03-F150 | Preferences | Section 8. | M | 1 |
| UT03-F151 | Undo | Install, remove, enable, disable and collection edits are undoable in the session. | M | 1 |
| UT03-F152 | Catalogue rebuild | "Rebuild font catalogue" re-scans through the font service. | S | 2 |
| UT03-F153 | Session restore | Reopens the last view, font, text and axis settings. | S | 1 |
| UT03-F154 | Actions in the palette | Every action in the shared action table. | M | 1 |

## 5. GUI

### 5.1 Window map

```
Fonts and Characters
├─ Main window, two modes (header switch): Fonts | Characters
│  ├─ Fonts mode
│  │  ├─ Sidebar: sources, collections, filters
│  │  ├─ Family list / grid
│  │  ├─ Font page (tabs: Preview | Waterfall | Glyphs | Features | Axes | Coverage | Info)
│  │  └─ Inspector: glyph details / metadata
│  └─ Characters mode
│     ├─ Sidebar: Recent, Favourites, Scripts, Blocks
│     ├─ Character grid
│     ├─ Identify panel (paste text)
│     └─ Inspector: character details
├─ Font file preview window (not installed)
├─ Compare window
├─ Install report sheet
├─ Duplicates sheet
├─ Library check report
├─ Subset export dialog
├─ Collection editor popover
├─ Specimen print preview (through SH04)
└─ Preferences
```

### 5.2 Main window

Shown at 1200×720, Fonts mode, a family selected, Preview tab.

```
+------------------------------------------------------------------------------------------+
| [=] Fonts and Characters  (1)[ Fonts | Characters ]   [Search fonts Ctrl+F]   [_][o][x] |
+-----------+------------------------+-----------------------------------+-----------------+
| SOURCES(2)| Sample [Hamburgefonstiv](3)| Source Serif  — 8 styles · variable (5)| INFO  (8)|
| All   212 | [Grid|List] [Filter v] | [Preview|Waterfall|Glyphs|Features|    | Family          |
| User   38 +------------------------+  Axes|Coverage|Info]                  | Source Serif    |
| System174 | > Source Serif    VAR  +-----------------------------------+ Version 4.005   |
| Disabled 6|   Hamburgefonstiv      | Text [The quick brown fox…___] [v]| Format OTF/CFF2 |
| Recent    |   Inter           VAR  | Size [----o--------] 48  Style[Regular v]| Glyphs 1 562   |
| COLLECTIONS|  Hamburgefonstiv      |                               (6) | Licence OFL 1.1 |
| ★ Favourites| JetBrains Mono  MONO |  The quick brown fox jumps over   | Embedding       |
| Project A |   Hamburgefonstiv      |  the lazy dog 0123456789          |  installable    |
| Monospace |   Noto Sans Arabic     |                                   | Source  User    |
| [+ New]   |   ‫ابجد هوز‬           |  2 characters missing: ☃ 𝔄  [Show]| Hash 3f9a…c21   |
| FILTERS   |   Old Display   [off]  |                               (7) | Status ✓ valid  |
| [ ] Mono  |   Hamburgefonstiv      | Axes  wght [-----o----] 430       | [Disable]       |
| [ ] Variable|                  (4) |       opsz [--o-------] 12        | [Remove…]       |
+-----------+------------------------+-----------------------------------+-----------------+
| 212 families · 38 user · 6 disabled          (9)[✓ All fonts valid]   Unicode 16 · CLDR  |
+------------------------------------------------------------------------------------------+
```

1. Mode switch (`tabBar`). `Ctrl+1` Fonts, `Ctrl+2` Characters.
2. Sidebar (`treeView`): sources with counts, collections, quick filter `checkBox`es.
3. Sample text `lineEdit` for list rows; view and filter controls.
4. Family list (`modelListView`; rows hold a `label` and an `image` rendered by the helper). Badges are words: VAR, MONO, COLOUR, off.
5. Font page header and `tabBar`.
6. Preview surface: an `image` in a `scrollArea`, drawn from a bitmap the helper returns (see 6.1). Controls: `lineEdit` with presets `comboBox`, size `slider`, style `comboBox`.
7. Missing-glyph line and quick axes (`slider`s) when the font is variable.
8. Inspector (`propertyGrid`, read-only) with the two state actions. Remove is outlined `--danger`.
9. Status: library health badge (`--ok` valid, `--warn` n warnings, `--danger` n broken or quarantined).

States: empty (cannot be empty: system fonts exist; a collection can be: "Drag fonts here"), loading (rows appear with a neutral placeholder until the helper returns each sample), error (font failed: card shows "Cannot be read" and the reason), read-only (system fonts: Remove disabled with reason), narrow (sidebar overlay, list and page become a two-level navigation, inspector a sheet).

### 5.3 Other views and dialogs

**Characters mode** (1200×720)

```
+------------------------------------------------------------------------------------------+
| [=] Fonts and Characters   [ Fonts | Characters ]  [Search characters Ctrl+L]  [_][o][x] |
+-----------+----------------------------------------------------------+-------------------+
| Recent    | Font [System fallback v]  Size [--o---]  [x] Dim missing | €             (3) |
| ★ Favourites| Block: Currency Symbols  U+20A0–U+20CF            (1) | EURO SIGN         |
| SCRIPTS   | ₠ ₡ ₢ ₣ ₤ ₥ ₦ ₧ ₨ ₩ ₪ ₫ [€] ₭ ₮ ₯                        | U+20AC            |
|  Latin    | ₰ ₱ ₲ ₳ ₴ ₵ ₶ ₷ ₸ ₹ ₺ ₻ ₼ ₽ ₾ ₿                          | Sc Currency symbol|
|  Greek    | ⃀                                                         | Script Common     |
|  Arabic   |                                                          | Since Unicode 2.1 |
| BLOCKS    |                                                          | UTF-8  E2 82 AC   |
|  Basic Latin|                                                        | UTF-16 20AC       |
|  …        +----------------------------------------------------------+ HTML  &euro;      |
| > Currency| Identify [paste or type text here________________]   (2) | Type: Compose = e |
|  Arrows   |  U+0065 e LATIN SMALL LETTER E                           |       AltGr+E (de)|
|  Emoji    |  U+0301 ◌́ COMBINING ACUTE ACCENT  ← 1 cluster            | In 148 fonts [Show]|
|           |  U+200B [ZWSP] ZERO WIDTH SPACE   ! invisible            | [Copy] [★] [Add to|
|           |                                                          |  buffer]          |
+-----------+----------------------------------------------------------+-------------------+
| Buffer: € → ✓      [Copy buffer] [Clear]                  Unicode 16.0 · 154 998 chars   |
+------------------------------------------------------------------------------------------+
```

1. Character grid (`gridView`, virtualised; cells are glyph images from the helper, keyed by font and code point). 2. Identify panel (`lineEdit` plus `tableView`); invisible characters get a `--warn` flag in words. 3. Details (`propertyGrid` plus copy `toolButton`s per row).

**Glyphs tab** (font page): `gridView` of all glyphs with a toolbar (order `comboBox`, filter `lineEdit`, cell size `slider`); selecting a glyph fills the inspector:

```
+-- Glyph 412 "eacute" ---------------------+
|        ┌─────────────────────┐            |
|  asc ──│        ´            │            |
|  x-h ──│      ╭───╮          │            |
|  base──│      ╰───┘          │            |
|  desc──└─────────────────────┘            |
| Code points  U+00E9                       |
| Advance 520 · LSB 42 · RSB 38 (units)     |
| Box  42,-12 → 482,720                     |
| Composite: e (71) + acutecomb (318)       |
| Class base · 2 contours · 34 points       |
| [Copy char] [Copy SVG path] [Alternates]  |
+-------------------------------------------+
```

**Features tab** (font page, 2 columns)

```
+------------------------------------------------------------------------------------------+
| Script [Latin v]  Language [Default v]                       [Reset] [Copy as CSS]       |
+--------------------------------------+---------------------------------------------------+
| [x] kern  Kerning            default | Preview                                           |
| [x] liga  Standard Ligatures default |  office fluffy 1/2 2nd  →  oﬃce ﬂuﬀy ½ 2ⁿᵈ       |
| [ ] dlig  Discretionary Ligatures    |                                                   |
| [x] frac  Fractions                  | Changed by "frac":                                |
| [ ] onum  Oldstyle Figures           |  1/2 → ½    3/4 → ¾    (12 substitutions)         |
| [ ] ss01  "Single-storey a"          |                                                   |
| [ ] ss02  "Slashed zero"             |                                                   |
| [ ] cv05  Character Variant 5        |                                                   |
+--------------------------------------+---------------------------------------------------+
```

`listView` of `checkBox` rows; preview `image`; substitution list `tableView`.

**Compare window** (1100×640)

```
+------------------------------------------------------------------------------------------+
| [=] Compare                [Side by side | Overlay | Metrics]                 [_][o][x] |
+------------------------------------------------------------------------------------------+
| Text [Hamburgefonstiv 0123456789______]  Size [---o----] 64   [+ Add font]  (max 4)     |
+------------------------------------------------------------------------------------------+
| Source Serif Regular            [x]                                                      |
|   Hamburgefonstiv 0123456789                                                             |
| Inter Regular                   [x]                                                      |
|   Hamburgefonstiv 0123456789                                                             |
| Noto Serif Regular              [x]                                                      |
|   Hamburgefonstiv 0123456789                                                             |
+------------------------------------------------------------------------------------------+
| x-height 475 / 546 / 536 (per 1000 units)        [x] Align baselines   [x] Show metrics  |
+------------------------------------------------------------------------------------------+
```

**Font file preview window** (720×520, for a file that is not installed)

```
+------------------------------------------------------------------------------------------+
| [=] NewFace-Regular.otf — not installed                                       [_][o][x] |
+------------------------------------------------------------------------------------------+
| ! From a download (example.org). Parsed in a sandbox. Hinting programs are off.     (1) |
+------------------------------------------------------------------------------------------+
| NewFace Regular · version 1.2 · 642 glyphs · licence: unknown                            |
|                                                                                          |
|   The quick brown fox jumps over the lazy dog                                            |
|   ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789                       |
|                                                                                          |
| Validation: ✓ structure  ✓ checksums  ! 2 unknown tables will be dropped on install (2)  |
|                       [Details…]   [Install for all users…]   [Install]                  |
+------------------------------------------------------------------------------------------+
```

1. Provenance banner, shown only for untrusted sources. 2. Validation line; a failed check replaces Install with a disabled button and the reason.

**Install report sheet** (batch): `tableView` with file, family, result (installed, already installed, duplicate, failed validation, quarantined), reason; `[Undo install] [Done]`.

**Duplicates sheet**: groups in a `treeView`; each group lists copies with source, version, hash, path; radio to choose which to keep; `[Disable others] [Move others to trash]`.

**Subset export dialog** (600×520)

```
+-- Export subset of Source Serif ---------------------------------------------+
| Characters  (•) Blocks/scripts  ( ) Language  ( ) From text                 |
|   [x] Basic Latin  [x] Latin-1  [ ] Latin Ext-A  [ ] Greek  [ ] Cyrillic …  |
| Features    [x] kern [x] liga [ ] smcp [ ] onum   [Keep all] [Keep none]    |
| Axes        wght [Keep range 400–700 v]   opsz [Pin at 12 v]                |
| Format      [WOFF2 v]      [ ] Keep hinting                                 |
| Result      218 glyphs, about 24 KiB (from 1 562 glyphs, 410 KiB)           |
| Licence: OFL 1.1 allows modification; a renamed family may be required. (1) |
|                                              [Cancel]  [Export…]            |
+-----------------------------------------------------------------------------+
```

1. Plain licence note from the detected licence class; when `fsType` forbids subsetting the dialog shows only the reason and Close.

### 5.4 Menus

```
File    Open Font File… Ctrl+O · Install Fonts… Ctrl+I · Install for All Users…
        New Collection Ctrl+N · Export Subset… · Export Glyphs as SVG… · Print Specimen… Ctrl+P
        Close Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Ctrl+C · Copy Code Point Ctrl+Shift+C
        Copy as ▸ (HTML entity, CSS, ForgeScript, JSON, SVG path) · Add to Buffer Ctrl+B
        Find Font Ctrl+F · Find Character Ctrl+L · Preferences Ctrl+,
View    Fonts Ctrl+1 · Characters Ctrl+2 · Grid / List · Font Page Tabs ▸ (Alt+1…Alt+7)
        Metrics Overlay Ctrl+M · Allow Fallback · Show Unassigned · Inspector F4 · Sidebar F9
        Zoom In Ctrl++ · Zoom Out Ctrl+- · Reset Ctrl+0
Font    Enable/Disable Ctrl+E · Remove Delete · Add to Collection ▸ · Favourite Ctrl+D
        Compare… Ctrl+Shift+D · Validate · Show in Files · Characters in This Font
Library Check All Fonts… · Find Duplicates… · Rebuild Catalogue F5 · Fallback Order (opens Settings)
Help    Fonts Help F1 · About Unicode Data · About
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Ctrl+1` / `Ctrl+2` | Fonts / Characters |
| `Ctrl+L` | Character search |
| `Enter` (character grid) | Copy the character |
| `Ctrl+B` | Add to buffer |
| `Arrow keys` | Move in grids; `PageUp/Down` by screen; `Home/End` block start and end |
| `Ctrl+G` | Go to code point |
| `Alt+1…7` | Font page tabs |
| `Ctrl+E` | Enable or disable |
| `Ctrl+Shift+D` | Compare selected fonts |
| `Ctrl+M` | Metrics overlay |

Context menus: family (Preview, Enable/Disable, Add to Collection ▸, Favourite, Compare, Validate, Show in Files, Remove); glyph (Copy Character, Copy Code Point, Copy Glyph Name, Copy SVG Path, Show in Character Map, Alternates); character (Copy, Copy as ▸, Add to Buffer, Favourite, Fonts That Have It, Show Block).

Drag and drop: font files from SY02 onto the window (preview and install); families onto a collection; a character from the grid into any text field (typed offer `text/plain`); a family onto the Compare window.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Empty collection or empty search: one line and a "Clear filters" button. |
| Loading | Placeholder cells; samples fill in as the helper answers. Catalogue scan shows progress in the status bar. |
| Error | Helper crashed on a font: toast "NewFace could not be read and was quarantined", details link. Font service unreachable: banner, read-only mode from the cached catalogue. |
| Offline | Always. |
| Read-only | System fonts and app-bundled fonts: no Remove; Disable allowed unless protected. |
| Narrow (<640) | Two-level navigation; character details as a bottom sheet. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Mode switch, page tabs | `tabBar`, `tabWidget` |
| Sidebars | `treeView`, `checkBox` |
| Family list | `modelListView`, `gridView` |
| Preview surfaces | `image` in `scrollArea` |
| Glyph and character grids | `gridView` |
| Inspector | `propertyGrid` |
| Feature and axis panels | `listView`, `checkBox`, `slider`, `spinBox`, `comboBox` |
| Tables | `tableView` |
| Dialogs | `dialog`, `radioButton`, `lineEdit` |

Missing: popovers and sheets (FUSE-WP-30); a glyph-outline view with points and metric lines (custom paint on `Painter`, FUSE-WP-23; `UT03-GAP-1`); the system font service with enable/disable and per-user install (FUSE-WP-17; `UT03-GAP-2` if that work package does not include management calls).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | App host | Library views, collections, character map, search, export orchestration |
| UI | FUSE UI | App host | All windows |
| `font-helper` | C++23 | One per untrusted font, pooled for installed fonts; capability mode | Parse, validate, sanitise, shape and rasterise. Holds only the font handle (read) and a shared-memory handle for output bitmaps. Returns metadata records, glyph bitmaps, outlines as point lists, shaping results |
| `subset-helper` | C++23 | One per export | Reads one font handle, writes one output handle |
| Unicode database | Read-only data in the bundle (`ucd.bin`, `names.idx`, `cldr-annot.bin`) | App host | Names, properties, blocks, scripts, confusables, annotations |
| Font service client | ForgeScript | App host | Catalogue queries, install, remove, enable, disable (the service does the file moves) |

```
SY02 / intent ──font handle──► App host (ForgeScript + FUSE UI) ──► font service (platform)
                                   │ bus: requests (text, size, axes, features)      │ installs,
                                   ▼                                                  ▼ catalogue
                             font-helper (cap mode)                            user/system font dirs
                    fds: font (ro), shm (bitmaps out). No fs, no net, no GPU.
```

Why previews are bitmaps from the helper: the app host must never run a font parser on untrusted bytes. The UI's own text stack only ever loads fonts that the font service has validated and installed. An uninstalled or suspect font is therefore drawn in the helper and shown as an `image`.

Engine choice inside the helper: FUSE's FULL profile uses FreeType and HarfBuzz (`FUSE:docs/ui/README.md`, V6). The helper may link the same vendored libraries (FreeType: FTL/GPLv2 dual licence; HarfBuzz: MIT-style "Old MIT") because they are contained and already needed by G-03. The validator and sanitiser are own C++23 code and run before either library sees the file. TrueType bytecode interpretation is compiled out of the helper.

### 6.2 Data model

Stores: `~/.local/share/org.pbsd.fonts/library.sqlite` (collections, favourites, quarantine, recent characters), `~/.cache/org.pbsd.fonts/` (sample bitmaps keyed by font hash, text, size, axes). The font catalogue itself belongs to the font service.

| Entity | Fields |
|---|---|
| `FontFace` (from the service, cached) | `face_id: u64`, `file_hash: bytes32`, `face_index: u16`, `family: string`, `style: string`, `full_name`, `postscript_name`, `version: string`, `format: enum{ttf, cff, cff2, ttc, woff, woff2}`, `weight: u16`, `width: u8`, `italic: bool`, `monospace: bool`, `variable: bool`, `colour: enum{none, colr0, colr1, bitmap, svg}`, `glyph_count: u32`, `source: enum{user, system, app}`, `enabled: bool`, `protected: bool`, `fs_type: u16`, `licence_class: enum`, `valid: enum{ok, warnings, broken, quarantined}` |
| `Axis` | `face_id`, `tag: char4`, `name`, `min: f32`, `def: f32`, `max: f32`, `hidden: bool` |
| `NamedInstance` | `face_id`, `name`, `coords: list<f32>` |
| `Feature` | `face_id`, `table: enum{gsub, gpos}`, `script: char4`, `lang: char4`, `tag: char4`, `ui_name: string?` |
| `Coverage` | `face_id`, `ranges: list<(u32, u32)>` (compressed), `count: u32` |
| `Collection` | `id: u32`, `name`, `kind: enum{manual, smart}`, `filter_json: string?`, `order: u16` |
| `CollectionMember` | `collection_id`, `family: string` |
| `Quarantine` | `file_hash`, `when: i64`, `reason: string`, `check: string` |
| `ValidationResult` | `file_hash`, `checks: list<{id: string, state: enum{pass, warn, fail}, detail: string}>`, `validator_version: u16` |
| `RecentChar` | `cluster: string`, `last_used: i64`, `count: u32`, `favourite: bool` |
| `PreviewState` | `text`, `size_px: u16`, `script: char4`, `lang: char4`, `features: map<char4, u8>`, `axes: map<char4, f32>`, `direction: enum` |

Unicode data files carry the Unicode version in a header; the app refuses a file whose version field does not match the index it was built with. `library.sqlite` has `meta.schema_version` and migrates forward with a copy kept.

### 6.3 File formats and protocols

| Format | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| OpenType / TrueType (`.ttf`, `.otf`), collections (`.ttc`, `.otc`) | yes | yes (sanitised copy, subset) | own validator and table reader; FreeType for rasterising; HarfBuzz for shaping | font-helper |
| CFF, CFF2 charstrings | yes | subset only | FreeType; own bounds checks on charstring stack depth and subroutine recursion | font-helper |
| Variable-font tables (`fvar`, `gvar`, `avar`, `HVAR`, `STAT`) | yes | subset | FreeType/HarfBuzz | font-helper |
| Colour tables (`COLR`, `CPAL`, `CBDT`, `sbix`) | yes | subset keeps or drops | FreeType; PNG strikes decoded by own or the platform PNG decoder inside the helper | font-helper |
| `SVG ` table | yes | no | restricted SVG subset renderer (FUSE-WP-27), no scripts, no external references | font-helper |
| WOFF, WOFF2 | yes | WOFF2 | own container reader; Brotli decoder (MIT) is the one new library, vendored | font-helper / subset-helper |
| Type 1 (`.pfb`), bitmap (`.pcf`, `.bdf`) | view only | no | FreeType; not installable (stated) | font-helper |
| Unicode Character Database, CLDR annotations | yes (pre-compiled at build time) | no | own compiler at build time; FORGE compiles Unicode tables in (`FORGE:docs/text-locale.md`, cited from FORGE_BUILD_PLAN.md FORGE-WP-04.5) | App host, trusted bundle data |
| Library state JSON | yes | yes | own | App host |
| SVG (glyph export), PNG (copy as image) | no | yes | own writers | App host |

### 6.4 Integration

- Intents handled: `open`, `view` (font MIME types), `search` (character phrases), `pick` (a character, for apps that ask).
- Intents emitted: `print` (specimen), `open` to SY01 (font settings), `open` to SY02 ("Show in Files").
- MIME: `font/ttf`, `font/otf`, `font/collection`, `font/woff`, `font/woff2`, `application/x-font-type1`, `application/x-font-pcf`, `application/x-font-bdf`.
- Services: `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.undo-trash`, `svc.notify`, `svc.search`, `svc.audit`, `svc.thumbnail` (this bundle also supplies the font thumbnailer, which is the same helper).
- Offers `org.pbsd.fonts.inspect.v1`: validate a font handle and return metadata; used by SY02 previews, SY05 (packages containing fonts) and IN01 (if it ever allows downloaded fonts to be installed).
- Links: SY01 (defaults, fallback order, rendering), AX04 (how to type, insertion, quick picker), SY02, UT06 (specimen PDF), SE01 (the confusables view supports its phishing explanations: Proposed).

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Collections, quarantine list, caches |
| `fs.portal` | Font files the user opens; export destinations |
| `net.none` | No network. Licence and vendor URLs in fonts are shown as text only |
| `proc.helper:font-helper` | Contain parsing and rendering of hostile fonts |
| `proc.helper:subset-helper` | Contain subsetting |
| `svc.settings` | Preferences; read system font settings |
| `svc.portal.file`, `svc.intent` | Open, save, hand-offs |
| `svc.clipboard` | Copy characters and images |
| `svc.undo-trash` | Undoable removal |
| `svc.portal.print` (`dev.print`) | Specimens |
| `svc.notify` | Library check results |
| `svc.search` | Character answers in the launcher |
| `svc.audit` | Install, remove, quarantine records |
| `svc.thumbnail` | Provide the font thumbnailer |

The app does not hold `fs.path` to the font directories. The font service owns them and accepts only validated files from this app. System-wide install is an admin action authorised by an SH04 prompt.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Every app that renders text | Malicious font file | Parser or hinting-VM memory corruption once installed | Validation and sanitised re-serialisation before install; bytecode off by default; quarantine by hash; the same rules apply to all install paths through the font service |
| This app | Malicious font file being previewed | Exploit in FreeType or HarfBuzz | Helper in capability mode with two descriptors, memory and CPU limits, no GPU; the host only ever sees bitmaps and typed records |
| The user's eyes | Font that draws misleading glyphs (a font where "a" looks like "b", or blank glyphs) | Spoofed text in documents and addresses | Out of scope to prevent; the glyph grid and "fonts that have it" make it visible; system UI fonts are protected and cannot be shadowed silently (F127) |
| System availability | Font with huge tables, deep composites, endless charstrings | Resource exhaustion | Structural limits (F120), `rlimit`s (F126) |
| Text fields | Pasted text with bidi controls or confusables | User misreads | Identify panel and security view (F083, F092) |
| Privacy | Font metadata URLs, unique installed-font sets | Fingerprinting, network beacons | No network; the installed set is not exposed by this app (IN01 handles web fingerprinting) |
| Bitmap cache | Local attacker | Learn what the user previewed | Cache is in `fs.app`, cleared on demand, not backed up |

Stored: collections, favourites, recent characters, quarantine list, validation results. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `preview.text` | string | locale pangram | Default sample text |
| `preview.size_px` | u16 | 48 | Default preview size |
| `preview.waterfall_sizes` | list<u16> | 8,9,10,11,12,14,18,24,36,48,72,96 | Waterfall sizes |
| `preview.allow_fallback` | bool | false | Fill missing glyphs from other fonts |
| `preview.metrics_overlay` | bool | false | Show metric lines |
| `library.view` | enum | `list` | list, grid |
| `library.group_by_family` | bool | true | Group styles under a family |
| `install.target` | enum | `user` | user, ask |
| `install.sanitise` | bool | true | Install the re-serialised copy |
| `install.keep_unknown_tables` | bool | false | Keep tables the sanitiser does not know |
| `safety.preview_bytecode` | bool | false | Run TrueType hinting programs in previews (helper build must allow it) |
| `safety.helper_mem_mib` | u16 | 256 | Helper memory cap |
| `safety.helper_cpu_s` | u8 | 5 | Helper CPU cap per request |
| `chars.show_unassigned` | bool | false | Show unassigned code points |
| `chars.dim_missing` | bool | true | Dim characters the chosen font lacks |
| `chars.recent_count` | u16 | 64 | Size of the recent list |
| `chars.copy_on_enter` | bool | true | Enter copies |
| `chars.escape_style` | enum | `html` | Default "copy as" form |
| `search.launcher` | bool | true | Answer character queries in the launcher |

## 9. Accessibility and languages

- Every glyph and character cell has an accessible name: the Unicode name for encoded characters ("euro sign, U+20AC"), the glyph name otherwise. Invisible characters are named, never blank.
- A screen-reader user can explore a font without seeing it: coverage, features and metrics are all tables.
- Character names are English (the Unicode standard); CLDR annotations give localised keywords and short names where available, and search uses both.
- Right-to-left previews and the grid for right-to-left blocks follow the block's direction; the app chrome mirrors as usual.
- Needs the FULL text profile (G-03) for its own UI text in complex scripts; previews come from the helper regardless.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 250 ms with a cached catalogue |
| Family list with 2 000 families | Scroll at 60 fps; samples fill within 100 ms of becoming visible |
| Preview update on axis drag | Under 16 ms for a line of 60 glyphs at 48 px |
| Open an uninstalled font to first preview | 200 ms including helper start and validation, for a 1 MiB font |
| Character search | Under 30 ms per key over the full name index |
| Largest font | 64 MiB file, 65 535 glyphs per face (format limit) |
| Memory | Host 120 MiB with 2 000 families; helper capped at `safety.helper_mem_mib` |
| Library check | 500 fonts per minute |

## 11. Testing and proof

- **Unit**: code-point parsers, escapes, name search ranking, normalisation views, licence classification, `fsType` decoding, collection logic.
- **Golden**: main window both modes, font page tabs, compare, preview window, subset dialog; themes and scales. Glyph bitmaps from the helper are compared against stored references for a fixed font set.
- **Scripted UI**: T1–T12.
- **Fuzz**: validator, table reader, WOFF/WOFF2 reader, sanitiser round trip, subset-helper, on seed corpora of valid fonts plus public malformed-font collections.
- **Differential**: sanitiser output must shape and rasterise identically to the input for valid fonts (bitmap equality over a sample text set); subset output must match the original for the kept characters.
- **Sandbox**: helper tries to open files, sockets and devices; all must fail (G-06).
- **Unicode conformance**: normalisation and property lookups against the Unicode test files for the bundled version.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Font validator (C++23) | Every accepted file has in-bounds tables, offsets and counts; terminates | T2 | PRISM absence-of-runtime-error; fuzzing | A formal grammar of the accepted subset would allow T1 for the table directory |
| Table directory and `cmap` reader | Parsed result matches the bytes (round trip) | T1 | PRISM proof of parse/serialise inverse on the directory and format 4/12 `cmap` | Proof effort |
| Sanitiser | Output contains only allowed tables; output passes the validator | T2 | PRISM plus differential tests | — |
| FreeType, HarfBuzz, Brotli | None claimed | T4 | Sandbox and fuzzing | Third-party C/C++ outside the subset |
| Unicode lookups | Table lookups in bounds | T2 | PRISM | — |
| App logic | Memory-safe by language | T3 | Tests | — |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| UT03-W01 | font-helper: start, capability mode, validator, metadata records | G-06, G-07, G-08, FORGE-WP-27, FORGE-WP-28 |
| UT03-W02 | Rasterise and shape in the helper; bitmap transport over shared memory | UT03-W01, G-03 (vendored FreeType, HarfBuzz) |
| UT03-W03 | Font service client: catalogue, install per user, remove, enable, disable | FUSE-WP-17 (`UT03-GAP-2`) |
| UT03-W04 | Library views, search, preview and waterfall | G-02, UT03-W02 |
| UT03-W05 | Font file preview window with validation line; quarantine | UT03-W01 |
| UT03-W06 | Unicode data compiler and bundle data; character grid, details, search, identify | FORGE-WP-23 |
| UT03-W07 | Glyph grid; metadata view | UT03-W02 |
| UT03-W08 | Recent, favourites, copy buffer | UT03-W06 |
| UT03-W09 | Font thumbnailer for `svc.thumbnail` | UT03-W02, G-09 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| UT03-W10 | Variable axes, named instances | UT03-W02 |
| UT03-W11 | Feature inspector and toggles | UT03-W02 |
| UT03-W12 | Glyph details with outline view | `UT03-GAP-1` (FUSE-WP-23) |
| UT03-W13 | Sanitised install; system-wide install with admin prompt | SH04 |
| UT03-W14 | Collections, filters, favourites | UT03-W04 |
| UT03-W15 | Duplicates, broken-font report, library check, audit | UT03-W01 |
| UT03-W16 | Compare window (side by side, overlay) | UT03-W02 |
| UT03-W17 | Coverage summary, fonts-that-have-it, missing lists | UT03-W06 |
| UT03-W18 | Licence and embedding view; colour fonts | UT03-W07, FUSE-WP-27 |
| UT03-W19 | How-to-type and escapes | AX04 |
| UT03-W20 | Specimen printing | G-16, UT06 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| UT03-W21 | subset-helper and export dialog; WOFF2 writer | UT03-W01 |
| UT03-W22 | Shaping trace, kerning view, feature samples, alternates | UT03-W11 |
| UT03-W23 | Metrics, coverage and version compare | UT03-W16 |
| UT03-W24 | Smart collections, session activation, fallback chain view | SY01 |
| UT03-W25 | Launcher answers, insertion into the last field | SH01, AX04 |

## 13. Open questions and risks

- The font service (discovery, per-user install, enable and disable, validation gate) is assumed from FUSE-WP-17's title. Its interface was not read in detail; if it has no management calls this app needs them added (`UT03-GAP-2`).
- FreeType and HarfBuzz are large third-party parsers. Containing them is the plan; replacing the outline and `cmap` paths with own proved code is a later option.
- Whether the system text stack runs TrueType bytecode at all is a platform decision (SY01, FUSE-WP-16). This spec assumes "no" by default.
- A sanitised copy changes the file hash, which breaks vendor signatures on fonts. The original hash is recorded; whether to keep originals is open.
- Brotli (for WOFF2) is one more vendored library; WOFF2 could be dropped to avoid it.
- Unicode and CLDR data licences (Unicode Licence v3) must be recorded in the SBOM.
- Character insertion into another app's field (F095) needs an AX04 interface that is not specified yet.
