<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Text Editor

By Odin Loch

| | |
|---|---|
| ID | `OF01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.text-editor` |
| Category | office |
| Priority | P0 |
| Backing repos | FUSE (`codeEdit` widget today; code editor core FUSE-WP-37 Proposed), FORGE (`forge::os::text`: Unicode, encodings, regex) |
| Status | Planned. Nothing built. |

## 1. Purpose

The everyday editor for plain text, configuration files, logs, Markdown and source code. It opens any file fast, including files of several gigabytes, never loses unsaved text, and never touches the network. It replaces Kate/KWrite from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Editing text in any supported encoding, with tabs, splits, multi-cursor, column mode, folding, minimap.
- Syntax highlighting from declarative grammar files. No language servers.
- Find and replace with regex in one buffer, all open buffers, and a folder the user granted.
- Huge files through a piece table with lazy, memory-mapped loading.
- Session restore and persistence of unsaved buffers.
- Snippets, macros in ForgeScript, spell check, Markdown preview, two-way diff view.
- A read-only log-follow mode.

Out of scope:

| Item | Owner |
|---|---|
| Projects, language servers, debugger, build, tests | DV01 IDE |
| Binary editing (this app detects binary data and hands off) | DV04 Hex Editor |
| Git staging, history, three-way merge | DV03 Version Control |
| Rich notes, backlinks, notebooks | OF02 Notes |
| Styled documents, page layout | OF04 Writer |
| Terminal panel | SY03 Terminal |
| File management beyond open/save/rename of the current file | SY02 Files |
| CSV as a grid | OF05 Sheets, OF09 Data Studio |

## 3. Users and top tasks

Personas:

- **Admin**: edits configuration files under `/etc` and `/usr/local/etc`, reads logs, needs encoding and line-ending certainty.
- **Writer of plain text**: Markdown, notes, lists; wants spell check, word count, preview.
- **Developer**: quick edits outside the IDE; wants multi-cursor, regex, grammar highlighting, diff.
- **Analyst**: opens multi-gigabyte logs and dumps, searches them, extracts lines.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a file from Files and start typing | 1 (double click) |
| T2 | Find a string in the current file | 2 (`Ctrl+F`, type) |
| T3 | Replace a regex across a folder with a preview | 5 |
| T4 | Open a 4 GB log and jump to the last line | 2 (open, `Ctrl+End`) |
| T5 | Convert a file from CRLF/windows-1252 to LF/UTF-8 | 2 clicks in the status bar each |
| T6 | Edit 20 similar lines at once with multi-cursor | 3 |
| T7 | Recover unsaved text after a power cut | 0 (restored on next start) |
| T8 | Compare two files side by side | 3 |
| T9 | Run any command without the mouse | 2 (`Ctrl+K`, type) |
| T10 | Edit a root-owned config file | 3 (open, edit, save with one shell prompt) |
| T11 | Insert a snippet | 2 (prefix, `Tab`) |
| T12 | Record and replay a macro | 3 |
| T13 | Preview Markdown beside the source | 1 (`Ctrl+Shift+V`) |
| T14 | Send a file that turned out to be binary to the Hex Editor | 1 click on the banner |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Files and buffers

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F001 | New buffer | `Ctrl+N` opens "Untitled n" with the default encoding (UTF-8), line ending (LF) and language (Plain). | M | 1 |
| OF01-F002 | Open | `Ctrl+O` opens the file portal (SH04). Multi-select opens each file in a tab. The app receives handles, never paths it can re-open by itself. | M | 1 |
| OF01-F003 | Open by intent | Handles `open`, `edit`, `view` for `text/*` and listed types; a `line:column` argument places the caret. | M | 1 |
| OF01-F004 | Open recent | List of the last 50 files with path, time and a "missing" flag; reopening uses a remembered portal grant if the user allowed "always". | M | 1 |
| OF01-F005 | Reopen closed tab | `Ctrl+Shift+T` restores the last closed buffer with its undo history (kept for 20 closed tabs). | S | 1 |
| OF01-F006 | Save | `Ctrl+S`. Atomic: write to a sibling temporary file, `fsync`, rename over the target. Falls back to in-place write with a warning when the directory handle is read-only. | M | 1 |
| OF01-F007 | Save as, save copy | `Ctrl+Shift+S` through the save portal; "Save a Copy" keeps the buffer bound to the old file. | M | 1 |
| OF01-F008 | Save all | Saves every modified buffer that has a file; lists untitled ones. | M | 1 |
| OF01-F009 | Save with elevated rights | When the handle is read-only because of ownership, "Save as administrator" asks the shell (SH03 prompt) for a one-shot write handle to that one file. The editor never runs as root. | S | 2 |
| OF01-F010 | Revert | Discards changes and reloads from disk; the discarded text goes to local history (F017). | M | 1 |
| OF01-F011 | External change detection | Watches the open handle (kqueue `EVFILT_VNODE`). Unmodified buffers reload silently; modified ones show a banner: Reload, Keep mine, Compare. | M | 1 |
| OF01-F012 | Deleted or moved file | Banner "File was removed"; buffer stays and is marked unsaved. | M | 1 |
| OF01-F013 | Rename and move | `F2` on the tab renames the file through the portal; "Move to..." picks a folder. | S | 2 |
| OF01-F014 | Close with unsaved changes | No dialog by default: the buffer is persisted (F190) and returns on next start. An optional setting restores the classic Save/Discard/Cancel dialog. | M | 1 |
| OF01-F015 | Read-only mode | Per-buffer lock toggle; set automatically for read-only handles, files in snapshots and log-follow mode. | M | 1 |
| OF01-F016 | Binary detection | First 64 KiB scanned: NUL bytes or over 10% invalid sequences in every candidate encoding marks the file as binary. Banner offers "Open in Hex Editor" (intent to DV04), "Open anyway as Latin-1 (read-only)". | M | 1 |
| OF01-F017 | Local history | Every save and every revert stores a compressed copy (zstd) in the app data store, keyed by file identity. Limits: 50 versions or 30 days per file, 1 GiB total. | S | 2 |
| OF01-F018 | Snapshot versions | "Earlier versions" lists ZFS snapshots that contain the file (through SY02's snapshot interface) and opens one read-only or in the diff view. | S | 2 |
| OF01-F019 | Open from standard input | `text-editor -` reads a pipe into an untitled buffer; `--wait` blocks the caller until the tab closes (for `$EDITOR`). | S | 1 |
| OF01-F020 | Open a folder | A granted folder appears in the sidebar file tree (F158). Not a project system. | S | 2 |
| OF01-F021 | New from template | Templates in `~/.config/org.pbsd.text-editor/templates/`; shipped: shell script, Markdown, ForgeScript module, rc.conf fragment. | C | 3 |
| OF01-F022 | File information | Path, size, owner, mode, times, encoding, BOM, line endings, line count, provenance tag (where the file came from, if SY02 recorded it). | S | 2 |
| OF01-F023 | Disposable open | Files with an "untrusted" provenance tag open read-only with highlighting helper disabled until the user clicks "Trust for editing". | S | 2 |

### 4.2 Editing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F030 | Undo and redo | Unlimited per buffer, grouped by typing bursts (500 ms gap or word boundary). Survives save. Persisted with unsaved buffers. Memory cap 256 MiB per buffer, oldest steps drop first with a notice. | M | 1 |
| OF01-F031 | Selective undo in selection | "Undo in Selection" reverts only edits inside the selected range. | C | 3 |
| OF01-F032 | Clipboard | Cut, copy, paste through `svc.clipboard`. With no selection, copy and cut act on the whole line. | M | 1 |
| OF01-F033 | Paste from history | `Ctrl+Shift+V` in an editor opens the last 20 clipboard items (AX05 owns the history; this is a picker). When Markdown preview has focus the shortcut is unused. | S | 2 |
| OF01-F034 | Paste with indent fix | Pasted blocks are re-indented to the caret's level; toggle per language. | S | 2 |
| OF01-F035 | Line operations | Duplicate (`Ctrl+D`), delete (`Ctrl+Shift+K`), move up/down (`Alt+Up/Down`), join (`Ctrl+J`), split, insert above/below (`Ctrl+Shift+Enter`, `Ctrl+Enter`). | M | 1 |
| OF01-F036 | Sort lines | Ascending, descending, numeric, natural, case-insensitive, by locale collation (`forge_text` collator), unique, shuffle, reverse. | S | 2 |
| OF01-F037 | Case conversion | Upper, lower, title, sentence, swap, `snake_case`, `camelCase`, `kebab-case`. Uses full Unicode case mapping. | S | 1 |
| OF01-F038 | Indent and outdent | `Tab` / `Shift+Tab` on a selection; `Ctrl+]` / `Ctrl+[` anywhere. | M | 1 |
| OF01-F039 | Auto-indent | Keeps the previous line's indent; grammar rules can increase or decrease after patterns (`{`, `then`, `:`). | M | 1 |
| OF01-F040 | Tabs or spaces | Per buffer: indent with tabs or N spaces, tab display width 1 to 16. Detected from the file on open (most common leading whitespace in the first 2000 lines). | M | 1 |
| OF01-F041 | Convert indentation | Tabs to spaces, spaces to tabs, re-indent to a new width. | S | 2 |
| OF01-F042 | Comment toggle | `Ctrl+/` line comment, `Ctrl+Shift+/` block comment, tokens from the grammar. | M | 1 |
| OF01-F043 | Bracket matching | Highlights the pair; `Ctrl+Shift+\` jumps; "Select to Bracket". Pairs from the grammar. | M | 1 |
| OF01-F044 | Auto-close pairs | Brackets and quotes; typing over the closer; wrap selection when a pair opener is typed. Per language, off for Plain. | S | 1 |
| OF01-F045 | Smart Home and End | Home toggles between first non-blank and column 0; End between last non-blank and line end. | S | 1 |
| OF01-F046 | Word and subword movement | `Ctrl+Left/Right` by UAX 29 word; `Ctrl+Alt+Left/Right` by camelCase or underscore part. | M | 1 |
| OF01-F047 | Expand and shrink selection | `Alt+Shift+Right/Left`: word, string, bracket pair, line, fold block, paragraph, all. | S | 2 |
| OF01-F048 | Overwrite mode | `Insert` toggles; block caret; shown in the status bar. | S | 1 |
| OF01-F049 | Trim trailing whitespace | Command and on-save option; "only lines I changed" mode. | M | 1 |
| OF01-F050 | Final newline | Ensure one on save (option); status bar warns when missing. | M | 1 |
| OF01-F051 | Insert special | Date and time (ISO 8601 or locale), file name, path, UUID v4, random bytes as hex, Unicode by code point (`Ctrl+Shift+U`), character picker hand-off to AX04. | S | 2 |
| OF01-F052 | Transform selection | Base64, URL, HTML entity, JSON string encode and decode; hex to bytes text; hash of the selection (SHA-256, BLAKE3) to the clipboard. | S | 2 |
| OF01-F053 | Format structured text | Pretty-print and minify JSON; re-indent XML. Own code in ForgeScript; refuses input over 64 MiB. | S | 2 |
| OF01-F054 | Reflow paragraph | Hard-wrap the paragraph or selection at the ruler column (`Alt+Q`), keeping comment prefixes and list markers. | S | 2 |
| OF01-F055 | Align | Align selected lines on a typed delimiter (`=`, `:`, `|`) or regex. | C | 3 |
| OF01-F056 | Increment and decrement | `Ctrl+Alt+Up/Down` on numbers (decimal, hex, binary), dates and `true`/`false` under each caret. | C | 3 |
| OF01-F057 | Number sequence | With several carets or a column selection: insert a sequence with start, step, padding and base. | S | 2 |
| OF01-F058 | Drag selected text | Drag moves, `Ctrl`+drag copies, within and between splits and windows. | S | 2 |
| OF01-F059 | Bookmarks | `Ctrl+F2` toggle, `F2`/`Shift+F2` (when a text view has focus) next and previous; kept per file in the session store. | S | 2 |
| OF01-F060 | Go to line | `Ctrl+L`: `line`, `line:column`, `+n`, `-n`, `n%`, byte offset `@n`. | M | 1 |
| OF01-F061 | Go to symbol | `Ctrl+Shift+O`: headings and symbols found by the grammar's symbol rules (regex based, no semantic analysis). | S | 2 |
| OF01-F062 | Navigation history | `Alt+Left/Right` steps through caret jumps across buffers (last 100). | S | 2 |
| OF01-F063 | Invisible and confusable characters | Highlights zero-width, bidirectional control (U+202A to U+202E, U+2066 to U+2069) and non-ASCII homoglyph characters in files whose grammar marks them as code. Command "Remove invisible characters". | M | 1 |

### 4.3 Multi-cursor and column mode

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F070 | Add caret by click | `Ctrl+Alt+click` adds a caret; `Ctrl+Alt+click` on a caret removes it. | M | 1 |
| OF01-F071 | Add caret above and below | `Ctrl+Alt+Shift+Up/Down`. | M | 1 |
| OF01-F072 | Select next occurrence | `Ctrl+Shift+D` adds the next match of the selection; `Ctrl+Shift+L` selects all matches; skip with `Ctrl+Shift+;`. | M | 1 |
| OF01-F073 | Carets at line ends | `Alt+Shift+I` puts a caret at the end of each selected line. | S | 1 |
| OF01-F074 | Split selection | Split a selection into one per line, or by regex. | S | 2 |
| OF01-F075 | Column selection | `Alt+drag` or `Alt+Shift+arrows` selects a rectangle; virtual space past line ends is allowed while in column mode. | M | 1 |
| OF01-F076 | Column mode toggle | `Alt+C` makes every selection rectangular until toggled off; shown in the status bar as `COL`. | S | 2 |
| OF01-F077 | Column paste | Pasting N lines onto N carets puts one line at each; a rectangular clipboard pastes as a block. | M | 1 |
| OF01-F078 | Caret limits | Up to 10 000 carets; above that the command becomes a single batched replace with one undo step. | M | 1 |
| OF01-F079 | Undo caret | `Ctrl+U` undoes the last caret addition without undoing text. | S | 2 |
| OF01-F080 | Per-caret clipboard | Copy with N carets and paste with N carets maps one to one. | S | 2 |

### 4.4 Find and replace

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F090 | Find bar | `Ctrl+F`; incremental; options: match case, whole word, regex, in selection, wrap, preserve case. Match count "3 of 41". | M | 1 |
| OF01-F091 | Replace bar | `Ctrl+H`; Replace, Replace All (one undo step), Replace in Selection; capture groups `$1`, `${name}`; case modifiers `\u \l \U \L \E`. | M | 1 |
| OF01-F092 | Regex engine | `forge_text` regex (Verified: `FORGE:docs/text-locale.md`, Unicode classes, named groups, lookahead, backreferences, step limit 1e7). A search that hits the step limit stops with a clear message, never hangs. | M | 1 |
| OF01-F093 | Multi-line search | The pattern field grows to several lines; `\n` matches any line ending in the buffer. | S | 1 |
| OF01-F094 | Highlight all matches | In the text, on the scroll bar and in the minimap. | M | 1 |
| OF01-F095 | Find next, previous, selected | `Ctrl+G`, `Ctrl+Shift+G`, `Ctrl+F3` (find the word under the caret). | M | 1 |
| OF01-F096 | Search history | Last 50 patterns and replacements, in the app store; excluded for private windows (F214). | S | 2 |
| OF01-F097 | Find in open buffers | Scope selector: current, all tabs, this window. | S | 2 |
| OF01-F098 | Find in folder | `Ctrl+Shift+F`. Scope: a folder the user granted. Include and exclude globs, honour `.gitignore`, skip binary, size limit per file (default 64 MiB), follow no symlinks out of the grant. Runs in the search helper. | M | 2 |
| OF01-F099 | Results panel | Tree of file, line, match with context lines; streaming results; cancel; keep several result sets as tabs. | M | 2 |
| OF01-F100 | Replace in folder with preview | Shows each change as a diff row with a check box; "Replace n in m files" writes atomically per file; one cross-file undo that restores all files from local history. | M | 2 |
| OF01-F101 | Search in huge files | Streaming search over the piece table without loading the file; progress and match list fill as it runs; first match in a 4 GiB file shown within the budget in section 10. | M | 2 |
| OF01-F102 | Filter lines | "Keep lines matching", "Delete lines matching", "Copy matching lines to a new buffer". | S | 2 |
| OF01-F103 | Saved searches | Name a pattern with its options and scope; listed in the command palette. | C | 3 |
| OF01-F104 | Quick find | Typing with a selection and pressing `Ctrl+F` seeds the field; `Esc` returns the caret to where it started. | M | 1 |

### 4.5 Huge files

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F110 | Piece table storage | The buffer is a piece tree (balanced tree of pieces over the original file and an append-only add buffer). Edits cost O(log n) in pieces, not in file size. | M | 1 |
| OF01-F111 | Lazy load | The original file is memory-mapped read-only through its handle. Pages are touched only when shown, searched or saved. First screen is shown before the line index is complete. | M | 1 |
| OF01-F112 | Background line index | A worker records the offset of every 1024th line break; line numbers show "~" until the scan passes the visible range. | M | 1 |
| OF01-F113 | Size limits | Supported: 16 GiB or 2^31 lines per buffer. Above that the file opens read-only in paged view with search only. | M | 2 |
| OF01-F114 | Long lines | A line over 10 000 graphemes is chunked for layout; soft wrap applies per chunk; highlighting stops at column 20 000 with a marker. No operation is quadratic in line length. | M | 1 |
| OF01-F115 | Large-file mode | Above 64 MiB (setting): minimap, spell check, folding and "highlight all occurrences" turn off, highlighting only for the visible range; a status-bar chip says so and lets the user turn each back on. | M | 1 |
| OF01-F116 | Transcoding huge files | Non-UTF-8 files are decoded in 4 MiB windows on demand; the piece table stores byte ranges plus the decoder state at window starts. | S | 2 |
| OF01-F117 | Save huge files | Streams pieces to a temporary file in 8 MiB blocks with progress and cancel; needs free space equal to the new size; in-place save offered only for append-only edits. | M | 2 |
| OF01-F118 | Log follow | `View > Follow` (tail): read-only, appended bytes extend the piece table, view sticks to the end until the user scrolls up; handles truncation and rotation. | S | 2 |
| OF01-F119 | Original file changed under the map | If the mapped file shrinks or changes, the buffer is frozen read-only and the user is told; unsaved edits are kept in the add buffer and can be saved as a patch against a fresh copy. | M | 2 |

### 4.6 Encodings and line endings

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F120 | Encoding detection | Order: BOM, strict UTF-8, BOM-less UTF-16, byte statistics (Verified facility: `sniff_charset` in `FORGE:docs/text-locale.md`), then a modeline or `.editorconfig` `charset`. Confidence shown; low confidence shows a banner with choices. | M | 1 |
| OF01-F121 | Supported encodings | Every charset `forge_text` lists: UTF-8, UTF-16LE/BE, UTF-32LE/BE, US-ASCII, ISO-8859 family, windows-125x, KOI8-R/U, IBM437/850/866, macintosh, Shift_JIS, EUC-JP, ISO-2022-JP, GBK, GB18030, Big5, EUC-KR. | M | 1 |
| OF01-F122 | Reopen with encoding | Reinterprets the same bytes; no change to the file. | M | 1 |
| OF01-F123 | Convert encoding | Changes the save encoding. Characters that cannot be encoded are listed with line numbers before saving; choices: cancel, replace with `?`, save as UTF-8. | M | 1 |
| OF01-F124 | BOM control | Add or remove; default none for UTF-8, present for UTF-16/32. | M | 1 |
| OF01-F125 | Invalid bytes are preserved | Undecodable bytes are shown as `\xNN` chips and are written back unchanged unless edited. A file opened and saved without edits is byte-identical. | M | 1 |
| OF01-F126 | Line endings | LF, CRLF, CR detected per file; convert from the status bar. | M | 1 |
| OF01-F127 | Mixed line endings | Detected and kept per line; a status chip "Mixed" opens a dialog: normalise to LF/CRLF/CR or keep. Visible markers when "Show Line Endings" is on. | M | 1 |
| OF01-F128 | Unicode normalisation | Commands: to NFC, NFD, NFKC, NFKD on the selection or buffer; a status chip when the file is not NFC (off by default). | C | 3 |
| OF01-F129 | Show whitespace | None, boundary, selection, all; tabs as arrows, spaces as dots, NBSP and other spaces with distinct marks. | M | 1 |

### 4.7 View and layout

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F140 | Tabs | Reorder by drag, pin, close others, close to the right, preview tab (italic, replaced by the next single click in the file tree), modified dot, read-only lock, tab overflow list. | M | 1 |
| OF01-F141 | Splits | Split right `Ctrl+\`, split down `Ctrl+Shift+-`, up to 8 panes per window, drag a tab to an edge, equalise, focus pane `Ctrl+1..8`. | M | 1 |
| OF01-F142 | Same buffer in two panes | One buffer, several views with independent caret, scroll, wrap and folds. | M | 1 |
| OF01-F143 | Detach tab to a window | Drag a tab out; windows share the buffer set and session. | S | 2 |
| OF01-F144 | Soft wrap | Off, window width, ruler column; wrap indent (same, +1, +2); wrap at word boundaries by UAX 14. | M | 1 |
| OF01-F145 | Line numbers | Absolute, relative, interval, off; current line emphasised. | M | 1 |
| OF01-F146 | Gutter | Line numbers, fold arrows, bookmarks, change bars (added, modified, deleted since last save), spell and diff marks. | M | 1 |
| OF01-F147 | Rulers | Any number of column rulers; from settings, `.editorconfig` `max_line_length` or modeline. | S | 1 |
| OF01-F148 | Indent guides | Vertical guides per indent level; active scope highlighted. | S | 2 |
| OF01-F149 | Current line and scope highlight | Line background; matching brackets; other occurrences of the selected word. | M | 1 |
| OF01-F150 | Minimap | Right-edge overview drawn from token colours at 1 or 2 px per line; viewport slider; click and drag to scroll; marks for search hits, bookmarks, changes, selection. Hidden under 640 px width and in large-file mode. | S | 2 |
| OF01-F151 | Scroll bar marks | Same marks as the minimap when the minimap is off. | S | 2 |
| OF01-F152 | Code folding | By indentation, by grammar fold markers (`{}`, region comments, Markdown headings). Fold, unfold, fold all, fold level 1 to 9, fold recursively; folded text shows a `…` chip with the hidden line count; folds persist per file. | M | 2 |
| OF01-F153 | Sticky scope lines | The enclosing fold headers stay pinned at the top of the pane (up to 5 lines). | C | 3 |
| OF01-F154 | Zoom | `Ctrl++ / - / 0`, per window, 50% to 400%. | M | 1 |
| OF01-F155 | Fonts | Monospace by default; proportional allowed; ligatures on or off (needs FULL text, G-03); line height 1.0 to 2.0; letter spacing. | M | 1 |
| OF01-F156 | Themes | Follows the system theme tokens; syntax colour schemes as CSS files (token class to colour); high-contrast scheme shipped. | M | 1 |
| OF01-F157 | Focus mode | `Ctrl+Shift+F11`: hides tabs, sidebar, status bar; centres a fixed-width column; optional typewriter scrolling. | C | 3 |
| OF01-F158 | Sidebar | Views: Open files, Folder tree (granted folders only), Outline (symbols), Search results, Snippets, History. `F9` collapses. | S | 2 |
| OF01-F159 | Status bar | Caret line:column, selection size (characters, lines, carets), language, encoding, line ending, indent, zoom, spell language, read-only lock, large-file chip, background jobs. Each chip is a button. | M | 1 |
| OF01-F160 | Word count | Words, characters with and without spaces, graphemes, lines, bytes for the buffer and the selection; reading time for prose languages. | S | 2 |
| OF01-F161 | Scroll past end, smooth scroll | Options; smooth scroll honours reduced motion. | S | 2 |
| OF01-F162 | Bidirectional text | UAX 9 layout per line; caret movement visual or logical (setting); needs G-03. | S | 2 |

### 4.8 Syntax highlighting and grammars

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F170 | Grammar format | Declarative `.fusegrammar` files (Proposed in FUSE-WP-37): JSON with `version`, `name`, `scopes`, `file_patterns`, `first_line`, `states` (regex rules with push, pop, set), `comments`, `brackets`, `indent`, `folds`, `symbols`, `embed`. No code in a grammar. | M | 1 |
| OF01-F171 | Linear-time matcher | Grammar regexes run on the linear-time matcher (no back-references, no look-behind). A grammar that needs them is rejected at load with the rule named. | M | 1 |
| OF01-F172 | Incremental tokenising | Each line caches its end state; re-tokenising stops when the state converges; the tail runs in 2 ms slices off the key path. | M | 1 |
| OF01-F173 | Shipped grammars, phase 1 | Plain, ForgeScript, C, C++, shell (sh, csh), Makefile, CMake, JSON, YAML, TOML, INI and `rc.conf`, `pf.conf`, `.fuseui`, CSS, Markdown, XML, HTML, diff/patch, log (timestamps, levels), SQL, Python, Lua, Rust, Go, JavaScript, MathScript, man page (mdoc), Git commit and rebase. | M | 1 |
| OF01-F174 | Shipped grammars, phase 2 | TeX, Java, Kotlin, Swift, Perl, Ruby, PHP, Awk, sed, Assembly (amd64, aarch64), Dockerfile, Nix, Protocol Buffers, GraphQL, reStructuredText, AsciiDoc, CSV (rainbow columns), `.desktop`, systemd units, crontab, hosts, fstab, loader.conf, sysctl.conf, DTrace, PRISM annotations. | S | 2 |
| OF01-F175 | Language detection | Order: explicit choice, modeline, file name pattern, first-line pattern (`#!`, `<?xml`), content heuristics among candidates. Shown and changeable in the status bar. | M | 1 |
| OF01-F176 | Embedded languages | A grammar may embed another by scope (code fences in Markdown, CSS in HTML, SQL in strings when marked). Depth limit 4. | S | 2 |
| OF01-F177 | Token classes | Fixed class list: `comment`, `string`, `number`, `keyword`, `type`, `function`, `variable`, `constant`, `operator`, `punctuation`, `tag`, `attribute`, `heading`, `emphasis`, `strong`, `link`, `inserted`, `deleted`, `changed`, `invalid`. Schemes style these CSS classes (F156). | M | 1 |
| OF01-F178 | User grammars | Drop a file in `~/.config/org.pbsd.text-editor/grammars/`. Loaded as data, validated against the schema, size limit 1 MiB, rule limit 5000. A bad grammar shows the error and falls back to Plain. | S | 2 |
| OF01-F179 | Grammar import | Convert TextMate `.tmLanguage.json` and Sublime `.sublime-syntax` (YAML subset) to `.fusegrammar`, reporting rules that need features the matcher lacks. Runs in the search helper sandbox. | C | 3 |
| OF01-F180 | Colour schemes | Shipped: Default Dark, Default Light, High Contrast Dark, High Contrast Light, Solarised Dark and Light, Monochrome (bold and italic only, for colour-blind use). User schemes are CSS in `~/.config/.../schemes/`. | M | 1 |
| OF01-F181 | Scope inspector | `Tools > Inspect Scope` shows the token class, grammar state stack and scheme rule under the caret. | C | 3 |
| OF01-F182 | Rainbow brackets | Optional: nesting depth cycles six colours; unmatched closers use `invalid`. | C | 3 |
| OF01-F183 | Colour swatches | In CSS-like grammars a 10 px swatch precedes colour literals; click opens the portal colour picker (SH04) and rewrites the literal. | C | 3 |
| OF01-F184 | Link detection | URLs and file paths are underlined on `Ctrl` hover; `Ctrl+click` emits an `open` intent after showing the full target. No fetch by the editor. | S | 2 |

### 4.9 Session and recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F190 | Unsaved-buffer persistence | Every modified or untitled buffer is journalled to the session store: edit operations are appended within 1 s of the keystroke or at 4 KiB, with `fsync` every 5 s (setting). Closing the window or the app never asks; the text is back on next start. | M | 1 |
| OF01-F191 | Session restore | Windows, tabs, splits, carets, selections, scroll, folds, bookmarks, wrap, zoom, language and encoding overrides return. Files are reopened through remembered portal grants; a file whose grant is gone shows "Grant needed" with a button. | M | 1 |
| OF01-F192 | Crash recovery | After an unclean exit the journal is replayed onto the last known file state. If the file on disk changed meanwhile, the buffer opens as a conflict with Compare. | M | 1 |
| OF01-F193 | Journal integrity | Each record has a BLAKE3 chain hash; a torn tail is cut at the last good record and reported. | M | 1 |
| OF01-F194 | Named sessions | Save, switch, rename and delete sessions (sets of windows and tabs). `--session <name>` on the command line. | S | 2 |
| OF01-F195 | Persistent undo | Undo history is stored with the session per file identity (device, inode, size, mtime, BLAKE3 of first and last 64 KiB) and reattached when the file is unchanged. Cap 64 MiB per file. | S | 2 |
| OF01-F196 | Scratch buffers | Untitled buffers are kept for ever until closed with "Discard"; listed under `File > Scratch Buffers` with first line and age. | M | 1 |
| OF01-F197 | Discarded text bin | Text closed with "Discard" stays 30 days in the app store and can be restored. | S | 2 |
| OF01-F198 | Encrypted session store | When the home dataset is not encrypted, the session store can be sealed with a key from `svc.keyring` (XChaCha20-Poly1305 per record). Off by default on an encrypted dataset. | S | 2 |
| OF01-F199 | Session export and wipe | "Forget this file" removes its history, undo, bookmarks and journal. "Clear all editor history" wipes the stores after one confirmation. | M | 1 |

### 4.10 Snippets and macros

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F200 | Snippet expansion | Type a prefix, press `Tab`. Tab stops `$1..$9`, final `$0`, placeholders `${1:text}`, choices `${1|a,b|}`, mirrors, variables (`$SELECTION`, `$FILENAME`, `$DATE`, `$CLIPBOARD`, `$UUID`), regex transforms. | S | 2 |
| OF01-F201 | Snippet files | JSON per language in `~/.config/.../snippets/<language>.json`; fields `prefix`, `body`, `description`, `scope`. Shipped sets for the phase 1 grammars. | S | 2 |
| OF01-F202 | Snippet picker and editor | `Ctrl+Alt+J` lists snippets for the language; the sidebar Snippets view creates, edits, duplicates and deletes them. | S | 2 |
| OF01-F203 | Surround with snippet | A selection becomes `$SELECTION` of the chosen snippet. | C | 3 |
| OF01-F204 | Import snippets | Reads VS Code and TextMate snippet files as data. | C | 3 |
| OF01-F205 | Macro record and play | `Ctrl+Shift+R` starts and stops; `Ctrl+Shift+E` plays the last macro; "Play n times" and "Play to end of file". Recording captures editor actions, not raw keys. | S | 2 |
| OF01-F206 | Macros are ForgeScript | A recording is saved as a readable ForgeScript function calling the `editor::` API (`insert`, `move`, `select`, `find`, `replace`, `each_caret`, `lines`). Users may edit it or write macros by hand. | S | 2 |
| OF01-F207 | Macro sandbox | A macro engine instance has an empty `Caps` set: no `fs`, `net`, `proc`, `env`, `reflect`. It sees only the `editor::` bindings for the buffer it was run on. A step and time budget (default 5 s, 256 MiB) stops runaway macros; all its edits are one undo step. | M | 2 |
| OF01-F208 | Macro signing | User macros are signed with the user's script key from SE04 when saved (Ed25519 over BLAKE3, detached `.sig`, Verified mechanism in `FORGE:docs/scripting.md`). A macro whose file changed outside the editor is shown as "modified, unsigned" and needs a click to re-trust. | M | 2 |
| OF01-F209 | Macro manager | List, rename, bind to a shortcut, edit, export, import (import shows the source and its API calls before trusting). | S | 2 |
| OF01-F210 | Macro on event | Optional per-language hooks `on_save`, `on_open` bound to a trusted macro; off by default; shown as a chip in the status bar when active. | C | 3 |
| OF01-F211 | Filter through command | Not offered: the editor has no `proc.exec`. The menu item opens SY03 Terminal with the selection as a temporary file handle instead. | S | 3 |
| OF01-F212 | Repeat last action | `Ctrl+.` repeats the last edit command at the carets. | C | 3 |

### 4.11 Privacy and security

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F214 | Private window | `Ctrl+Shift+N`: no session journal on disk (kept in locked memory), no recent files, no search history, clipboard copies flagged sensitive. Marked with a `--secret` coloured header chip. | S | 2 |
| OF01-F215 | No network | The manifest declares `net.none`. There is no update check, no extension gallery, no remote grammar or dictionary download, no crash upload. The header never shows `[net]`. | M | 1 |
| OF01-F216 | Per-file capability view | `File > Access` lists each open handle with rights (read, write), how it was granted (portal, intent, remembered) and a Revoke button that closes the handle. | S | 2 |
| OF01-F217 | Secret detection on copy | Text matching key patterns (PEM blocks, `AKIA…`, 32+ hex with "key" or "token" nearby) is copied with the sensitive flag so `svc.clipboard` expires it. | S | 2 |
| OF01-F218 | Trojan-source warning | A banner on open when a code file holds bidirectional controls or mixed-script identifiers; "Show" reveals them (F063). | M | 1 |
| OF01-F219 | Modeline safety | Only a fixed list of keys is honoured from modelines and `.editorconfig` (F262); nothing in a file can run code or change security settings. | M | 1 |
| OF01-F220 | Provenance chip | Files tagged by SY02 or IN03 as downloaded show "From: <origin>" in the status bar; the tag is kept on save. | S | 2 |
| OF01-F221 | Audit records | Elevated saves, folder grants, macro trust decisions and revocations are written to `svc.audit`. File contents and names of ordinary edits are not. | M | 2 |
| OF01-F222 | Redact for sharing | "Copy with secrets masked" replaces detected secrets and e-mail addresses with `█` runs in the copied text only. | C | 3 |
| OF01-F223 | Swap and dump hygiene | Private-window buffers and the keyring key sit in `secure_buffer` pages (Verified facility: `FORGE:docs/security-api.md`), excluded from core dumps. | S | 2 |

### 4.12 Diff view

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F230 | Compare two files or buffers | `Tools > Compare...`: with another tab, a file, the saved version, the clipboard, a local-history version or a snapshot version. | M | 2 |
| OF01-F231 | Side-by-side and unified | Toggle; synchronised scrolling with filler lines; overview ruler. Uses `diffView` (FUSE-WP-38). | M | 2 |
| OF01-F232 | Intra-line diff | Word and character changes inside changed lines. | M | 2 |
| OF01-F233 | Options | Ignore whitespace (trailing, amount, all), case, line endings, blank lines; algorithm: O(ND) or histogram. | S | 2 |
| OF01-F234 | Navigate and apply | `F7` / `Shift+F7` next and previous change; copy hunk left or right; revert hunk; both sides editable when writable. | M | 2 |
| OF01-F235 | Unsaved-changes gutter | Change bars (F146) open an inline peek of the original text with Revert. | S | 2 |
| OF01-F236 | Export patch | Save the comparison as a unified diff (`text/x-diff`); "Apply patch..." applies a unified diff to the current buffer with a preview and rejects listed. | S | 3 |
| OF01-F237 | Large diff bound | Above 200 000 lines per side or 2 s the engine falls back to a flagged heuristic and says so. | M | 2 |
| OF01-F238 | Three-way merge | Out of scope here: conflict markers are highlighted and "Open in Version Control" hands the file to DV03. | S | 3 |

### 4.13 Writing aids

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F240 | Spell check | Through the spelling service (G-17). In prose grammars all text is checked; in code grammars only `comment` and `string` tokens. Squiggles through the decoration API. | S | 2 |
| OF01-F241 | Suggestions | Context menu and `Alt+Enter`: up to 8 suggestions, Add to dictionary, Ignore in this file, Ignore always. | S | 2 |
| OF01-F242 | Languages | Per buffer, several at once; set from the status bar or a modeline (`spell=en_AU,de`). Automatic detection among enabled dictionaries per paragraph (option). | S | 2 |
| OF01-F243 | Markdown preview | `Ctrl+Shift+V` opens a preview tab; `Ctrl+K V` opens it to the side. CommonMark plus tables, task lists, strike-through, footnotes (`markdownView` after FUSE-WP-42). Raw HTML is shown as text. | S | 2 |
| OF01-F244 | Preview sync | Scroll sync both ways; double click in the preview moves the caret to the source line. | S | 2 |
| OF01-F245 | Preview images | Local images load only from the same granted folder; remote images are never fetched, shown as a placeholder with the URL. | M | 2 |
| OF01-F246 | Markdown editing aids | Bold `Ctrl+B`, italic `Ctrl+I`, link `Ctrl+Alt+K`; list continuation on Enter; table format (`Alt+Shift+T` aligns pipes); heading promote and demote; toggle task box; paste URL over selection makes a link. | S | 2 |
| OF01-F247 | Markdown export | Export the preview to HTML (single file, no scripts) or to PDF through the print pipeline (G-16). | S | 3 |
| OF01-F248 | Outline for prose | Sidebar Outline shows Markdown headings; drag a heading to move its section. | S | 3 |
| OF01-F249 | Abbreviations | User list of `abbr -> expansion` applied on word boundary in prose grammars; off by default. | C | 3 |
| OF01-F250 | Word completion | `Ctrl+Space` offers words already in open buffers (no language intelligence). | S | 2 |

### 4.14 Commands, configuration, print

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F260 | Command palette | `Ctrl+K` or `Ctrl+Shift+P`: fuzzy search over every action, open tab, recent file, language, encoding, snippet, macro, setting and help page. Prefixes: `>` commands, `:` line, `@` symbol, `#` text in open buffers, `?` help. | M | 1 |
| OF01-F261 | One action table | Every function is a `ui::action` with id, label, default shortcut; menus, palette, shortcut settings and macros use the same ids. | M | 1 |
| OF01-F262 | `.editorconfig` and modelines | Honoured keys: `indent_style`, `indent_size`, `tab_width`, `end_of_line`, `charset`, `trim_trailing_whitespace`, `insert_final_newline`, `max_line_length`. Vim and Emacs modelines map to the same keys plus language and spell language. `.editorconfig` is read only inside a granted folder. | S | 2 |
| OF01-F263 | Per-language settings | Any setting in section 8 marked "L" can be overridden per language. | M | 1 |
| OF01-F264 | Keymap presets | Default, Emacs-style, Vim-style modal (normal, insert, visual, `:` command line with `w q s/// g//`), each a ForgeScript key table. | S | 3 |
| OF01-F265 | Command line | `text-editor [files] [+line[:col]] [--wait] [--new-window] [--session n] [--diff a b] [--read-only] [--private] [-]`. Paths are resolved by the launcher into handles (SH04) before the app sees them. | M | 1 |
| OF01-F266 | `$EDITOR` use | A small launcher `edit` hands the file handle to the running instance and waits (F019). Works from SY03 inside a capability-scoped shell. | M | 1 |
| OF01-F267 | Print | `Ctrl+P` through the print portal: header (file, date), footer (page n of m), line numbers, syntax colours or monochrome, wrap, font size, selection only. PDF spool (G-16). | S | 2 |
| OF01-F268 | Export as HTML or PDF | Highlighted text as one self-contained HTML file or PDF. | C | 3 |
| OF01-F269 | Copy as rich text | Copies the selection with colours as HTML and RTF offers beside plain text. | C | 3 |
| OF01-F270 | Share | `share` intent with the file handle or the selection as text. | S | 2 |
| OF01-F271 | Settings UI | `Ctrl+,` opens a searchable settings tab; every setting shows default, current, and which layer set it (default, user, language, `.editorconfig`, modeline). | M | 1 |
| OF01-F272 | Settings as text | Settings can be opened as the JSON file behind `svc.settings` with schema-driven completion and validation. | S | 2 |
| OF01-F273 | Welcome tab | First start only: four lines on persistence, portal grants, the palette and where settings are. | S | 1 |
| OF01-F274 | In-app help | `F1` opens the help page for the focused control from `help/*.md`. | M | 1 |
| OF01-F275 | Hand-offs | "Open in": Hex Editor (DV04), IDE (DV01), Version Control (DV03), Sheets (OF05, for CSV), Notes (OF02, "Save as note"), Files (SY02, reveal). | M | 1 |

### 4.15 Accessibility

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF01-F280 | Text ranges in the a11y tree | The editor exposes line, word and character ranges, caret and selections, token class and spelling state as text attributes (needs FUSE-WP-55). | M | 1 |
| OF01-F281 | Spoken status | `Ctrl+Alt+S` announces line, column, selection size, language and encoding through a live region. | S | 2 |
| OF01-F282 | Audible and tactile cues | Optional sounds for fold, error banner, search wrap and "no match". | C | 3 |
| OF01-F283 | Multi-caret narration | With several carets only the primary is narrated; a command lists all caret lines. | S | 2 |
| OF01-F284 | Braille friendliness | Indentation level announced as a number, not as spaces; long-line chunks align with braille panning. | S | 3 |
| OF01-F285 | Minimap and decorations are optional | Nothing is conveyed only by the minimap, colour or change bars; each has a list view (`View > Changes`, `View > Problems in File`). | M | 2 |

## 5. GUI

### 5.1 Window map

```
Main window (one or more)
├─ Header bar: menu, title, search/palette field, lock chip, window buttons
├─ Tab strip (per pane)
├─ Sidebar (F9): Open Files | Folder | Outline | Search | Snippets | History
├─ Editor area: 1–8 panes in nested splits
│   ├─ Code view (text, gutter, minimap, scroll bar marks)
│   ├─ Banner stack (binary, external change, encoding, trojan-source, large file, read-only)
│   ├─ Find/replace bar (per pane)
│   ├─ Markdown preview pane
│   └─ Diff view pane
├─ Bottom panel (Ctrl+J): Search Results | Changes | Problems in File | Macro Output
├─ Status bar
├─ Popovers: language, encoding, line ending, indent, spell language, go to line, caret list
├─ Sheets: Convert Encoding, Mixed Line Endings, Replace in Folder preview, Print options, File Information, File Access
├─ Dialogs: Compare..., Macro Manager, Snippet Editor, Unsaved changes (optional), Clear history
├─ Command palette (overlay)
└─ Tabs that are not files: Settings, Welcome, Keyboard Shortcuts, Scratch Buffers
Portal windows (drawn by SH04, not by this app): open, save, folder grant, print, colour picker, elevated save prompt
```

### 5.2 Main window

Shown at 1280×800, medium density, one split, sidebar and minimap on.

```
+-----------------------------------------------------------------------------------------+
| [=] Text Editor — rc.conf (/etc)        [ Search or command  Ctrl+K ]     [🔒] [▁][▢][✕] | (1)
+-----------+-----------------------------------------------+-----------------------------+
| OPEN  (2) | rc.conf ● | notes.md | [🔒] messages     [+]  | notes.md (preview)      [x] | (3)
| > rc.conf +-----------------------------------------------+-----------------------------+
|   notes.md| ! Owned by root. Saving needs approval. [Save as administrator] [x]     (4) |
|   messages+----+------------------------------------+-----+-----------------------------+
| FOLDER    |  1 | # network                          |▒▒   | # Notes                     |
| v etc/    |  2 | hostname="pbsd"                    |▒    |                             |
|   rc.conf |  3 | ifconfig_em0="DHCP"                |▒▒▒  | - buy disks                 |
|   pf.conf |> 4 | pf_enable="YES"|                   |▒▒   | - [x] snapshot before update|
|   hosts   |  5 v sshd_enable="NO"                   |[▒▒] |                             |
| OUTLINE   |  6 | …  12 lines folded                 |▒    |                             |
|   network | 19 | # services                         |▒▒   |                             |
|   services|    |                (5)                 | (6) |             (7)             |
|           +----+------------------------------------+-----+-----------------------------+
|           | Find [ pf_          ] [Aa][W][.*][sel]  2 of 5  [<][>]  [Replace v]   [x] (8)|
+-----------+-------------------------------------------------------------------------+---+
| Ln 4, Col 16 · 1 sel · rc.conf (shell-like) · UTF-8 · LF · Spaces: 4 · en_AU · 100%  ⟳ 1 | (9)
+-----------------------------------------------------------------------------------------+
```

1. Header bar (`headerBar`, FUSE-WP-29). The lock chip appears for read-only or private windows. There is never a `[net]` badge.
2. Sidebar (`sidebar` with `listView` and `treeView`), 220 px. Sections collapse. The Folder tree lists only granted folders.
3. Tab strips (`tabView`, FUSE-WP-31), one per pane. `●` modified, `[🔒]` read-only, italic for preview tabs.
4. Banner stack (`banner`, FUSE-WP-30). At most three shown; more collapse into "2 more".
5. Code view (`codeView`, FUSE-WP-37): gutter lanes (bookmark, number, fold, change bar), text, caret, fold chip.
6. Minimap, 96 px, with viewport slider `[▒▒]`.
7. Markdown preview (`markdownView`, FUSE-WP-42) in a `splitter` pane.
8. Find bar (`searchField`, `toolButton` toggles, `label`). Grows a second row for Replace.
9. Status bar (`statusBar`); each chip is a `toolButton` that opens a popover. `⟳ 1` is the background job counter.

States: empty (no tabs: "Open a file or start typing" with [New] [Open]), loading (first screen shown, `~` line numbers), error banner, read-only, locked (session store sealed and keyring locked: buffers listed but blank until unlock), narrow.

### 5.3 Other views and dialogs

**Find in folder and results** (bottom panel, 1280×260).

```
+-----------------------------------------------------------------------------------------+
| Search Results ×  | Changes | Problems                                          [^][x]  |
+-----------------------------------------------------------------------------------------+
| Find    [ sshd_(\w+)="NO"                 ] [Aa][W][.*]     In [ etc/ (granted)     v]  |
| Replace [ sshd_$1="YES"                   ] [Preserve case]                             |
| Include [ *.conf, rc.*      ]  Exclude [ *.bak      ]  [x] .gitignore  [x] skip binary  |
| [Find]  [Replace 3 in 2 files…]                      3 matches · 2 files · 0.4 s  [Stop] |
+-----------------------------------------------------------------------------------------+
| v [x] rc.conf (2)                                                                       |
|     [x]  5: -sshd_enable="NO"        +sshd_enable="YES"                                 |
|     [x] 41: -sshd_dsa="NO"           +sshd_dsa="YES"                                    |
| v [x] rc.conf.local (1)                                                                 |
|     [ ]  2: -sshd_enable="NO"        +sshd_enable="YES"                                 |
+-----------------------------------------------------------------------------------------+
```

`treeView` with check boxes (FUSE-WP-28). States: no folder granted ("Grant a folder to search" + [Choose Folder]), running (streaming rows, progress), step-limit hit (warning row naming the file), cancelled, no matches.

**Diff view** (pane, 1100×500).

```
+------------------------------------------------------------------------------------------+
| rc.conf (saved) ↔ rc.conf (buffer)   [Side by side v] [Ignore: none v]  3 changes [<][>] |
+---------------------------------------------+--+-----------------------------------------+
|  3 | ifconfig_em0="DHCP"                    |  |  3 | ifconfig_em0="DHCP"                |
|  4 | pf_enable="NO"                         |→←|  4 | pf_enable="YES"                    |
|  5 | sshd_enable="NO"                       |  |  5 | sshd_enable="NO"                   |
|    | ////////////////////////////////////// |← |  6 | sendmail_enable="NONE"             |
+---------------------------------------------+--+-----------------------------------------+
```

`diffView` (FUSE-WP-38). Centre lane buttons copy a hunk across. States: identical ("No differences"), heuristic fallback chip, one side read-only.

**Convert encoding sheet** (480×360).

```
+------------------------------------------------------+
| Save "report.txt" as which encoding?                 |
| Current: windows-1252 (detected, 82% sure)           |
| Encoding  [ UTF-8                              v]    |
| [ ] Write a byte-order mark                          |
| Line endings  ( ) Keep  (•) LF  ( ) CRLF  ( ) CR     |
| ! 2 characters cannot be written in ISO-8859-1:      |
|   line 14 col 9  "€"     line 88 col 2  "‑"          |
|                    [Cancel] [Reopen as…] [Convert]   |
+------------------------------------------------------+
```

`sheet` with `comboBox`, `checkBox`, `radioButton`, `listView`. The warning list shows only when the target cannot hold every character.

**Command palette** (overlay, 640 wide).

```
+--------------------------------------------------------------+
| > sort                                                       |
+--------------------------------------------------------------+
| > Sort Lines Ascending                           (no shortcut)|
|   Sort Lines Numeric                                         |
|   Sort Lines Unique                                          |
|   Setting: editor.sort.locale                     en_AU      |
+--------------------------------------------------------------+
| ↑↓ choose · Enter run · Ctrl+Enter keep open · Tab complete  |
+--------------------------------------------------------------+
```

`commandPalette` (FUSE-WP-30). Rows show the shortcut and, for settings, the current value.

**Macro manager** (dialog, 720×480).

```
+--------------------------------------------------------------------------+
| Macros                                                                    |
+----------------------+---------------------------------------------------+
| > csv-quote   ✓signed| fn run(ed: editor::Buffer) {                      |
|   strip-ansi  ✓signed|   ed.each_caret([&](c) {                          |
|   fix-dates   ! edited|    c.select_line(); c.replace(quote(c.text()));  |
|                      |   });                                             |
| [+] [Import…] [−]    | }                                                 |
+----------------------+---------------------------------------------------+
| Shortcut [ Ctrl+Alt+Q ]  Budget [ 5 s ]  API used: select, replace       |
|                              [Export…] [Trust and Sign] [Run] [Close]    |
+--------------------------------------------------------------------------+
```

`listView`, `codeView`, `lineEdit`. "! edited" marks a macro whose signature no longer matches.

**Binary file banner and restored-session banner** (inline).

```
| ! This file looks binary (NUL bytes found). [Open in Hex Editor] [Open as Latin-1, read-only] [x] |
| i 3 unsaved buffers were restored from 14:02 yesterday.  [Show] [x]                                |
| ! rc.conf changed on disk and you have unsaved edits.  [Compare] [Reload] [Keep mine]              |
```

**File information sheet** (420×380): `propertyGrid` with path, size, owner, mode, modified, encoding, BOM, line endings, lines, words, provenance, handle rights, and [Copy Path] [Reveal in Files].

**Settings tab**: `sidebar` of groups (Editor, Files, Search, Appearance, Languages, Spelling, Session, Privacy, Shortcuts), `searchField`, rows from the settings schema with a layer chip.

### 5.4 Menus

```
File    New Ctrl+N · New Private Window Ctrl+Shift+N · New from Template ▸ · Open… Ctrl+O · Open Folder…
        Open Recent ▸ · Reopen Closed Tab Ctrl+Shift+T · Scratch Buffers
        Save Ctrl+S · Save As… Ctrl+Shift+S · Save a Copy… · Save All · Save as Administrator
        Revert · Earlier Versions ▸ (Local History, Snapshots) · Rename… F2 · Move To… · File Information · Access…
        Reopen with Encoding ▸ · Convert Encoding… · Line Endings ▸
        Print… Ctrl+P · Export ▸ (HTML, PDF, Patch) · Share…
        Close Tab Ctrl+W · Close Window Ctrl+Shift+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Undo in Selection · Repeat Ctrl+.
        Cut Ctrl+X · Copy Ctrl+C · Paste Ctrl+V · Paste from History · Copy with Secrets Masked · Copy as Rich Text
        Line ▸ (Duplicate Ctrl+D, Delete Ctrl+Shift+K, Move Up Alt+Up, Move Down Alt+Down, Join Ctrl+J, Sort ▸, Reflow Alt+Q, Align…)
        Text ▸ (Case ▸, Transform ▸, Normalise ▸, Remove Invisible Characters, Trim Trailing Whitespace)
        Indent ▸ (Indent Ctrl+], Outdent Ctrl+[, Convert ▸) · Toggle Comment Ctrl+/ · Block Comment Ctrl+Shift+/
        Insert ▸ (Date, UUID, File Name, Unicode… Ctrl+Shift+U, Number Sequence…, Snippet… Ctrl+Alt+J, Character… )
        Format ▸ (JSON Pretty, JSON Minify, XML Re-indent, Markdown Table Alt+Shift+T)
Select  Select All Ctrl+A · Expand Alt+Shift+Right · Shrink Alt+Shift+Left · To Bracket
        Add Caret Above/Below Ctrl+Alt+Shift+Up/Down · Add Next Occurrence Ctrl+Shift+D · Select All Occurrences Ctrl+Shift+L
        Carets at Line Ends Alt+Shift+I · Split into Lines · Split by Regex… · Undo Caret Ctrl+U · Column Mode Alt+C
Find    Find Ctrl+F · Replace Ctrl+H · Find Next Ctrl+G · Previous Ctrl+Shift+G · Find Word Under Caret Ctrl+F3
        Find in Folder Ctrl+Shift+F · Filter Lines ▸ · Saved Searches ▸
        Go to Line Ctrl+L · Go to Symbol Ctrl+Shift+O · Go to Bracket Ctrl+Shift+\ · Back Alt+Left · Forward Alt+Right
        Bookmarks ▸ (Toggle Ctrl+F2, Next, Previous, Clear, List)
View    Command Palette Ctrl+K · Sidebar F9 · Bottom Panel Ctrl+J · Minimap · Status Bar · Focus Mode Ctrl+Shift+F11 · Full Screen F11
        Split Right Ctrl+\ · Split Down Ctrl+Shift+- · Equalise · Move Tab to New Window
        Soft Wrap Alt+Z · Line Numbers ▸ · Whitespace ▸ · Line Endings · Indent Guides · Rulers… · Sticky Scope
        Fold ▸ (Fold Ctrl+Shift+[, Unfold Ctrl+Shift+], Fold All, Unfold All, Level 1–9)
        Language ▸ · Colour Scheme ▸ · Zoom In/Out/Reset Ctrl++/-/0 · Follow (tail)
        Markdown Preview Ctrl+Shift+V · Preview to the Side Ctrl+K V · Changes · Problems in File
Tools   Compare… · Compare with Saved Ctrl+Alt+D · Apply Patch…
        Spelling ▸ (Check Buffer, Language ▸, Add Word, Ignore)
        Macros ▸ (Record Ctrl+Shift+R, Play Ctrl+Shift+E, Play Many…, Manage…) · Snippets…
        Word Count · Inspect Scope · Open in ▸ (Hex Editor, IDE, Version Control, Sheets, Notes, Files)
        Sessions ▸ · Clear Editor History… · Settings Ctrl+, · Keyboard Shortcuts
Help    Text Editor Help F1 · Keyboard Reference · Grammar Format · Macro API · About
```

### 5.5 Keyboard and pointer

- Shortcuts are in sections 4 and 5.4. Where a default collides with a reserved key (DESIGN_SYSTEM.md section 6) the reserved meaning wins outside a text view; `F2` renames a tab when the tab strip has focus and steps to the next bookmark when a text view has focus. All are rebindable.
- Chords: `Ctrl+K` followed by a key within 1 s (`Ctrl+K V`, `Ctrl+K Ctrl+0` fold all); `Ctrl+K` alone opens the palette after the timeout.
- Pointer: double click selects a word, triple a line, quadruple a paragraph; `Shift+click` extends; middle click pastes the primary selection (setting); `Ctrl+wheel` zooms; `Alt+wheel` scrolls by pages; click in the number gutter selects lines; click a fold arrow folds; drag the pane splitter.
- Context menu, text: Cut, Copy, Paste, Select All, spelling suggestions, Add Caret Here, Toggle Comment, Format, Transform ▸, Compare with Clipboard, Open Link, Insert Snippet ▸.
- Context menu, tab: Close, Close Others, Close to the Right, Pin, Split Right, Move to New Window, Copy Path, Reveal in Files, Rename, Compare with…, File Information, Forget This File.
- Context menu, gutter: Toggle Bookmark, Fold, Revert Change, Copy Line Number Link.
- Drag and drop: files from SY02 onto the editor open (the drop carries handles); onto a pane edge open in a new split; text drags move or copy (F058); a tab dragged out becomes a window; a tab dropped on SY02 offers `text/plain` and a file handle.
- Touch: one-finger scroll, pinch zoom, long press for the context menu and selection handles.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No tabs: centred hint and [New] [Open] [Open Folder]. |
| Loading | First screen in under 100 ms; status job "Indexing lines 42%"; editing allowed at once. |
| Error | Banners for open, decode and save failures with the errno text and [Details]; a failed save keeps the buffer modified and journalled. |
| Offline | Not applicable: the app has no network use. |
| Locked | Session store sealed and keyring locked: tabs show titles, bodies show "Unlock to restore" with [Unlock]. |
| Read-only | Lock on the tab and status bar; typing shows a one-line toast "Read-only" with [Make Writable] or [Save as Administrator] where possible. |
| Disk full | Save fails before the rename; the original file is untouched; banner offers Save As. |
| Narrow (< 640) | Sidebar becomes an overlay, minimap hides, status chips collapse into "…", panes stack as tabs. |
| Large-file mode | Chip in the status bar; disabled features greyed with the reason in the tooltip. |

### 5.7 Widgets

What `codeEdit` has today (Verified, `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp:348-360`): a multi-line editor with a line-number gutter; syntax colouring for four built-in languages (`CodeLanguage`: Plain, Lua, Cpp, Css) in five token classes (text, keyword, string, number, comment); one caret; selection with Shift; Tab inserts spaces; auto-indent on Enter; undo with `Ctrl+Z`; copy, cut, paste; vertical scrolling; an API of `codeEditSetText`, `codeEditText`, `codeEditLineCount`, `codeEditCaret`, `codeEditSetCaret`, `codeEditTokenAt`; edits emit `TextChanged`. The document crosses the API as one `std::string`. FUSE_BUILD_PLAN.md section 2.2 adds (from the source): undo stores whole-document snapshots, capped at 128.

What must be added (all Proposed): everything in FUSE-WP-19 (piece tree, snapshots, edit-log undo, line index, transcoder hook, long-line chunking) and FUSE-WP-37 (`codeView` over `BufferHandle`, several views per buffer, `SelectionSet` with multi-caret and column mode, grammar loader and linear-time matcher, incremental tokeniser, folding, soft wrap, gutter lanes, decorations, bracket matching, indent guides, rulers, whitespace and invisible-character display, overwrite mode, IME, minimap, redo, horizontal scrolling).

| Region | Widget | Exists today | Needs |
|---|---|---|---|
| Editor | `codeView` | `codeEdit` only (limits above) | FUSE-WP-19, FUSE-WP-37 |
| Diff | `diffView` | no | FUSE-WP-38 |
| Tabs, splits | `tabView`, `splitter` | `tabBar`, `tabWidget`, `splitter` | FUSE-WP-31 (reorder, tear-off) |
| Sidebar lists | `listView`, `treeView` | yes, limited (64 collapsible rows, single select) | FUSE-WP-28 |
| Header, status, banners, palette, sheets, popovers | chrome kit | no | FUSE-WP-29, FUSE-WP-30 |
| Preview | `markdownView` | headings, lists, code, quotes only; inline marks stripped | FUSE-WP-42 |
| Settings rows | `propertyGrid`, `lineEdit`, `comboBox`, `toggleSwitch`, `spinBox` | yes | — |
| Find bar | `lineEdit`, `toolButton` | yes | multi-line `searchField` (FUSE-WP-29) |

`OF01-GAP-1`: a shell prompt that returns a one-shot write handle to a single root-owned file (F009). Not in PLATFORM.md section 8. `OF01-GAP-2`: a launcher-side path-to-handle resolver for command-line arguments (F265, F266).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic (`main.fs`, `src/*.fs`) | ForgeScript | app host | Actions, buffer registry, session, settings layers, banners, hand-offs |
| Views (`ui/*.fuseui`, `style/*.css`) | FUSE UI | app host | Windows, panes, sheets, schemes |
| Code editor core | C++23 (FUSE library, FUSE-WP-19/37/38) | app host | Piece tree, selections, tokeniser, layout, diff. Part of the platform, not of this bundle |
| Text library | C++23 (FORGE `forge::os::text`) | app host | Encodings, regex for user search, segmentation, collation (Verified to exist: `FORGE:docs/text-locale.md`) |
| `search-helper` | C++23 | helper, capability mode | Folder search and replace preview over directory and file handles passed in; grammar import |
| Macro engine | ForgeScript (second engine instance, empty caps) | app host | Runs user macros against the `editor::` API only |
| Session store | ForgeScript + SQLite | app host | Journal, sessions, local history, undo |
| `edit` launcher | C++23 | short-lived | Resolves arguments to handles through SH04, forwards to the instance, waits |

```
 SH04 portals ──handles──▶ app host ─────────────────────────────────────────────┐
 svc.settings/clipboard/   │ ForgeScript app logic ── macro engine (no caps)     │
 keyring/notify/audit ◀───▶│ FUSE UI: codeView, diffView, markdownView           │
 spelling service (G-17) ◀▶│ piece tree ◀─ mmap(read-only) file handle           │
                           │ session store (SQLite + journal) in fs.app          │
                           └──IPC + passed dir/file handles──▶ search-helper     │
                                                               (Capsicum, no net)┘
```

### 6.2 Data model

Stores under `~/.local/share/org.pbsd.text-editor/` (schema version in each):

| Entity | Fields |
|---|---|
| `session` | `id: u64`, `name: text`, `updated: i64`, `active: bool` |
| `window` | `id`, `session_id`, `x,y,w,h: i32`, `maximised: bool`, `layout: json` (split tree), `private: bool` (never stored when true) |
| `buffer` | `id: u64`, `window_id`, `pane: u8`, `tab_index: u16`, `file_identity: blob(48)` (dev, inode, size, mtime, edge hash), `grant_token: text` (portal re-open token), `display_path: text`, `untitled_n: u32`, `encoding: text`, `bom: bool`, `eol: u8`, `language: text`, `indent: json`, `read_only: bool`, `pinned: bool` |
| `view_state` | `buffer_id`, `pane`, `carets: blob` (pairs of u64 offsets), `scroll_line: u64`, `scroll_x: f32`, `folds: blob`, `wrap: u8`, `zoom: f32` |
| `journal_record` (file `journal/<buffer_id>.j`) | `seq: u64`, `kind: u8` (insert, delete, group, checkpoint), `offset: u64`, `len: u32`, `bytes`, `prev_hash: [u8;32]`, `hash: [u8;32]` |
| `history_version` | `file_identity`, `time: i64`, `reason: u8` (save, revert, replace-in-folder), `zstd_blob_path: text`, `size: u64`, `blake3: [u8;32]` |
| `bookmark` | `file_identity`, `line_anchor: u64`, `label: text` |
| `recent` | `grant_token`, `display_path`, `opened: i64`, `missing: bool` |
| `search_history` | `pattern`, `replacement`, `flags: u16`, `scope`, `time` |
| `macro` (file `macros/<name>.fs` + `.sig`) | `name`, `shortcut`, `budget_ms`, `trusted_hash` |
| `snippet` (JSON files) | `prefix`, `body`, `description`, `scope` |

In-memory buffer: piece tree over `original` (read-only mapping) and `add` (append-only); `LineIndex` (sparse); `EditLog`; `Decorations`. Migration is forward only; the previous database is copied to `*.bak-<version>` first. Journals are replay-safe: a checkpoint record carries the file identity the offsets refer to.

### 6.3 File formats and protocols

| Format | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Text in the encodings of F121 | yes | yes | FORGE `forge::os::text` | app host (decoder is bounds-checked; fuzzed) |
| `.fusegrammar` | yes | no | FUSE-WP-37 loader | app host; schema, size and rule limits |
| TextMate / Sublime grammars | import | no | own converter | `search-helper` |
| `.editorconfig`, modelines | yes | no | own code (ForgeScript) | app host; fixed key list |
| Snippet JSON (own, VS Code, TextMate) | yes | own | ForgeScript JSON | app host |
| Unified diff | yes | yes | own code on FUSE-WP-38 | app host |
| Markdown (CommonMark) | yes | — | FUSE-WP-42 parser | app host |
| HTML, RTF (clipboard and export) | no | yes | own writer | app host |
| PDF (print, export) | no | yes | print pipeline G-16 | print service |
| zstd (local history) | yes | yes | zstd (BSD-3-Clause; in the HardenedBSD base system) | app host; only on blobs this app wrote, hash-checked first |
| SQLite (session store) | yes | yes | SQLite (public domain) via `fuse_ui_sql` | app host; own files only |
| Session bus messages | yes | yes | schema compiler (G-08) | app host |

### 6.4 Integration

- Intents handled: `open`, `edit`, `view`, `new` (text document), `search` (opens find in the current buffer). Emitted: `open` (links, hand-offs to DV04, DV01, DV03, OF05, OF02, SY02), `share`, `print`.
- MIME: view and edit `text/plain`, `text/markdown`, `text/x-*` source types, `application/json`, `application/xml`, `application/toml`, `application/yaml`, `text/x-diff`, `text/x-log`, `inode/x-empty`; export `text/html`, `application/pdf`, `text/x-diff`.
- Services: `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.notify` (long saves and searches), `svc.keyring` (session seal key, macro signing key reference), `svc.audit`, `svc.a11y`, spelling service (G-17).
- Interface offered on the session bus: `org.pbsd.text-editor.Open(handle, line, column, wait) -> token` and `Closed(token)` for the `edit` launcher and other apps.
- FUSE: Verified today is only `codeEdit` and `markdownView` as described in 5.7; the rest is Proposed in FUSE_BUILD_PLAN.md.
- FORGE: Verified `forge::os::text` regex, encodings, segmentation and collation; signing of scripts; `secure_buffer`. ForgeScript bindings for `ui::` and services are Proposed (FORGE-WP-12, 13, 24).
- Not indexed by `svc.search`: the editor contributes nothing to the system index.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Session store, journals, local history, settings, user grammars, snippets, macros |
| `fs.portal` | Open and save user files and granted folders; remembered grants for session restore |
| `proc.helper:search-helper` | Folder search and grammar import outside the app process |
| `net.none` | The editor never opens a socket |
| `dev.print` | Printing through the portal |
| `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.notify`, `svc.keyring`, `svc.audit`, `svc.a11y`, `svc.spell` (G-17, name Proposed) | As in 6.4 |

Not requested: `net.*`, `proc.exec`, `ai.local`, `bg.*`, `fs.path:*`, `svc.search`, any device but print.

Assets: file contents (often configuration with secrets), unsaved text, the session store, the user's macro signing key (held by SE04, never by this app).

| Threat | Mitigation |
|---|---|
| Hostile file content exploits the decoder, tokeniser or layout | Memory-safe C++23 subset with bounds checks; linear-time matcher; long-line chunking; fuzzers on decoder, grammar loader, piece tree; untrusted-provenance files open read-only first (F023) |
| Regex denial of service in search | Step limit 1e7 per search (Verified in FORGE); search runs on a job and can be cancelled |
| Hostile grammar, snippet or scheme file | Data only; schema and size limits; no code paths; CSS limited to colour and font properties for token classes |
| Macro abuses the user's rights | Empty capability set, `editor::` API only, budgets, signature bound to the user's key, visible API summary before trust |
| Trojan-source and homoglyph tricks | F063, F218 |
| Modeline or `.editorconfig` attack | Fixed key list, no expressions (F219) |
| Symlink or path tricks during folder search and replace | Helper receives a directory handle; opens with `O_NOFOLLOW` relative to it under Capsicum; never leaves the grant |
| Mapped file changed or truncated underneath | Freeze and notice (F119); the host passes a private snapshot or the buffer copies on first touch (FUSE-WP-19 risk Q-08) |
| Data loss on crash or power cut | Journal with chained hashes; atomic save; previous version in local history |
| Secrets left in the session store | Private windows; "Forget this file"; optional sealing; store lives in `fs.app` which other apps cannot read |
| Elevated save misused | The prompt is drawn by the shell, names the single file, grants one write, is audited (OF01-GAP-1) |
| Clipboard leakage | Sensitive flag on detected secrets and in private windows |

Stored: session, journals, local history, recent list, search history, bookmarks, macros, snippets, settings; all under the app's own directories. Leaves the machine: nothing.

## 8. Settings

"L" marks keys that can be set per language.

| Key | Type | Default | Description |
|---|---|---|---|
| `editor.font.family` | string | system monospace | L |
| `editor.font.size` | int 6–72 | 13 | L |
| `editor.font.ligatures` | bool | false | Needs G-03 |
| `editor.line_height` | float 1.0–2.0 | 1.4 | |
| `editor.indent.use_tabs` | bool | false | L |
| `editor.indent.size` | int 1–16 | 4 | L |
| `editor.indent.detect` | bool | true | |
| `editor.wrap` | enum off, window, ruler | off | L |
| `editor.rulers` | list of int | [] | L |
| `editor.line_numbers` | enum absolute, relative, interval, off | absolute | |
| `editor.whitespace` | enum none, boundary, selection, all | selection | |
| `editor.minimap` | bool | true | |
| `editor.auto_close_pairs` | bool | true | L |
| `editor.trim_trailing_whitespace` | enum off, all, changed | changed | L |
| `editor.final_newline` | bool | true | L |
| `editor.default_encoding` | string | UTF-8 | |
| `editor.default_eol` | enum lf, crlf, cr | lf | |
| `editor.large_file_bytes` | int | 67108864 | Large-file mode threshold |
| `editor.undo_memory_bytes` | int | 268435456 | Per buffer |
| `editor.middle_click_paste` | bool | true | |
| `editor.invisible_chars` | enum off, code, always | code | F063 |
| `files.ask_on_close` | bool | false | Classic save dialog |
| `files.reload_unmodified` | bool | true | |
| `files.local_history.enabled` | bool | true | |
| `files.local_history.max_bytes` | int | 1073741824 | |
| `files.local_history.max_days` | int | 30 | |
| `search.step_limit` | int | 10000000 | Regex steps per search |
| `search.folder.max_file_bytes` | int | 67108864 | |
| `search.folder.respect_gitignore` | bool | true | |
| `search.history` | bool | true | |
| `session.restore` | enum all, pinned, none | all | |
| `session.journal_fsync_seconds` | int 1–60 | 5 | |
| `session.seal` | enum auto, always, never | auto | F198 |
| `session.persistent_undo` | bool | true | |
| `spell.enabled` | bool | true | L |
| `spell.languages` | list | [system locale] | L |
| `spell.in_code` | enum comments_strings, off | comments_strings | |
| `markdown.preview.sync_scroll` | bool | true | |
| `macros.budget_ms` | int | 5000 | |
| `macros.require_signature` | bool | true | |
| `privacy.detect_secrets_on_copy` | bool | true | |
| `appearance.scheme` | string | follows system theme | |
| `keymap.preset` | enum default, emacs, vim | default | |

## 9. Accessibility and languages

- F280–F285 apply. Every banner is a live region. The find bar announces "n of m" on each step.
- High-contrast schemes are shipped and chosen automatically with the system high-contrast theme.
- Right-to-left and mixed-direction lines need G-03; until then such files open with a banner saying layout is left-to-right only.
- IME pre-edit is drawn inline at every caret; only the primary caret takes composition, the rest receive the committed text.
- UI strings in `locale/*.tbl`. Grammar and command names are translated; token class names are not.
- Sorting, case mapping and word movement use the buffer's language when set, else the system locale.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame, empty session | ≤ 150 ms |
| Cold start restoring 20 tabs | ≤ 300 ms (only visible tabs load) |
| Open to first screen, any file size up to 16 GiB | ≤ 100 ms |
| Key to paint | ≤ 4 ms (aligns with FUSE-WP-37) |
| Scroll frame | ≤ 4 ms |
| 10 000-caret insert | ≤ 30 ms |
| First match of a literal in a 4 GiB file | ≤ 2 s on NVMe; full scan ≥ 1 GiB/s literal, ≥ 100 MiB/s regex |
| Folder search | ≥ 200 MiB/s over warm cache, results streamed within 50 ms of the first hit |
| Save 1 GiB | bounded by disk; UI stays at frame budget |
| Memory, empty session | ≤ 60 MiB |
| Memory above the mapping, 4 GiB file with 100 000 edits | ≤ 64 MiB (FUSE-WP-19) |
| Journal overhead | ≤ 2% of typing CPU; ≤ 1 `fsync` per 5 s |
| Idle | zero frames, zero wake-ups (caret blink is the only timer and stops after 30 s without input) |

## 11. Testing and proof

Nothing has run. All rows are plans.

- Unit (ForgeScript): settings layering, modeline parser, `.editorconfig`, snippet expansion, macro recording to source, case and sort commands, session replay.
- Engine (C++23, in FUSE): the gates named in FUSE-WP-19, 37, 38 (`fuse_text_piece_tree_ops`, `fuse_text_piece_tree_4g`, `fuse_text_long_line`, and the WP-37 and WP-38 gates).
- Golden renders: main window in each state of 5.6; each scheme; each banner; diff view; palette.
- Scripted UI through the a11y tree: tasks T1–T14 of section 3 with step counts asserted.
- Fuzz: encoding decoders, binary detection, grammar loader, grammar converter, modeline parser, unified-diff reader, journal reader, snippet JSON.
- Differential: regex results against a reference engine on a corpus; diff output against `diff -u`; encoding round trips against `iconv`; round-trip identity (open, save without edits, compare bytes) over a corpus including invalid bytes and mixed line endings.
- Crash tests: kill the app at random points during typing and saving; replay must lose at most `journal_fsync_seconds` of typing and never corrupt the target file.
- Confinement: the app and `search-helper` make no socket call; the helper cannot open a path outside its directory handle (test with symlinks and `..`).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Piece tree (FUSE-WP-19) | Balance, length and line sums; no overflow to 2^44 | T1 | PRISM proof of insert, delete, rotation | — |
| `SelectionSet`, matcher (FUSE-WP-37) | Sorted, in bounds; steps ≤ input × rules | T2 | PRISM | Full functional spec of grammars |
| Diff engine (FUSE-WP-38) | `patch(A, diff(A,B)) = B` | T1 (bounded) | PRISM | Unbounded sizes |
| Journal writer and replay | Replay of a prefix yields a prefix state; chain hash detects any change | T2 | PRISM on the C++23 record codec; model test of replay | Replay logic is ForgeScript (interpreter not proved) |
| Atomic save | Target is old or new, never mixed | T3 | Fault-injection tests | Depends on file system semantics |
| Encoding decoders | No out-of-bounds; round trip for valid input | T2 | PRISM plus fuzz | Third-party tables |
| `search-helper` | Never opens outside the passed directory handle | T2 | PRISM on path handling; Capsicum test | Kernel is the enforcer |
| Macro sandbox | A macro reaches only `editor::` | T3 | Negative tests on each capability | ForgeScript VM not proved |
| App logic (ForgeScript) | — | T4 | Unit and UI tests | Interpreted language |

## 12. Build plan

Phase 1, usable core (target milestone M2):

| ID | Work item | Depends on |
|---|---|---|
| OF01-W01 | Bundle skeleton, manifest, action table, menus, palette | G-02, G-07, FUSE-WP-14, FUSE-WP-29, FUSE-WP-30 |
| OF01-W02 | Buffer registry on `codeView`; open, save (atomic), revert, external change | FUSE-WP-19, FUSE-WP-37, FORGE-WP-20, SH04 |
| OF01-W03 | Encodings, BOM, line endings, invalid-byte preservation, binary detection | FORGE-WP-23, W02 |
| OF01-W04 | Tabs, splits, status bar, popovers | FUSE-WP-31, W01 |
| OF01-W05 | Editing commands of 4.2 and multi-cursor of 4.3 (phase 1 rows) | W02 |
| OF01-W06 | Find and replace bar | W02 |
| OF01-W07 | Grammar set, phase 1; schemes; language detection | FUSE-WP-37.3 |
| OF01-W08 | Session store, journal, restore, crash recovery | W02, G-09 (`svc.settings`) |
| OF01-W09 | Huge-file behaviour: lazy load, line index, large-file mode, long lines | W02 |
| OF01-W10 | Command line, `edit` launcher, bus interface | OF01-GAP-2, G-08 |
| OF01-W11 | Settings tab, per-language layers, help pages, welcome tab | W01 |
| OF01-W12 | A11y text ranges and scripted task tests | FUSE-WP-55, G-04 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| OF01-W13 | `search-helper`; find in folder; replace with preview; streaming search in huge files | FORGE-WP-27, FORGE-WP-28, G-06 |
| OF01-W14 | Diff view and compare sources | FUSE-WP-38 |
| OF01-W15 | Folding, minimap, scroll marks, indent guides, bidi | FUSE-WP-37, G-03 |
| OF01-W16 | Local history, snapshot versions, persistent undo | W08, SY02 snapshot interface |
| OF01-W17 | Snippets | W05 |
| OF01-W18 | Macros: recorder, `editor::` API, sandboxed engine, signing | FORGE-WP-09, SE04 |
| OF01-W19 | Spell check | G-17, FUSE-WP-18.5 |
| OF01-W20 | Markdown preview and editing aids | FUSE-WP-42 |
| OF01-W21 | Private window, sealing, access sheet, secret detection, audit | `svc.keyring`, `svc.audit` |
| OF01-W22 | Elevated save | OF01-GAP-1, SH03 |
| OF01-W23 | Print; phase 2 grammars; `.editorconfig` and modelines; sidebar views; log follow; sessions | G-16 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF01-W24 | Keymap presets (Emacs-style, Vim-style) | W05 |
| OF01-W25 | Grammar and snippet import; scope inspector | W13 |
| OF01-W26 | Patch apply and export; HTML and PDF export; copy as rich text | W14, G-16 |
| OF01-W27 | Focus mode, sticky scope, selective undo, align, increment, templates, abbreviations | W05 |
| OF01-W28 | Macro event hooks; cues; braille refinements | W18, AX01 |

## 13. Open questions and risks

1. Elevated save (OF01-GAP-1) needs a platform decision: a shell-drawn one-shot handle, or no elevated save at all and a hand-off to SY03.
2. The grammar format is shared with DV01; it must be frozen (with a version field) before either app ships grammars.
3. Memory-mapping a file another process truncates can fault. The answer (private snapshot or copy on first touch) is open in FUSE-WP-19; ZFS clones may make a private snapshot cheap.
4. Persistent undo keyed by file identity can attach to the wrong content if a file is replaced with equal size and time; the edge hash lowers but does not remove the risk. A full-file hash is too slow for huge files.
5. Vim-style modal editing is a large surface; it may move to a separate optional bundle.
6. The spelling service (G-17) has no interface yet; `svc.spell` is a placeholder name.
7. Session sealing when the keyring is locked delays restore; the default `auto` avoids it on encrypted home datasets.
8. Unverified: whether `forge::os::text` regex can search across piece boundaries without copying; if not, a streaming adapter is needed (W13).
