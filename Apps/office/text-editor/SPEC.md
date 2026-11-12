<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Text Editor

By Odin Loch

| | |
|---|---|
| ID | `OF01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.text-editor` |
| Category | office |
| Priority | P0 |
| Backing repos | FUSE (`codeEdit` widget), FORGE (`forge_text`: Unicode, encodings, regex) |
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
| OF01-F014 | Close with unsaved changes | No dialog by default: the buffer is persisted (F100) and returns on next start. An optional setting restores the classic Save/Discard/Cancel dialog. | M | 1 |
| OF01-F015 | Read-only mode | Per-buffer lock toggle; set automatically for read-only handles, files in snapshots and log-follow mode. | M | 1 |
| OF01-F016 | Binary detection | First 64 KiB scanned: NUL bytes or over 10% invalid sequences in every candidate encoding marks the file as binary. Banner offers "Open in Hex Editor" (intent to DV04), "Open anyway as Latin-1 (read-only)". | M | 1 |
| OF01-F017 | Local history | Every save and every revert stores a compressed copy (zstd) in the app data store, keyed by file identity. Limits: 50 versions or 30 days per file, 1 GiB total. | S | 2 |
| OF01-F018 | Snapshot versions | "Earlier versions" lists ZFS snapshots that contain the file (through SY02's snapshot interface) and opens one read-only or in the diff view. | S | 2 |
| OF01-F019 | Open from standard input | `text-editor -` reads a pipe into an untitled buffer; `--wait` blocks the caller until the tab closes (for `$EDITOR`). | S | 1 |
| OF01-F020 | Open a folder | A granted folder appears in the sidebar file tree (F140). Not a project system. | S | 2 |
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
