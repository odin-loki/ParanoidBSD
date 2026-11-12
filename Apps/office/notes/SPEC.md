<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Notes

By Odin Loch

| | |
|---|---|
| ID | `OF02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.notes` |
| Category | office |
| Priority | P1 |
| Backing repos | FUSE (rich text FUSE-WP-35, Markdown view FUSE-WP-42, canvas FUSE-WP-43: all Proposed), FORGE (text library, signing, keystore: Verified; `svc.model`: Proposed) |
| Status | Planned. Nothing built. |

## 1. Purpose

A personal knowledge base. Notes are Markdown files in notebooks, edited as rich text, linked to each other, tagged, searched in full, and optionally encrypted per notebook. Notes sync between the user's own devices with no server account. It replaces the note-taking role that the Plasma plan left to KDE apps (PLATFORM.md V20).

## 2. Scope

In scope:

- Notebooks, sections and notes; Markdown on disk; rich editing with a source view.
- Links, backlinks, graph, tags, attachments, drawings and handwriting, tables, tasks, templates, quick capture.
- Full-text search; encrypted notebooks; per-note history; snapshot browsing.
- Sync between own devices by shared folder or peer to peer; honest conflict handling.
- Import (Markdown folders, ENEX and others), export, web clips received from IN01, optional local AI.

Out of scope:

| Item | Owner |
|---|---|
| Page layout, styles, print-quality documents | OF04 Writer |
| Projects, boards, recurring tasks, reminders that fire | CO04 Tasks (Notes can send a task there) |
| Calendar events | CO02 Calendar |
| Plain-text and code editing of arbitrary files | OF01 Text Editor |
| Full vector drawing | OF08 Draw (Notes embeds simple sketches only) |
| PDF annotation | OF03 Document Viewer (Notes stores the PDF and links to a page) |
| Capturing a web page | IN01 Web Browser (emits a clip; Notes receives it) |
| Audio recording and transcription | ME06 Sound Recorder (Notes embeds the result) |
| Key storage and device identity keys | SE04 Passwords and Keys |
| Computational cells | AI04 Notebook |
| Multi-user collaboration, publishing to the web | Not planned |

## 3. Users and top tasks

Personas:

- **Collector**: saves thoughts, clips and files quickly, finds them later by search.
- **Researcher**: builds linked notes, uses backlinks, the graph and citations.
- **Planner**: daily notes, checklists, meeting notes from templates.
- **Guarded user**: keeps a journal, medical or legal notes in an encrypted notebook that locks itself.
- **Two-device user**: laptop and desktop, no cloud; syncs over the home network or a USB stick.

| # | Task | Target steps |
|---|---|---|
| T1 | Capture a thought from anywhere | 2 (global shortcut, type; saved on close) |
| T2 | Find a note by any word in it | 2 (`Ctrl+K`, type) |
| T3 | Link to another note while typing | 2 (`[[`, pick) |
| T4 | See what links to this note | 0 (backlinks panel) or 1 (`F4`) |
| T5 | Open today's daily note | 1 (`Ctrl+Shift+D`) |
| T6 | Tick a task | 1 (click or `Ctrl+Enter`) |
| T7 | Attach a file | 1 (drop) |
| T8 | Unlock an encrypted notebook | 2 (click, authenticate) |
| T9 | Restore yesterday's version of a note | 3 |
| T10 | Pair a second device and sync | 5, once; then 0 |
| T11 | Resolve a sync conflict | 2 (open conflict, choose or merge) |
| T12 | Import an ENEX file | 3 |
| T13 | Clip an article from the browser | 2 (in IN01: Share, Notes) |
| T14 | Export a notebook as a Markdown folder | 3 |
| T15 | Sketch with a pen inside a note | 1 (pen touches the page) |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Notebooks and organisation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F001 | Notebooks | A notebook is a folder: `notebook.json`, notes as `.md`, `attachments/`, `templates/`. Create, rename, recolour (8 token colours), set icon, archive, delete to trash. | M | 1 |
| OF02-F002 | Notebook location | Default `~/.local/share/org.pbsd.notes/notebooks/<uuid>/`. "External notebook" binds a folder granted through the portal, so plain Markdown folders are edited in place. | M | 1 |
| OF02-F003 | Sections | Folders inside a notebook, nested to depth 8; drag to reorder; order kept in `.order` files. | M | 1 |
| OF02-F004 | Notes | One Markdown file per note; file name is a slug of the title plus a 6-character id on collision; renaming a note renames the file and updates links (F056). | M | 1 |
| OF02-F005 | Front matter | Optional YAML subset block: `id`, `title`, `created`, `modified`, `tags`, `aliases`, `pinned`, `template`, `source_url`, `lang`. Unknown keys are kept untouched. | M | 1 |
| OF02-F006 | Note list | Middle pane, virtualised; shows title, two-line snippet, date, tag chips, attachment and task counts, lock, conflict mark. Density follows the system. | M | 1 |
| OF02-F007 | Sort and group | Sort by modified, created, title, manual; group by none, day, month, section, tag. Per notebook. | M | 1 |
| OF02-F008 | Pin and favourite | Pinned notes stay on top of their list; Favourites is a sidebar place across notebooks. | S | 1 |
| OF02-F009 | Move and copy | Drag notes between sections and notebooks; moving into an encrypted notebook encrypts, and warns that the plain copy stays in history and snapshots until pruned. | M | 1 |
| OF02-F010 | Duplicate, merge, split | Duplicate a note; merge selected notes in list order; split a note at headings of a chosen level. Links are fixed up. | S | 2 |
| OF02-F011 | Trash | Deleted notes go to `svc.undo-trash` with their attachments; Undo toast; purge after 30 days (setting). | M | 1 |
| OF02-F012 | Archive | Archived notes leave lists and the graph but stay searchable with a filter. | S | 2 |
| OF02-F013 | Multiple windows and tabs | Notes open in tabs; `Ctrl+Enter` on a list row opens a new tab; drag a tab out for a window; split view shows two notes. | S | 2 |
| OF02-F014 | Recent and history navigation | `Alt+Left/Right` walks visited notes; sidebar Recent lists the last 30. | M | 1 |
| OF02-F015 | Note statistics | Words, characters, reading time, links in and out, created, modified, revisions, size with attachments. | S | 2 |
| OF02-F016 | Smart lists | Saved searches as sidebar places ("Untagged", "Open tasks", "This week", user-made). | S | 2 |
| OF02-F017 | Daily notes | `Ctrl+Shift+D` opens or creates `daily/YYYY-MM-DD.md` from the daily template; calendar popover jumps to any day; previous and next day buttons. | S | 1 |
| OF02-F018 | Read-only notebooks | A notebook on a read-only handle or inside a snapshot opens locked; edits are refused with a reason. | M | 1 |

### 4.2 Editor

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F020 | Rich Markdown editing | The editor shows formatted text and hides Markdown marks away from the caret ("live" mode). Storage is always Markdown. Uses `richTextEdit` (FUSE-WP-35) with a Markdown reader and writer. | M | 1 |
| OF02-F021 | Source mode | `Ctrl+/` toggles raw Markdown in `codeView` with the Markdown grammar from OF01. Caret position maps both ways. | M | 1 |
| OF02-F022 | Reading mode | `Ctrl+E` toggles a read-only view with wider margins; links are one click. | S | 1 |
| OF02-F023 | Round-trip guarantee | Loading and saving a note without edits leaves the file byte-identical. Edits change only the touched blocks; untouched Markdown keeps its original spelling (list markers, emphasis style, line wrapping). | M | 1 |
| OF02-F024 | Block types | Paragraph, headings 1–6, bullet, numbered and task lists (nested to 10), quote, callout (`> [!note]`, warning, tip, important), code block with language, table, horizontal rule, image, embed, maths block, footnote, definition list, details (collapsible). | M | 1 |
| OF02-F025 | Inline styles | Bold, italic, strike-through, inline code, highlight (`==`), subscript, superscript, link, inline maths, keyboard key, colour-free by design (no arbitrary colours or fonts). | M | 1 |
| OF02-F026 | Markdown shortcuts while typing | `# `, `- `, `1. `, `[] `, `> `, ```` ``` ````, `---`, `**x**`, `_x_`, `` `x` `` convert as typed; `Ctrl+Z` once restores the literal characters. | M | 1 |
| OF02-F027 | Slash menu | `/` at line start opens a block picker with fuzzy filter: heading, list, task, table, code, quote, callout, image, file, sketch, maths, date, template, divider. | S | 1 |
| OF02-F028 | Format toolbar and bubble | A thin toolbar (optional) and a selection bubble with inline styles, link, highlight, turn into ▸. | S | 1 |
| OF02-F029 | Block handles | Hovering shows a handle left of the block: drag to move, click for the block menu (turn into, duplicate, copy link to block, delete, fold). `Alt+Up/Down` moves by keyboard. | S | 2 |
| OF02-F030 | Folding | Fold headings and list items; state kept per note in the index, not in the file. | S | 2 |
| OF02-F031 | Outline | Inspector tab with the heading tree; click jumps; drag moves a section. | S | 1 |
| OF02-F032 | Code blocks | Highlighting by OF01 grammars; language picker; copy button; line numbers optional; wrap toggle. Code is never run. | M | 1 |
| OF02-F033 | Maths | `$…$` and `$$…$$` in a LaTeX subset rendered by the MathScript typesetter (hand-off shared with OF04; Proposed). Unrenderable input shows the source in a warning style. | S | 2 |
| OF02-F034 | Diagrams from text | Fenced `diagram` blocks in a small own grammar (flow, sequence) drawn through `Painter`; other diagram languages show as code. | C | 3 |
| OF02-F035 | Footnotes | `[^id]` with a popover preview and a footnote area at the end. | S | 2 |
| OF02-F036 | Find and replace in note | `Ctrl+F`, `Ctrl+H`; case, whole word, regex (FORGE regex, step-limited). | M | 1 |
| OF02-F037 | Undo and redo | Per note, unlimited for the session, built on the rich text operation log; survives switching notes; cleared on close of the notebook. | M | 1 |
| OF02-F038 | Autosave | Every change is saved within 2 s of idle and on blur, atomically (temporary file, `fsync`, rename). There is no Save command. | M | 1 |
| OF02-F039 | Spell check and autocorrect | Through the spelling service (G-17); per-note language from front matter; autocorrect list is user-editable and off by default. | S | 2 |
| OF02-F040 | Typography helpers | Smart quotes, dashes and ellipsis per language (option); never inside code. | C | 2 |
| OF02-F041 | Paste handling | Paste rich text (HTML offer) converts to Markdown through the import helper's sanitiser; `Ctrl+Shift+V` pastes plain; pasted images become attachments; pasted URL over a selection makes a link. | M | 1 |
| OF02-F042 | Date and time insert | `@today`, `@now`, `@tomorrow`, or a date picker; inserted as ISO text and optionally linked to the daily note. | S | 2 |
| OF02-F043 | Emoji and symbols | `:` completion from the system list; picker hand-off to AX04. | C | 2 |
| OF02-F044 | Word count goal | Optional target per note with a quiet progress chip. | C | 3 |
| OF02-F045 | Focus and typewriter modes | Hide panes; dim other paragraphs; keep the caret line centred. | C | 3 |
| OF02-F046 | Line width and fonts | Content column 60–120 characters or full width; three text faces (system, serif, monospace); size 11–24. | S | 1 |
| OF02-F047 | Raw HTML in notes | Shown as literal text, never interpreted. Imported HTML is converted to Markdown first. | M | 1 |
| OF02-F048 | Long notes | Notes up to 20 MiB of Markdown stay within the frame budget; larger files open in source mode with a notice. | M | 2 |
| OF02-F049 | Editing external changes | A note changed on disk while open reloads if unmodified, else the two are merged block by block; clashes become a conflict (F163). | M | 1 |

### 4.3 Links, backlinks and graph

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F050 | Wiki links | `[[Title]]`, `[[Title|label]]`, `[[Title#Heading]]`, `[[Title#^block]]`. Typing `[[` opens a completer over titles and aliases across open notebooks. | M | 1 |
| OF02-F051 | Markdown links | `[label](relative/path.md)` resolved the same way; external notebooks can be set to write this form only. | M | 1 |
| OF02-F052 | Link resolution | By `id` first, then title, then alias, then path. Ambiguous titles show a picker and store the id. | M | 1 |
| OF02-F053 | Create from link | Following a link to a missing note creates it in the same section after one click on "Create". Unresolved links are styled `--warn`. | M | 1 |
| OF02-F054 | Backlinks panel | Inspector tab listing notes that link here with the surrounding paragraph; "Unlinked mentions" lists plain-text matches of the title with a "Link" button. | M | 1 |
| OF02-F055 | Block references and embeds | `![[Note]]`, `![[Note#Heading]]`, `![[Note#^block]]` show the target inline, read-only, with "Open". Embed depth limit 3; cycles are cut. | S | 2 |
| OF02-F056 | Rename refactoring | Renaming a note or heading rewrites links to it in every unlocked notebook as one undoable action; locked notebooks are listed as pending and fixed on next unlock. | M | 1 |
| OF02-F057 | Hover preview | Hovering a link for 400 ms shows the first 20 lines of the target; `Ctrl+hover` at once. | S | 2 |
| OF02-F058 | External links | `http(s)`, `mailto`, `file` and app schemes emit an `open` intent after showing the full target in the status bar. Notes never fetches a URL itself. | M | 1 |
| OF02-F059 | Links to other apps | Paste a link to a mail (CO01), event (CO02), contact (CO03), file (SY02), PDF page (OF03); shown as a chip with the app icon; opening is by intent. | S | 2 |
| OF02-F060 | Graph view | Notes as nodes, links as edges, force layout computed on a job; filter by notebook, tag, depth from the current note; colour by notebook or tag; click opens. Encrypted locked notebooks are absent. | S | 2 |
| OF02-F061 | Local graph | Inspector tab: neighbours of the current note to depth 1–3. | S | 2 |
| OF02-F062 | Graph list fallback | The same data as an accessible tree ("links to", "linked from") for keyboard and screen reader use. | M | 2 |
| OF02-F063 | Orphans and broken links report | `Tools > Link Report`: notes with no links, unresolved links, duplicate titles; fix actions inline. | S | 2 |

### 4.4 Tags and properties

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F064 | Tags | `#tag` inline and `tags:` in front matter; nested with `/` (`#work/client`); Unicode letters allowed. | M | 1 |
| OF02-F065 | Tag completion | `#` opens a completer with use counts. | M | 1 |
| OF02-F066 | Tag sidebar | Tree of tags with counts; click filters; `Ctrl+click` adds to the filter; AND, OR, NOT. | M | 1 |
| OF02-F067 | Tag management | Rename, merge, delete a tag everywhere (undoable); set a colour token and icon. | S | 2 |
| OF02-F068 | Properties panel | Inspector form over front matter: text, number, date, checkbox, list, link types; add custom keys. | S | 2 |
| OF02-F069 | Table of notes | A view of a section or smart list as a table with front-matter keys as columns; sort and filter; edit cells in place. | C | 3 |
| OF02-F070 | Aliases | Other names a note answers to in links and search. | S | 2 |
| OF02-F071 | Bulk edit | Select many notes: add or remove tags, move, pin, archive, export, delete. | S | 2 |
| OF02-F072 | Tag suggestions | From existing tags by text overlap (no model); model-based suggestions are F192. | C | 3 |

### 4.5 Attachments and media

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F073 | Attach files | Drop, paste or `Ctrl+Shift+A`. The file is copied into `attachments/` under its BLAKE3 hash plus extension; identical files are stored once per notebook. | M | 1 |
| OF02-F074 | Images inline | PNG, JPEG, WebP, GIF (first frame until clicked), SVG subset; decoded by the system image service, not in the app; resize handles write `|width` in the link. | M | 1 |
| OF02-F075 | Image tools | Crop, rotate, alt text, caption; "Edit in" hands off to CR01 or ME01 and takes the result back. | S | 2 |
| OF02-F076 | File cards | Non-image attachments show as a card: icon, name, size, type, trust badge (provenance), Open (intent), Save As, Reveal. | M | 1 |
| OF02-F077 | PDF and document preview | First page thumbnail from `svc.thumbnail`; "Open at page n" links into OF03. | S | 2 |
| OF02-F078 | Audio and video | Inline player controls through `svc.media`; "Record audio" hands off to ME06 and embeds the result; transcript link when ME06 supplies one. | S | 3 |
| OF02-F079 | Scan and photo | "Scan document" (`scan` intent, UT06) and "Take photo" (ME04) insert the result. Notes holds no camera capability. | S | 2 |
| OF02-F080 | Screenshot insert | "Insert screenshot" asks ME05 through an intent. | C | 2 |
| OF02-F081 | Attachment manager | Per notebook: list with size, type, notes using it; find unused; remove unused to trash; total size. | S | 2 |
| OF02-F082 | Size limits | Warn above 100 MiB per file (setting); refuse above the free space minus 1 GiB. | M | 1 |
| OF02-F083 | Untrusted attachments | Files with an untrusted provenance tag open through "Open in a disposable sandbox" by default. Attachments are never executed or previewed with active content. | M | 2 |
| OF02-F084 | Metadata strip | Option to strip EXIF and document metadata on attach (through SE05's cleaner interface). | S | 3 |

### 4.6 Drawings and handwriting

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F085 | Sketch block | An inline drawing area on `canvasView` (FUSE-WP-43): pen, highlighter, eraser (stroke or pixel), lasso select, move, scale. | S | 2 |
| OF02-F086 | Pen input | Pressure and tilt shape the stroke; palm rejection; pen starts drawing with no mode switch; finger scrolls (setting). | S | 2 |
| OF02-F087 | Tools | 5 pen widths, 8 theme-aware colours, ruler, shape snap (line, rectangle, ellipse, arrow) after a 500 ms hold. | S | 2 |
| OF02-F088 | Storage | Each sketch is `attachments/<id>.ink.json` (strokes: points with `x,y,p,tilt,t`) plus a rendered `<id>.svg` so other tools show it. | S | 2 |
| OF02-F089 | Page backgrounds | Blank, ruled, grid, dotted; sizes: flow with the note or fixed A4/Letter pages. | C | 3 |
| OF02-F090 | Handwriting note | A whole note as an infinite vertical canvas with typed blocks allowed between strokes. | C | 3 |
| OF02-F091 | Handwriting search | Optional recognition through `svc.model` writes recognised text into the index only; needs `ai.local`. | C | 3 |
| OF02-F092 | Annotate an image | Draw on top of an attached image; the original is kept and the overlay stored as ink. | S | 3 |
| OF02-F093 | Open in Draw | Convert a sketch to an OF08 document by intent. | C | 3 |
| OF02-F094 | Undo in sketches | Stroke-level undo joined into the note's undo stack. | S | 2 |

### 4.7 Tables and tasks

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F095 | Tables | Pipe tables edited as a grid: Tab between cells, add or remove rows and columns, drag to reorder, column alignment, header row. | M | 1 |
| OF02-F096 | Table tools | Sort by column, sum and average in a footer row (display only, not stored as formulas), paste from OF05 or CSV, copy as CSV. | S | 2 |
| OF02-F097 | Wide tables | Horizontal scroll inside the block; first column can stick. | S | 2 |
| OF02-F098 | Cell content | Inline styles, links, images, line breaks (`<br>` written, shown as a break). No nested tables. | M | 1 |
| OF02-F099 | Open in Sheets | Send a table to OF05 as CSV by intent; paste back replaces it. | C | 3 |
| OF02-F100 | Task items | `- [ ]` and `- [x]`; click or `Ctrl+Enter` toggles; nested tasks; ticking a parent offers to tick children. | M | 1 |
| OF02-F101 | Task states | Extra marks kept as text: `[/]` in progress, `[-]` cancelled, `[>]` deferred. | S | 2 |
| OF02-F102 | Task metadata | Inline `due:2026-11-20`, `prio:high`, `@person` parsed into the index; date picker on click. | S | 2 |
| OF02-F103 | Tasks view | A sidebar place listing open tasks across unlocked notebooks, grouped by note, due date or tag; ticking writes back to the note. | S | 2 |
| OF02-F104 | Send to Tasks | "Send to Tasks" creates a CO04 item with a link back to the block; the note shows the link chip. Reminders belong to CO04. | S | 2 |
| OF02-F105 | Progress chip | Notes with tasks show `3/7` in the list and a bar in the header. | S | 2 |
| OF02-F106 | Checklists from selection | Turn selected lines into tasks and back. | M | 1 |
| OF02-F107 | Completed task handling | Option: move ticked items to the end of their list or strike them. | C | 3 |
| OF02-F108 | Kanban view of a note | Headings as columns, list items as cards; drag rewrites the Markdown. | C | 3 |

### 4.8 Templates and quick capture

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F109 | Templates | Markdown files in `templates/` (per notebook) and in the app config (global). Shipped: Blank, Daily, Meeting, Project, Decision record, Reading note, Journal, Checklist. | S | 1 |
| OF02-F110 | Template variables | `{{date}}`, `{{time}}`, `{{title}}`, `{{notebook}}`, `{{cursor}}`, `{{clipboard}}`, `{{date:+7d}}`, `{{prompt:Label}}` (asks once). No scripting in templates. | S | 1 |
| OF02-F111 | New from template | `Ctrl+Shift+N` opens the template picker; sections can set a default template. | S | 1 |
| OF02-F112 | Insert template | Insert at the caret from the slash menu. | S | 2 |
| OF02-F113 | Save as template | Any note can become a template. | S | 2 |
| OF02-F114 | Quick capture window | A 480×320 window opened by a shell shortcut (default `Super+N`, registered through SH01) or the `compose` intent: one editor, a notebook picker, tags. `Esc` or close saves to the Inbox section. | M | 1 |
| OF02-F115 | Capture from selection | The shell's share sheet (SH04) sends selected text, a file, an image or a URL; Notes makes a note or appends to a chosen one. | M | 1 |
| OF02-F116 | Append to daily note | Capture option "Add to today" with a timestamp line. | S | 2 |
| OF02-F117 | Capture when locked | If the target notebook is locked, the capture is sealed to the notebook's public capture key (F139) and filed on next unlock. | S | 2 |
| OF02-F118 | Launcher actions | SH01 search shows "New note", "Today's note", and note titles from unlocked, non-private notebooks. | S | 2 |
| OF02-F119 | Command line | `notes new [--notebook n] [--title t] [-]`, `notes open <id-or-title>`, `notes search <query>`; the launcher forwards to the running instance. | S | 2 |

### 4.9 Search

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F120 | Full-text search | SQLite FTS5 index over titles, body, tags, attachment names, front matter. Tokeniser: Unicode word breaks from FORGE text; prefix matching; diacritic folding. | M | 1 |
| OF02-F121 | Search as you type | `Ctrl+K` or the header field; results within 50 ms for 100 000 notes; highlighted snippets; Enter opens, `Ctrl+Enter` in a new tab. | M | 1 |
| OF02-F122 | Query language | `tag:`, `-tag:`, `notebook:`, `section:`, `title:`, `created:>2026-01-01`, `modified:<7d`, `has:task|open-task|attachment|image|sketch|link|conflict`, `is:pinned|archived|orphan`, `"phrase"`, `OR`, `-word`, `/regex/`. | M | 1 |
| OF02-F123 | Filters UI | The same filters as chips under the field for users who do not type operators. | S | 2 |
| OF02-F124 | Attachment text | Text of attached PDFs and plain files is extracted by the `svc.thumbnail`/extraction service in its sandbox and indexed (option). Images get OCR text only with `ai.local`. | S | 3 |
| OF02-F125 | Search in encrypted notebooks | Unlocked encrypted notebooks are searched through their own sealed index (F137). Locked ones return nothing and are not named in results. | M | 2 |
| OF02-F126 | Saved searches | See F016. | S | 2 |
| OF02-F127 | Replace across notes | Regex replace over a search result set with a preview of each change; one undo; history versions written first. | S | 3 |
| OF02-F128 | System search | Titles and text of non-encrypted, non-private notebooks are offered to `svc.search` (option per notebook, on by default for plain notebooks, never for encrypted ones). | S | 2 |
| OF02-F129 | Semantic search | Optional: embeddings from `svc.model` stored beside the index; "Related notes" panel. Needs `ai.local`. | C | 3 |
| OF02-F130 | Index rebuild | The index is a cache: rebuilt from files on demand, on schema change, or when an external notebook changed while the app was closed. Progress in the status bar. | M | 1 |
| OF02-F131 | Recent searches | Last 20, clearable; not kept for encrypted notebooks. | S | 2 |

### 4.10 Encrypted notebooks

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F132 | Create encrypted notebook | At creation only. A random 256-bit notebook key is made in the `vault-helper`; it is wrapped by a key held in SE04 (`svc.keyring`), optionally also by a passphrase (Argon2id) and by a hardware token. | M | 2 |
| OF02-F133 | File encryption | Each note and attachment is one file: header (`magic`, `version`, `file_id`, `nonce prefix`), then 64 KiB chunks sealed with XChaCha20-Poly1305 under a per-file key derived from the notebook key (BLAKE3 keyed derive). Chunk index and final flag are authenticated, so truncation and reordering are detected. | M | 2 |
| OF02-F134 | Names hidden | On-disk names are random ids. Titles, the section tree, tags and order live in an encrypted manifest (`manifest.enc`). | M | 2 |
| OF02-F135 | Unlock | Click the notebook: the shell's authentication prompt (SH03) releases the wrap key from the keyring to the helper. The app logic never sees key bytes. | M | 2 |
| OF02-F136 | Lock | Manual (`Ctrl+Shift+L`), on screen lock, on suspend, after idle (default 10 min), on window close (option). Locking closes the notes, wipes plaintext buffers and the helper's keys, and drops the index from memory. | M | 2 |
| OF02-F137 | Sealed index | The FTS index of an encrypted notebook is an encrypted file decrypted only into the helper's memory-backed store while unlocked. | M | 2 |
| OF02-F138 | No plaintext leaks | Encrypted notebooks are excluded from `svc.search`, thumbnails, recent lists outside the app, AI features unless enabled per notebook, crash reports, and clipboard history (copies are flagged sensitive). | M | 2 |
| OF02-F139 | Capture key | Each encrypted notebook has an X25519 capture key pair; the public half lets quick capture and sync write sealed items while the notebook is locked. | S | 2 |
| OF02-F140 | Change or add unlock methods | Add or remove passphrase and tokens, rotate the wrap; re-keying the notebook key rewrites all files in the background, resumable. | S | 3 |
| OF02-F141 | Recovery sheet | Printable recovery code (the notebook key as 24 words) offered at creation; never stored by the app. | S | 2 |
| OF02-F142 | Convert plain to encrypted | Creates a new encrypted notebook and moves notes; states plainly that history, snapshots and backups made before still hold plaintext, with a link to AD03. | S | 3 |
| OF02-F143 | Tamper report | A file that fails authentication is quarantined and listed; the rest of the notebook stays usable. | M | 2 |
| OF02-F144 | Hidden content inside a plain notebook | Not offered. Encryption is per notebook so the boundary is clear. | — | — |

### 4.11 Versions and history

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F145 | Note revisions | A revision is stored when a note is closed, every 10 min of editing, before sync merges and before bulk changes. Blobs are content-addressed (BLAKE3), zstd-compressed, in `.history/` inside the notebook (encrypted in encrypted notebooks). | M | 2 |
| OF02-F146 | History view | Timeline of revisions with author device, size change; side-by-side or inline diff (`diffView`, FUSE-WP-38) on Markdown or rendered text. | M | 2 |
| OF02-F147 | Restore | Restore a revision (the current text becomes a revision first) or copy parts out. | M | 2 |
| OF02-F148 | Retention | Keep all for 7 days, daily for 90 days, monthly for ever (settings); "Prune history" shows space to be freed. | S | 2 |
| OF02-F149 | Snapshot browser | "Earlier snapshots" lists ZFS snapshots holding the notebook (through `svc.backup`); a snapshot opens as a read-only notebook to copy from. | S | 2 |
| OF02-F150 | Deleted notes recovery | From trash, from history (`.history/deleted`), or from snapshots, in that order, in one "Recover notes" dialog. | S | 2 |
| OF02-F151 | Activity log | Per notebook: created, renamed, moved, deleted, synced, conflicted; filter by device and date. Stored in the notebook so it syncs. | C | 3 |
| OF02-F152 | Purge for ever | "Erase this note and its history" removes revisions and index entries, and says that snapshots and other devices keep copies until they sync and prune. | S | 2 |

### 4.12 Sync between own devices

No server account, no vendor service. Two transports; both use the same merge rules.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F153 | Folder sync | A notebook can be bound to a "sync folder" granted through the portal: a removable drive, a network share mounted by SY02, or a folder some other tool replicates. Notes writes a replica there and reconciles on open, on change and on demand. | M | 2 |
| OF02-F154 | Peer-to-peer sync | Two paired devices on the same network sync directly: mutual authentication with device keys, Noise-style handshake (XX pattern, X25519, ChaCha20-Poly1305, BLAKE3 hashing) in the `sync-helper`. No relay, no account, no discovery beyond the local link. | S | 3 |
| OF02-F155 | Device pairing | Device A shows a QR code and a 9-word phrase holding its device public key and a one-time secret; device B scans (ME04 intent) or types it. Both show a 6-digit check to compare. Device keys are stored by SE04. | S | 3 |
| OF02-F156 | Discovery | Paired devices are found by link-local multicast (needs `OF02-GAP-1`) or by a typed address and port. Unpaired devices get no answer. | S | 3 |
| OF02-F157 | What syncs | Notes, attachments, templates, order, tags, history blobs (option), activity log. Not synced: window state, index, recent searches, settings (option). Chosen per notebook. | M | 2 |
| OF02-F158 | Change tracking | Each note has a version vector (`device_id -> counter`) in a sidecar state file (`.sync/state.sqlite` per replica) and a base revision hash for three-way merge. | M | 2 |
| OF02-F159 | Automatic merge | When two devices changed different blocks of a note, a three-way merge against the common base is applied and a revision of each side is kept. Front matter merges per key; tag lists merge as sets. | M | 2 |
| OF02-F160 | Delete versus edit | An edit on one device beats a delete on another: the note comes back, marked "restored by sync". | M | 2 |
| OF02-F161 | Rename and move | Tracked by note id, so a rename on one side and an edit on the other both apply. Two different renames keep the later one by counter and record the other as an alias. | M | 2 |
| OF02-F162 | Attachment sync | Content-addressed, so there are no attachment conflicts; transferred in 1 MiB chunks, resumable; large files can be excluded by size per device. | M | 2 |
| OF02-F163 | Conflicts are shown, never hidden | When the same block changed on both sides, both versions are kept: the note gets a conflict mark, and a conflict view shows mine, theirs and base with per-block "take mine / take theirs / keep both" and free editing. Nothing is discarded until the user resolves it. | M | 2 |
| OF02-F164 | Conflict copies for external tools | In folder sync, if another tool left `name (conflict …).md` files, they are detected and offered in the same conflict view. | S | 3 |
| OF02-F165 | Clock independence | Merge decisions never use wall-clock time; times are shown to the user only as information. | M | 2 |
| OF02-F166 | Sync status | Per notebook chip: Up to date, Syncing n items, Waiting for device, Conflicts (n), Error. A `[net]` badge shows while a peer link is open; click lists the peer address. | M | 2 |
| OF02-F167 | Encrypted notebooks in sync | Replicas hold only ciphertext. The notebook key travels once, during pairing, sealed to the other device's key after the user confirms. A sync folder never holds a key. | M | 3 |
| OF02-F168 | Selective and paused sync | Per device: choose notebooks, pause, sync only on named networks (asked through SY01's network list), only on mains power (`svc.power`). | S | 3 |
| OF02-F169 | Unpair and revoke | Unpairing deletes the peer's key and stops sync. For encrypted notebooks the app offers to re-key (F140), and states that the removed device keeps what it already has. | S | 3 |
| OF02-F170 | Limits stated in the app | The help page and the setup sheet say: sync is not a backup; both devices must be on at the same time for peer sync; a folder replica on a drive is as private as the drive unless the notebook is encrypted; there is no remote wipe. | M | 2 |

### 4.13 Import, export, clipping, print

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F171 | Import Markdown folder | Copy or bind in place (F002). Understands wiki links, `![[embeds]]`, front matter, `#tags`, attachment folders by common conventions; reports links that could not be resolved. | M | 1 |
| OF02-F172 | Import ENEX | Evernote export XML: notes, ENML content to Markdown, resources to attachments (MD5 hash links resolved), tags, created and updated times, source URL, to-do boxes, tables, encrypted spans kept as opaque blocks with a notice. | M | 2 |
| OF02-F173 | Import other apps | Joplin JEX and raw folder; Notion export (zip of Markdown and CSV); Google Keep Takeout JSON; Simplenote JSON; Standard Notes decrypted backup JSON; TextBundle and TextPack; OPML outlines; Zim wiki text; Tomboy/Gnote XML. | S | 3 |
| OF02-F174 | Import documents | HTML, plain text, RTF, ODT and DOCX (through OF04's converter helper interface) become one note each. | S | 3 |
| OF02-F175 | Import sandbox | Every import parser runs in the `import-helper` with no capabilities but the input handle and an output pipe. Output is Markdown plus attachment streams, validated again in the app. | M | 1 |
| OF02-F176 | Import preview and report | Before writing: counts of notes, attachments, tags, warnings. After: a report note listing skipped items and conversions that lost formatting. Import is one undoable action. | M | 2 |
| OF02-F177 | Duplicate detection | Re-importing the same source skips notes whose content hash exists; changed ones are offered as new revisions. | S | 3 |
| OF02-F178 | Export Markdown | Note, selection, section or notebook as a folder or zip: Markdown with relative attachment links; choice of wiki or standard links; front matter on or off. | M | 1 |
| OF02-F179 | Export HTML | One self-contained file per note (images inlined as data URIs, no scripts) or a folder with an index page. | S | 2 |
| OF02-F180 | Export PDF and print | `Ctrl+P` through the print portal (G-16): page size, margins, header and footer, include backlinks list, expand embeds, monochrome code. | S | 2 |
| OF02-F181 | Export other formats | Plain text, ENEX, TextBundle, OPML (outline), EPUB (a section as a book), ODT (through OF04's converter). | C | 3 |
| OF02-F182 | Export encrypted | Export from an encrypted notebook asks to confirm that the output is plaintext; option to write an age-encrypted archive to a recipient key from SE04 instead. | S | 3 |
| OF02-F183 | Web clip receiver | Handles the `share` intent from IN01 with a clip record: `title`, `url`, `captured`, `markdown`, `images[]`, `selection_only`. Notes stores it as a note with `source_url`; images become attachments. Notes makes no request to the web. | M | 2 |
| OF02-F184 | Clip modes | Article (reader text), selection, bookmark only (title, URL, description), full page as attached PDF (rendered by IN01). The mode is chosen in IN01; Notes shows the destination picker. | S | 2 |
| OF02-F185 | Clip safety | The clip's Markdown passes the same sanitiser as paste; links stay inert until clicked; a "From the web" provenance badge stays on the note. | M | 2 |
| OF02-F186 | Share a note | `share` intent with Markdown, HTML and plain-text offers, or a PDF; to CO01, CO05 and others. | S | 2 |
| OF02-F187 | Copy as | Copy selection as Markdown, HTML, or plain text. | S | 1 |
| OF02-F188 | Backup rules | Declares include and exclude rules to `svc.backup`: notebooks and history in, index and caches out. | M | 1 |

### 4.14 Local AI (optional)

All of this needs `ai.local`, is off until the user turns it on, runs through `svc.model` on the device, and can be enabled per notebook. No text leaves the machine.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F189 | Summarise | Summary of a note or selection in a side popover; "Insert below" writes it as a callout marked "Generated". | C | 3 |
| OF02-F190 | Title suggestion | For untitled notes on close; shown as a ghost title the user accepts with Tab. | C | 3 |
| OF02-F191 | Ask my notes | A question answered from retrieved notes with links to each source block; refuses when retrieval finds nothing. | C | 3 |
| OF02-F192 | Tag and link suggestions | Suggested tags and related notes from embeddings. | C | 3 |
| OF02-F193 | Rewrite helpers | Shorten, fix grammar, change tone, translate; always as a diff the user accepts or rejects. | C | 3 |
| OF02-F194 | Extract tasks | Proposes task items found in meeting notes. | C | 3 |
| OF02-F195 | Model and budget display | Shows which model answered, time and memory used; a Stop button; queueing by `svc.model`. | S | 3 |
| OF02-F196 | Generated-text marking | Inserted model output carries an HTML comment marker `<!-- generated: model, date -->` so it can be found and filtered (`has:generated`). | S | 3 |
| OF02-F197 | Encrypted notebooks | AI is off for encrypted notebooks unless enabled for that notebook; embeddings then live in the sealed index. | M | 3 |

### 4.15 General

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| OF02-F198 | Command palette | `Ctrl+K`: notes, commands, tags, templates, settings, help. Prefix `>` for commands, `#` for tags. | M | 1 |
| OF02-F199 | Session restore | Open tabs, scroll, caret, panes return; encrypted notebooks return locked. | M | 1 |
| OF02-F200 | Crash safety | Pending edits are journalled to the app data directory every second; replayed on start. For encrypted notebooks the journal is written through the helper, sealed. | M | 1 |
| OF02-F201 | Themes and density | System tokens; reading and editing use the same style sheet; per-notebook accent colour. | M | 1 |
| OF02-F202 | Network honesty | Without sync the app declares and opens no socket. With peer sync the `[net]` badge is on only while a link is open. | M | 1 |
| OF02-F203 | Integrity check | `Tools > Check Notebook`: file hashes against the manifest, broken attachments, index consistency, history blobs; repairs what it can. | S | 2 |
| OF02-F204 | Signed notes | Optional: sign a note (detached Ed25519 signature with a key from SE04) and verify signed notes; a trust badge shows state. | C | 3 |
| OF02-F205 | Audit records | Unlock, lock, pairing, key transfer, export from an encrypted notebook and re-key are written to `svc.audit`; note titles are not. | M | 2 |
| OF02-F206 | Macros | ForgeScript macros with the same sandbox and signing model as OF01 (empty capability set, `notes::` API over the open note only). | C | 3 |
| OF02-F207 | Help and first run | Three sample notes in a "Welcome" notebook explain links, tags, encryption and sync limits. | S | 1 |
| OF02-F208 | Keyboard reference | `Ctrl+?` lists all shortcuts, searchable. | S | 1 |
| OF02-F209 | Presentation of a note | `F5` when a note has focus shows it full screen, split at horizontal rules, for small meetings. | C | 3 |
| OF02-F210 | Publish to file bundle | "Share notebook copy" writes a signed, optionally age-encrypted archive another PBSD user can import. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Main window
├─ Header bar: menu, title, search/palette, sync chip, [net] badge (peer sync only), lock chip
├─ Sidebar (F9): Places (All, Inbox, Today, Favourites, Tasks, Recent, Smart lists, Trash)
│               Notebooks (tree of sections, lock icons) · Tags (tree) · Devices (sync)
├─ Note list (middle pane) with sort/group bar
├─ Editor pane(s): tab strip · note header (title, tags, progress) · editor · find bar · banners
├─ Inspector (F4): Outline | Backlinks | Local graph | Properties | History | Info
├─ Full views: Graph · Tasks · Table of notes · Conflict view · History compare · Attachment manager
├─ Sheets: New notebook · Encrypt options · Unlock methods · Recovery sheet · Sync setup · Pair device
│          Import (source, preview, report) · Export · Template picker · Link report · Check notebook
├─ Popovers: link completer · tag completer · slash menu · selection bubble · hover preview · date picker
├─ Quick capture window (separate, small)
└─ Settings tab
Drawn by the shell: file portal, share sheet, print dialog, authentication prompt, capability prompts
```

### 5.2 Main window

Shown at 1360×820, three panes plus inspector.

```
+------------------------------------------------------------------------------------------+
| [=] Notes — Research            [ Search notes  Ctrl+K ]      (⟳ Up to date) [▁][▢][✕]   | (1)
+-------------+----------------------+---------------------------------------+-------------+
| PLACES  (2) | Modified v  [+ New]  | Capability model ×  | Reading list    | Backlinks(6)|
|   All notes |----------------------+---------------------------------------+-------------+
|   Inbox   3 | > Capability model   | # Capability model                    | Handles     |
|   Today     |   Rights only narrow…|  #security #pbsd            3/7 tasks |  "…see      |
|   Tasks  12 |   2 h · #security 📎2|                                       |  [[Capabil… |
| NOTEBOOKS   |----------------------|  Rights **only narrow**. See          |  model]] …" |
| v Research  |   Handles            |  [[Handles]] and [[Lineage#Revoke]].  | Lineage     |
|     Papers  |   A handle is…       |                                       |  "…as in …" |
|     Ideas   |   Yesterday · #pbsd  |  - [x] read INVARIANTS.md             |-------------|
| > Journal 🔒|----------------------|  - [ ] draw the lineage tree          | Unlinked (2)|
|   Work      | ! Reading list   ⚠   |                                       |  SI-2 notes |
| TAGS        |   conflict · 1 block |  | Right | Narrows |                  |   [Link]    |
|   security 9|----------------------|  |-------|---------|                  |             |
|   pbsd    14|   Daily 2026-11-12   |  | read  | yes     |                  |             |
| DEVICES     |   …                  |  [sketch: lineage tree]               |             |
|   desktop ✓ |         (3)          |                 (4)                   |     (5)     |
+-------------+----------------------+---------------------------------------+-------------+
| 412 words · 3 links out · 6 in · saved 14:02 · en_AU                       Research · MD | (6)
+------------------------------------------------------------------------------------------+
```

1. Header bar (`headerBar`). Sync chip is a `toolButton` opening the sync popover. `[net]` appears only while a peer link is open.
2. Sidebar (`sidebar` with `treeView`, FUSE-WP-28): places, notebooks with lock icons, tag tree with counts, paired devices.
3. Note list (`modelListView` over `SqliteModel`), 280 px, variable row content, conflict rows marked with icon and word.
4. Editor (`richTextEdit`, FUSE-WP-35) under a `tabView`; note header has title (`lineEdit`), tag chips (`tagInput`), task progress (`progressBar`).
5. Inspector (`inspector` with `tabBar`): backlinks list with context, unlinked mentions.
6. Status bar (`statusBar`): counts, save time, language, notebook, mode (MD = live Markdown; SRC; READ).

States: empty app (no notebooks: "Create your first notebook" + [New Notebook] [Import…]); empty list ("No notes here" + [New Note]); loading (index rebuild progress in status bar, list fills); locked notebook (5.3); read-only; error banner; narrow.

### 5.3 Other views and dialogs

**Locked notebook** (in place of list and editor).

```
+----------------------+----------------------------------------------------+
|                      |                     🔒                             |
|   Journal is locked  |     "Journal" is encrypted and locked.             |
|                      |     Nothing in it is readable or searchable.       |
|                      |              [Unlock…]                             |
|                      |     Locks after 10 min idle · on screen lock       |
|                      |     2 captures waiting to be filed                 |
+----------------------+----------------------------------------------------+
```

[Unlock…] calls the shell prompt (SH03). States: locked, unlocking, wrong credential (message from the shell), tamper report present.

**Conflict view** (full pane, 1200×640).

```
+------------------------------------------------------------------------------------------+
| Reading list — 1 conflicting block · laptop (you) ↔ desktop        [Keep both] [Done]    |
+-----------------------------+-----------------------------+------------------------------+
| Mine (laptop, 13:50)        | Result                      | Theirs (desktop, 13:47)      |
+-----------------------------+-----------------------------+------------------------------+
| ## To read                  | ## To read                  | ## To read                   |
| - Capsicum paper            | - Capsicum paper            | - Capsicum paper             |
|>- seL4 proof overview   [→] | - seL4 proof overview       | - CHERI report           [←]<|
|                             | - CHERI report              |                              |
| - PRISM manual              | - PRISM manual              | - PRISM manual               |
+-----------------------------+-----------------------------+------------------------------+
| Base: common version from 12:10 [Show]     Merged automatically: 4 blocks [Review]       |
+------------------------------------------------------------------------------------------+
```

Three `richTextEdit` panes (outer two read-only), sync-scrolled. Result is editable. "Done" stores both originals as revisions.

**Graph view** (full pane).

```
+------------------------------------------------------------------------------------------+
| Graph   Notebook [Research v]  Tags [security ×] [+]  Depth [2 v]  Colour by [Tag v] [List]|
+------------------------------------------------------------------------------------------+
|              o Lineage                                                                   |
|             / \                           o Daily 2026-11-12                             |
|   Handles o---● Capability model ---o INVARIANTS                                         |
|             \        |                                                                   |
|              o SI-2  o Reading list ⚠                 · · (3 orphans)                    |
+------------------------------------------------------------------------------------------+
| 42 notes · 61 links · click opens · drag moves · wheel zooms                              |
+------------------------------------------------------------------------------------------+
```

`canvasView` (FUSE-WP-43) vector layer. [List] switches to the accessible tree (F062). States: computing layout, empty ("No links yet").

**Sync setup sheet** (560×460).

```
+----------------------------------------------------------+
| How should "Research" sync?                              |
| ( ) Not synced                                           |
| (•) Through a folder   [ /media/usb0/notes-sync   ][…]   |
| ( ) Directly with a paired device   [desktop v] [Pair…]  |
| What  [x] Notes  [x] Attachments (≤ [100] MiB)  [ ] History |
| When  [x] On change  [ ] Only on mains power             |
| i Sync is not a backup. Conflicts are kept and shown.    |
| i This notebook is not encrypted: the folder holds       |
|   readable files.                                        |
|                                [Cancel] [Start Syncing]  |
+----------------------------------------------------------+
```

**Pair device sheet**: a QR `image`, the 9-word phrase in a monospace `label`, then the 6-digit comparison with [They Match] [They Differ].

**Import sheet** (640×480).

```
+----------------------------------------------------------------+
| Import                                                         |
| Source  [ Evernote (.enex)        v]  File [ work.enex ][…]    |
| Into    [ Work v]  section [ Imported 2026-11-12 ]             |
| Parsed in a sandbox with no file or network access.            |
| Found: 1 204 notes · 3 380 attachments (1.2 GiB) · 87 tags     |
| ! 12 notes hold Evernote-encrypted text (kept as opaque blocks)|
| ! 3 attachments over 100 MiB                 [Skip v]          |
| [x] Keep created and modified times  [x] Keep source URLs      |
|                                         [Cancel] [Import]      |
+----------------------------------------------------------------+
```

**Quick capture window** (480×320).

```
+----------------------------------------------+
| Quick note                    [Inbox v]  [x] |
+----------------------------------------------+
| Ring the vet about Tuesday|                  |
|                                              |
+----------------------------------------------+
| #  tags…          [Add to today]   Esc saves |
+----------------------------------------------+
```

**History compare**: left a `listView` of revisions (time, device, ±size), right a `diffView`; [Restore This Version] [Copy Selection].

### 5.4 Menus

```
File    New Note Ctrl+N · New from Template… Ctrl+Shift+N · New Notebook… · New Section · Today's Note Ctrl+Shift+D
        Quick Capture · Import… · Export… Ctrl+Shift+E · Print… Ctrl+P · Share…
        Lock Notebook Ctrl+Shift+L · Lock All · Notebook Settings… · Close Tab Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut · Copy · Copy As ▸ · Paste Ctrl+V · Paste Plain Ctrl+Shift+V
        Find Ctrl+F · Replace Ctrl+H · Find in All Notes Ctrl+K · Replace Across Notes…
        Select All Ctrl+A · Select Block Ctrl+L
Format  Bold Ctrl+B · Italic Ctrl+I · Strike Ctrl+Shift+X · Code Ctrl+` · Highlight Ctrl+Shift+H · Link Ctrl+Shift+K
        Heading 1–6 Ctrl+Alt+1…6 · Paragraph Ctrl+Alt+0 · Bullet List Ctrl+Shift+8 · Numbered Ctrl+Shift+7
        Task Ctrl+Shift+9 · Toggle Task Ctrl+Enter · Quote Ctrl+Shift+. · Code Block Ctrl+Alt+C · Callout ▸
        Indent Tab · Outdent Shift+Tab · Move Block Up/Down Alt+Up/Down · Clear Formatting Ctrl+\
Insert  Link to Note [[ · Table… · Image… · File… Ctrl+Shift+A · Sketch · Maths · Date ▸ · Template ▸
        Footnote · Divider · Scan… · Photo… · Screenshot… · Audio Recording…
Note    Pin · Favourite · Move To… · Duplicate · Merge Selected · Split at Headings… · Archive · Delete Del
        Rename F2 · Properties · History… Ctrl+Alt+H · Send Tasks to Tasks · Sign Note · Erase with History…
View    Sidebar F9 · Note List Ctrl+Shift+B · Inspector F4 · Source Mode Ctrl+/ · Reading Mode Ctrl+E
        Outline · Backlinks · Local Graph · Graph · Tasks · Table of Notes · Focus Mode Ctrl+Shift+F11
        Split Right Ctrl+Alt+\ · Zoom Ctrl++/-/0 · Full Screen F11 · Present F5
Tools   Sync Now · Sync Settings… · Devices… · Conflicts (n)… · Link Report · Attachment Manager
        Check Notebook · Rebuild Index · Prune History… · Summarise (local AI) · Ask My Notes · Macros ▸ · Settings Ctrl+,
Help    Notes Help F1 · Keyboard Reference Ctrl+? · Markdown Reference · Sync and Its Limits · About
```

"Graph" has no default key because `Ctrl+G` is reserved for find next.

### 5.5 Keyboard and pointer

- `[[` link completer; `#` tag completer; `/` slash menu; `Ctrl+Enter` toggles the task at the caret; `Ctrl+click` or `Ctrl+Enter` on a link follows it; `Alt+Enter` opens it in a new tab.
- List pane: arrows move, Enter opens, `Ctrl+Enter` new tab, `F2` rename, `Delete` to trash, type-ahead by title.
- Context menu, note row: Open, Open in New Tab, Open in New Window, Pin, Favourite, Move To, Duplicate, Copy Link, Copy as Markdown, Export, History, Archive, Delete.
- Context menu, editor: Cut, Copy, Paste, Paste Plain, spelling suggestions, Turn Into ▸, Link, Copy Link to Block, Insert ▸, Summarise Selection (if AI is on).
- Context menu, notebook: New Note, New Section, Rename, Colour, Lock, Sync Settings, Export, Check, Archive, Delete.
- Context menu, attachment: Open, Open in Disposable Sandbox, Save As, Reveal, Copy, Replace, Remove.
- Drag and drop: notes onto sections, notebooks and tags (adds the tag); files and images into the editor (attach); text from other apps (new block); a note onto another note's editor (inserts a link; with `Alt`, an embed); list rows out to SY02 (exports Markdown); blocks by their handle.
- Pen: draws in sketch blocks; on text, a pen stroke starts a new sketch block at that line (setting).
- Touch: swipe a list row right to pin, left to archive (touch density only); pinch zooms text.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | See 5.2. A new notebook starts with one blank note open and focused. |
| Loading | List shows cached rows at once; "Indexing 2 310 of 9 000" in the status bar; search says "partial results" until done. |
| Error | Save failure: banner on the note, text kept in the journal, retry every 10 s. Corrupt file: opened in source mode with a banner. |
| Offline | Normal. Peer sync shows "Waiting for device". Nothing else needs a network. |
| Locked | Encrypted notebook view in 5.3; its notes close; links to it from other notebooks show "locked note". |
| Read-only | Lock chip; editor refuses input with a toast that says why (snapshot, read-only folder, conflict under review). |
| Conflict | Row and tab marked; banner "This note has 1 conflicting block" with [Resolve]. Editing continues on "mine". |
| Sync folder missing | Chip "Folder not found" with [Locate…]; local work continues. |
| Narrow (< 640) | One pane at a time: sidebar → list → editor with a back button; inspector as a sheet. |

### 5.7 Widgets

| Region | Widget | Exists today (V3) | Needs |
|---|---|---|---|
| Editor | `richTextEdit` | no (G-10) | FUSE-WP-35; Markdown reader and writer with source-preserving spans (`OF02-GAP-2`) |
| Source mode | `codeView` | `codeEdit` only | FUSE-WP-37 |
| Embeds, previews, reading mode | `markdownView` / read-only `RichDoc` | partial | FUSE-WP-42 |
| Sketch, graph | `canvasView` | no (G-12) | FUSE-WP-43, FUSE-WP-08 (pen) |
| Note list | `modelListView` + `SqliteModel` | yes, single select, fixed rows | FUSE-WP-28 (multi-select, variable rows, drag) |
| Sidebar trees | `treeView` | 64 collapsible rows limit | FUSE-WP-28 |
| Tag chips | `tagInput` | no | FUSE-WP-29 |
| History, conflicts | `diffView` | no | FUSE-WP-38 |
| Tables in notes | rich text table blocks | no | FUSE-WP-35 |
| Chrome, popovers, sheets, palette, banners | chrome kit | no | FUSE-WP-29, FUSE-WP-30 |
| Date picker, calendar popover | `calendar`, `dateTimeEdit` | yes | — |
| Properties | `propertyGrid` | yes | — |

`OF02-GAP-1`: a capability and portal for link-local discovery (multicast) limited to one service name; PLATFORM.md section 6 has no such grant. `OF02-GAP-2`: the rich text engine must keep, per block, the original source text and a "dirty" flag so untouched blocks are written back verbatim (F023); not stated in FUSE-WP-35.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Notebooks, note model, link graph, queries, templates, merge policy, UI glue |
| Views and styles | FUSE UI | app host | All windows and panes |
| Rich text, code, diff, canvas engines | C++23 (FUSE library) | app host | Editing and drawing; platform code, not in this bundle |
| Markdown block codec | C++23 (part of FUSE-WP-42, extended) | app host | Parse to blocks with source spans; write changed blocks |
| `vault-helper` | C++23 | helper, capability mode, no network | Holds notebook keys; encrypts and decrypts files, manifest, journal and sealed index for encrypted notebooks; key wrap and unwrap with `svc.keyring` |
| `import-helper` | C++23 | helper, capability mode, no file or network access beyond passed handles | ENEX and other import parsers; HTML-to-Markdown sanitiser for paste and clips; zip and tar readers |
| `sync-helper` | C++23 | helper; the only process with sockets | Peer transport, handshake, chunk transfer, folder replica scan; sees ciphertext only for encrypted notebooks |
| Index | SQLite FTS5 | app host (plain), `vault-helper` (encrypted) | Search, backlinks, tags, tasks, graph edges |

```
 IN01 ──share intent (clip)──▶ app host ◀──▶ svc.settings / keyring / notify / search / backup / model
 SH04 portals ──handles──────▶ │ ForgeScript logic ─ richTextEdit, codeView, canvasView, diffView
                               │ index (SQLite FTS5)      notebooks: Markdown files + attachments
                               ├──▶ import-helper   (untrusted bytes in, Markdown out; no caps)
                               ├──▶ vault-helper    (keys, AEAD; no net)  ◀── wrap key ── svc.keyring
                               └──▶ sync-helper     (sockets to paired peers only; no keys for content)
```

### 6.2 Data model

On disk, per notebook:

```
<notebook>/
  notebook.json      {id, name, colour, icon, created, format_version, encrypted:false, default_template, sort}
  <section>/…/<slug>.md          front matter + Markdown
  <section>/.order               list of file names
  attachments/<blake3>.<ext>     content-addressed
  attachments/<id>.ink.json      strokes; <id>.svg rendering
  templates/*.md
  .history/<note_id>/<time>-<device>.zst , .history/deleted/…
  .sync/state.sqlite , .sync/base/<note_id>   version vectors and merge bases
  .activity.log                  JSON lines
```

Encrypted notebook: `notebook.json` (clear: `id`, `format_version`, `encrypted:true`, `kdf`, wrapped keys, capture public key), `manifest.enc`, `objects/<random-id>` (sealed files), `inbox/<random-id>` (sealed captures), `index.enc`.

Index tables (cache, `~/.cache/org.pbsd.notes/index-<notebook>.sqlite`, schema version in `meta`):

| Table | Fields |
|---|---|
| `note` | `id: text pk`, `path: text`, `title: text`, `created: i64`, `modified: i64`, `pinned: bool`, `archived: bool`, `word_count: u32`, `task_open: u16`, `task_done: u16`, `content_hash: blob(32)`, `conflict: bool`, `lang: text` |
| `note_fts` | FTS5: `title`, `body`, `tags`, `attachment_names` |
| `link` | `src_id`, `dst_id` (nullable when unresolved), `dst_text`, `kind: u8` (wiki, markdown, embed, block), `anchor: text`, `src_block: u32` |
| `tag` | `note_id`, `tag: text` |
| `task` | `note_id`, `block: u32`, `state: u8`, `text`, `due: i64?`, `prio: u8?` |
| `attachment` | `hash: blob(32) pk`, `ext`, `size: u64`, `mime`, `provenance: text` |
| `attachment_use` | `note_id`, `hash` |
| `block_ref` | `note_id`, `block_id: text`, `offset: u64` |
| `embedding` (optional) | `note_id`, `block: u32`, `model: text`, `vec: blob` |

Sync state (`.sync/state.sqlite`): `replica(device_id, name, public_key, last_seen)`, `version(note_id, device_id, counter)`, `base(note_id, hash)`, `tombstone(note_id, vector, time)`, `pending_conflict(note_id, mine_hash, theirs_hash, base_hash)`.

App data (`~/.local/share/org.pbsd.notes/`): notebook registry (`notebooks.json`: id, location or portal grant token, sync settings), journals, global templates. Migration: `format_version` in `notebook.json`; forward only; a notebook from a newer version opens read-only.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Markdown (CommonMark + tables, tasks, footnotes, wiki links, callouts) | yes | yes | FUSE-WP-42 parser, extended | app host (fuzzed, bounded nesting) |
| Front matter (YAML subset: scalars, lists, flat maps) | yes | yes | own code | app host |
| ENEX / ENML | yes | export | own XML reader | `import-helper` |
| HTML (paste, clips, import) | yes | export | own tokeniser and allow-list converter, no script, no CSS evaluation | `import-helper` |
| JEX (tar), Notion (zip), TextPack (zip) | yes | TextBundle | own tar reader; zip through UT01's archive helper interface or own inflate | `import-helper` |
| Keep, Simplenote, Standard Notes JSON; OPML; Zim; Tomboy | yes | OPML | own code | `import-helper` |
| Ink JSON, SVG subset | yes | yes | own code; FUSE-WP-27 for SVG display | app host (own files); `import-helper` for foreign SVG |
| Images | view | — | system image service (out of process) | image service sandbox |
| Encrypted object format (F133) | yes | yes | Monocypher (BSD-2-Clause/CC0; already linked by FORGE, Verified `FORGE:docs/os.md`) for XChaCha20-Poly1305, X25519, Argon2id; BLAKE3 | `vault-helper` |
| age (export) | no | yes | own code on the same primitives (Proposed) or SE05's interface | `vault-helper` |
| Peer sync protocol (framed records over the Noise-style channel, TCP) | yes | yes | own code | `sync-helper` |
| zstd (history) | yes | yes | zstd (BSD-3-Clause, base system) | app host / `vault-helper` |
| PDF | no | print | print pipeline G-16 | print service |

### 6.4 Integration

- Intents handled: `open` and `view` (`text/markdown`, `application/x-enex+xml`, notebook folders), `new`, `compose` (quick capture), `share` (text, URL, image, file, web clip record), `search`. Emitted: `open` (links, attachments, OF03 page links), `share`, `print`, `scan` (UT06), `compose` task to CO04, capture requests to ME04, ME05, ME06.
- URL scheme `notes:` (`notes://<notebook-id>/<note-id>#<block>`) so other apps can link to a note. A link into a locked notebook opens the unlock view.
- Services: `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.notify` (sync conflicts, import done), `svc.search` (plain notebooks only), `svc.keyring`, `svc.clipboard`, `svc.thumbnail`, `svc.media`, `svc.power`, `svc.backup`, `svc.undo-trash`, `svc.audit`, `svc.a11y`, `svc.model` (optional), spelling service (G-17).
- Other apps: SE04 (wrap keys, device keys, signing keys), IN01 (clips), CO04 (tasks), OF03 (PDF pages), OF04 (document conversion, maths), OF08 (sketch hand-off), AD03 (snapshots and backup), SY02 (reveal, mounts), SE05 (metadata cleaner).
- Verified in repos: FORGE text segmentation and regex (`FORGE:docs/text-locale.md`); Monocypher-based sealing in FORGE's file keystore (`FORGE:docs/os.md`); script signing (`FORGE:docs/scripting.md`); FUSE has `markdownView` with the limits in `widgets.hpp:249-252` and no rich text widget. Everything else here is Proposed.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Default notebooks, registry, journals, index cache |
| `fs.portal` | External notebooks, sync folders, import sources, export targets, attachments picked by the user |
| `proc.helper:vault-helper`, `proc.helper:import-helper`, `proc.helper:sync-helper` | Section 6.1 |
| `net.none` | Default. Stays in force unless peer sync is enabled |
| `net.user` | Only when the user enables peer sync: endpoints are the paired devices the user added; granted to `sync-helper` alone; each first contact is logged |
| `dev.print` | Printing |
| `ai.local`, `svc.model` | Optional AI features of 4.14; off by default |
| `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.notify`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.thumbnail`, `svc.media`, `svc.power`, `svc.backup`, `svc.undo-trash`, `svc.audit`, `svc.a11y`, `svc.spell` | Section 6.4 |

Not requested: `proc.exec`, camera, microphone, screen, location, USB, `bg.run`, `bg.autostart`, `net.host`. Sync runs only while the app is open; this is stated in the app.

Assets: note text and attachments; notebook keys; device identity keys; the link graph (it reveals interests even without text).

| Threat | Mitigation |
|---|---|
| Hostile import file or web clip (XML bombs, malformed zip, script in HTML) | Parsed only in `import-helper` with no capabilities; entity expansion off; size, depth and count limits; output re-validated; HTML reduced to Markdown by allow-list |
| Hostile Markdown in a synced or external notebook | No HTML rendering, no remote fetch, no code execution; links inert until clicked and shown in full; embed depth limit |
| Theft of the disk or sync drive | Encrypted notebooks: content, names and structure sealed; plain notebooks: the app says they are readable |
| Malware in another app reading notes | Notebooks live under `fs.app` or user-granted folders; other apps have no handle; `notes:` links carry ids, not content |
| Key exposure in the app process | Keys exist only in `vault-helper` (`secure_buffer`, no dumps); script side holds opaque handles; lock wipes them |
| Rogue or stolen paired device | Pairing needs physical comparison; unpair and re-key; honest statement that already-synced data cannot be recalled |
| Network attacker on the LAN | Mutual authentication with pinned device keys; forward secrecy from ephemeral X25519; unpaired peers get no response; no listening socket unless peer sync is on |
| Tampered replica | AEAD on encrypted notebooks; content hashes and version vectors on plain ones; integrity check tool; conflicts instead of silent overwrite |
| Rollback of a replica to an old state | Version vectors make an old replica merge as old, not win; tombstones stop resurrection after their window (90 days, stated) |
| Plaintext left behind (history, snapshots, index, clipboard, system search) | F138; warnings at F009 and F142; purge tools; sealed index |
| Prompt injection through note text into AI features | Model output is never executed, never triggers actions, and is always shown as a proposal |

Stored: notebooks where the user put them; index in cache; registry and journals in app data. Leaves the machine: nothing by default; with sync, notebook data to the sync folder or paired device only.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `editor.mode` | enum live, source, reading | live | Default mode for opened notes |
| `editor.font` | enum system, serif, mono | system | |
| `editor.font_size` | int 11–24 | 15 | |
| `editor.line_width` | int 60–120 or 0 | 80 | 0 means full width |
| `editor.smart_typography` | bool | false | |
| `editor.spellcheck` | bool | true | |
| `editor.autosave_idle_ms` | int | 2000 | |
| `links.format` | enum wiki, markdown | wiki | Per notebook override |
| `links.update_on_rename` | bool | true | |
| `notes.new_location` | enum current_section, inbox | current_section | |
| `notes.title_from_first_line` | bool | true | |
| `daily.folder` | string | `daily` | |
| `daily.template` | string | `Daily` | |
| `capture.shortcut` | shortcut | `Super+N` | Registered with the shell |
| `capture.target` | notebook id | Inbox of the default notebook | |
| `attachments.warn_bytes` | int | 104857600 | |
| `attachments.strip_metadata` | bool | false | |
| `trash.purge_days` | int | 30 | |
| `history.enabled` | bool | true | Per notebook override |
| `history.keep_all_days` | int | 7 | |
| `history.keep_daily_days` | int | 90 | |
| `search.index_attachments` | bool | false | |
| `search.offer_to_system` | bool | true | Plain notebooks only |
| `encryption.idle_lock_minutes` | int 1–240 | 10 | |
| `encryption.lock_on_screen_lock` | bool | true | |
| `encryption.lock_on_close` | bool | false | |
| `sync.mode` | enum none, folder, peer | none | Per notebook |
| `sync.include_history` | bool | false | |
| `sync.attachment_max_bytes` | int | 104857600 | Per device |
| `sync.mains_only` | bool | false | |
| `sync.listen_port` | int | 0 (chosen at start) | Peer sync only |
| `ai.enabled` | bool | false | |
| `ai.notebooks` | list of ids | [] | Notebooks AI may read |
| `appearance.show_toolbar` | bool | false | |

## 9. Accessibility and languages

- The editor exposes headings, lists, tables, links and task state as a11y roles and text attributes (needs FUSE-WP-55). Task toggles are announced.
- The graph has a full list equivalent (F062). Conflict view panes are labelled "mine", "result", "theirs"; each difference is reachable with `F7` / `Shift+F7`.
- Sketches carry an optional text description; without it they are announced as "sketch, n strokes".
- Right-to-left notes: block direction follows the first strong character; needs G-03.
- Tags, titles and search fold case and diacritics by Unicode rules; no ASCII assumptions.
- Per-note language drives spelling, hyphenation in print, smart quotes and text-to-speech (AX01).

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | ≤ 250 ms with 10 000 notes (list from the cached index) |
| Quick capture window | ≤ 150 ms from the shortcut when the app is running, ≤ 300 ms cold |
| Open a 50 KiB note | ≤ 30 ms |
| Key to paint | ≤ 4 ms in a 20 MiB note |
| Search, 100 000 notes | first results ≤ 50 ms |
| Backlinks for a note | ≤ 10 ms from the index |
| Index rebuild | ≥ 5 000 notes/s plain; ≥ 1 000 notes/s encrypted |
| Graph layout, 5 000 nodes | first stable picture ≤ 1 s on a job; pan and zoom frame ≤ 4 ms |
| Unlock an encrypted notebook of 10 000 notes | ≤ 500 ms after authentication (manifest and index only) |
| Encrypt or decrypt | ≥ 500 MiB/s per core |
| Folder sync scan, 10 000 notes unchanged | ≤ 1 s |
| Memory | ≤ 150 MiB with 10 000 notes indexed and 5 tabs |
| Idle | zero frames, zero wake-ups; sync wakes only on file events or peer data |

## 11. Testing and proof

Nothing has run. All rows are plans.

- Unit (ForgeScript): link resolution, rename refactoring, query parser, template variables, task parsing, merge policy, retention rules.
- Round-trip corpus: 5 000 Markdown files from varied sources load and save byte-identical; single-block edits change only that block.
- Golden renders: each block type, both themes, locked view, conflict view, graph, quick capture.
- Scripted UI through the a11y tree: tasks T1–T15 with step counts.
- Fuzz: Markdown parser, front matter, ENEX, HTML converter, zip and tar readers, each JSON importer, ink JSON, encrypted object reader, sync record decoder.
- Sync model tests: random edit, rename, delete and merge schedules across three simulated replicas; properties: replicas converge, no edit is lost without a conflict record, no wall-clock dependence.
- Crypto tests: published vectors for XChaCha20-Poly1305, X25519, Argon2id, BLAKE3; truncation, reorder and bit-flip are detected; lock leaves no key in a memory scan of the helper.
- Differential: CommonMark spec examples; ENEX import compared with a hand-checked set.
- Confinement: app host opens no socket; `import-helper` cannot open any path; `sync-helper` cannot read plaintext of encrypted notebooks.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Encrypted object codec (`vault-helper`) | No out-of-bounds; any modified chunk is rejected; nonce never reused under one key | T2, T1 for nonce uniqueness | PRISM on the C++23 codec | Primitive correctness rests on Monocypher (third party) |
| Key handling | Keys never cross to the app host; wiped on lock | T3 | IPC schema review, memory-scan test | Needs information-flow tooling |
| Version-vector merge | Convergence; lost-update freedom | T1 (model) | PRISM or a model checker on the merge function | Policy code is ForgeScript; port the core to C++23 to prove it |
| Three-way block merge | Output contains every non-conflicting change; termination | T2 | PRISM plus property tests | — |
| Markdown block codec | Bounded reads and nesting; untouched blocks byte-identical | T2 | PRISM; round-trip corpus | — |
| Import parsers | Absence of runtime errors | T2 | PRISM plus fuzz | Format breadth |
| Sync transport | Handshake state machine has no unauthenticated data path | T2 | PRISM on the state machine; protocol review | No formal protocol proof tool chosen |
| App logic | — | T4 | Tests | Interpreted |

## 12. Build plan

Phase 1, usable core (plain notebooks, no sync):

| ID | Work item | Depends on |
|---|---|---|
| OF02-W01 | Bundle, manifest, actions, three-pane window, settings | G-02, G-07, FUSE-WP-28, FUSE-WP-29, FUSE-WP-30 |
| OF02-W02 | Notebook and note store; front matter; autosave; journal | FORGE-WP-20, SH04 |
| OF02-W03 | Markdown block codec with source-preserving spans | FUSE-WP-42, OF02-GAP-2 |
| OF02-W04 | Rich editor in live mode: blocks, inline styles, typing shortcuts, slash menu, tables, tasks | G-10 (FUSE-WP-35), W03 |
| OF02-W05 | Source and reading modes | FUSE-WP-37, OF01 Markdown grammar |
| OF02-W06 | Index, search, query language, tags | W02 |
| OF02-W07 | Links, completer, backlinks, rename refactoring | W06 |
| OF02-W08 | Attachments and images | system image service, `svc.thumbnail` |
| OF02-W09 | Templates, daily notes, quick capture, share receiver | SH01 (shortcut), SH04 (share sheet) |
| OF02-W10 | `import-helper` with Markdown-folder import and HTML paste sanitiser; Markdown export | FORGE-WP-27, FORGE-WP-28, G-06 |
| OF02-W11 | Trash, session restore, backup rules, help notebook | G-09 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| OF02-W12 | `vault-helper`; encrypted notebooks; lock rules; sealed index; capture key | SE04, SH03, FORGE-WP-22 |
| OF02-W13 | Revisions, history view, retention, snapshot browser | FUSE-WP-38, AD03, `svc.backup` |
| OF02-W14 | Sync core: version vectors, bases, three-way merge, conflict view | W13 |
| OF02-W15 | Folder sync | W14 |
| OF02-W16 | ENEX import; import preview and report | W10 |
| OF02-W17 | Web clip receiver | IN01 clip record schema, W10 |
| OF02-W18 | Sketch blocks and pen | G-12 (FUSE-WP-43), FUSE-WP-08 |
| OF02-W19 | Graph, local graph, link report | FUSE-WP-43, W07 |
| OF02-W20 | Tasks view, Send to Tasks, properties, smart lists, bulk edit | CO04 |
| OF02-W21 | Spell check, maths, footnotes, embeds, hover preview | G-17, MathScript typesetter, W07 |
| OF02-W22 | Print, HTML export, share, system search | G-16, `svc.search` |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF02-W23 | `sync-helper` peer transport, pairing, discovery | OF02-GAP-1, FORGE-WP-21, SE04, SE02 |
| OF02-W24 | Encrypted notebooks over sync; key transfer; unpair and re-key | W12, W23 |
| OF02-W25 | Other importers and exporters | W16, OF04 converter |
| OF02-W26 | Local AI features | G-15, FORGE-WP-29, FORGE-WP-30, AI02 |
| OF02-W27 | Handwriting notes, recognition, image annotation | W18, W26 |
| OF02-W28 | Table of notes, kanban, presentation, macros, signed notes, shared archive | W20 |

## 13. Open questions and risks

1. Byte-identical round trip (F023) constrains the rich text engine (OF02-GAP-2). If FUSE-WP-35 cannot carry source spans, live mode must be built on the code editor with decorations instead.
2. Peer discovery needs multicast, which the capability model does not grant (OF02-GAP-1). Without it, peer sync needs a typed address.
3. Peer sync makes Notes a network app. An alternative is to move the transport into IN03 Transfers ("local device send") and keep Notes at `net.none`; this needs a decision.
4. Sync only while the app is open is simple and honest but slow to converge. A `bg.run` sync agent is the alternative and costs an always-on listener.
5. Tombstone window (90 days): a device offline for longer can resurrect deleted notes. The app warns when a replica is that stale.
6. Encrypted-notebook index in memory limits notebook size by RAM; a paged, per-page-sealed index is the later answer.
7. The web clip record is a contract with IN01 that neither spec has frozen.
8. Maths rendering depends on a MathScript typesetter not read here (Unverified).
9. Whether `svc.keyring` can release a key straight to a helper process without passing through the app host is not specified in PLATFORM.md.
