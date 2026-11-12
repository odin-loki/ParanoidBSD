<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Help

By Odin Loch

| | |
|---|---|
| ID | `UT05` (from CATALOGUE.md) |
| Bundle | `org.pbsd.help` |
| Category | utils |
| Priority | P1 |
| Backing repos | HardenedBSD base: `hbsd/src/contrib/mandoc` (mandoc, ISC-style licence in `LICENSE`, Verified); FUSE (`markdownView`: Verified; rich-content path planned as FUSE-WP-42); FORGE (`svc.model`, embeddings: Proposed, FORGE-WP-29..31) |
| Status | Planned. Nothing built. |

## 1. Purpose

Help is the one place to read documentation on the machine: the PBSD handbook, each app's help, manual pages and Info manuals. Everything is local, searchable offline, and rendered natively without a web engine. It can answer questions over the installed documentation with a local model, always with citations, and never uses the network. It replaces KHelpCenter from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Handbook (system documentation shipped as Markdown).
- Per-app help from each bundle's `help/*.md` (PLATFORM.md section 3).
- Manual pages (mdoc and man macro sets, tbl, eqn) and Info manuals, rendered natively.
- Offline full-text search across all of it; `apropos`-style search of manual names and descriptions.
- Bookmarks, history, notes on pages, reading position.
- Context help: `F1` in any app opens the page for the focused thing.
- Tutorials and interactive tours that point at real controls in other apps.
- Command explainer: paste a command line, see what each part means from the manual pages.
- Printing and export of pages.
- Language fallback.
- Optional local question answering over the documents with citations.

Out of scope:

| Item | Owner |
|---|---|
| Writing documentation (authoring tools) | OF01 Text Editor, DV01 IDE |
| Web documentation, online forums, any network fetch | IN01 Web Browser (Help shows external links as text and offers to open them there) |
| General assistant chat, tool use, document question answering outside the help corpus | AI01 Assistant |
| The first-run welcome and tour of the desktop | AD01 Installer and First Run (it uses this app's tour engine) |
| Running commands | SY03 Terminal (Help can hand a command over, never runs it) |
| API reference generation for ForgeScript projects | DV01 IDE (generated docs are registered here as a collection) |
| Release notes of updates | SY05 Software (links into Help pages) |
| The tooltip and "what's this" text inside apps | each app; SH01 |

## 3. Users and top tasks

Personas:

- **New user**: does not know the system; wants task pages and tours.
- **Everyday user**: pressed `F1` or searched "how do I…".
- **Admin and developer**: reads manual pages all day; wants speed, keyboard, cross-references, sections.
- **Offline user**: no network at all; everything must already be here.
- **Screen-reader user**: needs proper headings, lists, tables and code blocks in the a11y tree.

| # | Task | Target steps |
|---|---|---|
| T1 | Open help for the app in front (`F1`) | 1 |
| T2 | Open a manual page by name | 2 (`Ctrl+L`, `zfs 8` Enter) |
| T3 | Search all documentation for a phrase | 2 |
| T4 | Jump to a section inside a long page | 2 (outline click) or `Ctrl+G` type |
| T5 | Follow a cross-reference and come back | 1 + 1 (`Alt+Left`) |
| T6 | Bookmark a page | 1 (`Ctrl+D`) |
| T7 | Find a flag inside a manual page | 2 (`Ctrl+F`, `-o`) |
| T8 | Explain a pasted command | 2 |
| T9 | Start a tour of an app | 2 |
| T10 | Ask a question and get an answer with sources | 2 |
| T11 | Print a page | 2 |
| T12 | Copy an example command to the terminal | 1 (button on the code block) |

## 4. Features

### 4.1 Library and navigation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT05-F001 | Collections | Sidebar tree: Handbook, Apps (one node per installed app with help), Manual pages (by section), Info manuals, Tutorials, Developer documentation, Release notes. | M | 1 |
| UT05-F002 | Collection discovery | Collections come from a registry: bundles with `help/` are added at install by the platform; manual and Info directories come from system configuration. No scanning of user files. | M | 1 |
| UT05-F003 | Home page | Search field, "Start here" pages, recently read, bookmarks, tours, and a status line (what is indexed, in which languages). | M | 1 |
| UT05-F004 | Page view | One scrolling page with headings, paragraphs, lists, tables, code blocks, notes and warnings, images, definition lists, keyboard-key styling. | M | 1 |
| UT05-F005 | Outline | Right-hand outline of the page's headings; the current section is marked; click or `Ctrl+G` jumps. | M | 1 |
| UT05-F006 | Tabs | Pages open in tabs; middle click or `Ctrl+Enter` opens a link in a new tab; tabs restore on start. | M | 1 |
| UT05-F007 | History | Back and forward per tab with scroll position; a global history list by day. | M | 1 |
| UT05-F008 | Bookmarks | `Ctrl+D`; folders; a bookmark can point at a heading. Stored locally. Import and export as JSON. | M | 1 |
| UT05-F009 | Breadcrumb | Collection › book › chapter › page; each part is a link. | M | 1 |
| UT05-F010 | Previous / next / up | Book-order navigation at the page foot and on `Alt+PageUp/PageDown`. | S | 1 |
| UT05-F011 | Address field | `Ctrl+L`: type a page title, `name(section)` or `name section`, an Info node `(manual)Node`, or a `help:` address; completion from the index. | M | 1 |
| UT05-F012 | Links | Internal links go inside Help. Links to other apps' actions use `svc.intent`. External (network) links are shown with the `--net` mark and are never fetched; "Open in Web Browser" or "Copy address". | M | 1 |
| UT05-F013 | Find in page | `Ctrl+F` with match count, case and whole-word options, regex option. | M | 1 |
| UT05-F014 | Reading position | Each page remembers where the user stopped; books show a "continue reading" mark. | S | 2 |
| UT05-F015 | Notes and highlights | Private highlights and margin notes on any page, stored locally, anchored by heading and text quote so they survive updates where possible. | C | 3 |
| UT05-F016 | Copy with source | Copy puts plain text and Markdown on the clipboard; "Copy link to this section" gives a `help:` address. | S | 2 |
| UT05-F017 | Code blocks | Monospace, optional wrapping, a Copy button, and "Send to Terminal" (pastes into SY03 without running; the user presses Enter there). | M | 1 |
| UT05-F018 | Images and diagrams | Bundled images only (PNG, the SVG subset). Click to zoom. No remote images. | M | 1 |
| UT05-F019 | Version shown | Each page shows which package and version it came from and its language. | S | 2 |
| UT05-F020 | Page not found | Lists near matches, other sections with the same name, and other languages. | M | 1 |

### 4.2 Manual pages and Info

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT05-F030 | Native rendering | Manual source is parsed by mandoc in a helper into a typed document tree and drawn by the same page view as Markdown. No HTML, no web engine, no terminal emulation. | M | 1 |
| UT05-F031 | Macro sets | mdoc and man, with tbl tables and eqn equations. Engine facts: `hbsd/src/contrib/mandoc/mdoc_*.c`, `man_*.c`, `tbl*.c`, `eqn*.c`. Unsupported roff requests are listed at the page foot as warnings. | M | 1 |
| UT05-F032 | Semantic styling | mdoc semantics are kept: flags, arguments, command names, file paths, environment variables, errors and cross-references each get a style and an a11y role, and are searchable by kind ("find the flag -r"). | M | 2 |
| UT05-F033 | Sections | Sections 1–9 and subsections; a page that exists in several sections shows a section switch. Section introductions (`intro`) are linked. | M | 1 |
| UT05-F034 | Cross-references | `name(section)` references are links; unresolved ones are shown as plain text with a dotted underline and a tooltip. | M | 1 |
| UT05-F035 | Synopsis panel | The SYNOPSIS is pinned in a collapsible panel while scrolling. | S | 2 |
| UT05-F036 | Option index | A generated list of all flags in the page with their one-line descriptions; clicking jumps. | S | 2 |
| UT05-F037 | Apropos | Search names and one-line descriptions, with section and architecture filters, using the mandoc database (`mandoc.db`, format in `hbsd/src/contrib/mandoc/mandoc.db.5`; reader `dba_read.c`, `mansearch.c`) or the app's own index built from it. | M | 1 |
| UT05-F038 | Manual paths | Base system, packages, and each app bundle's `man/` directory, from the registry. Compressed pages (`.gz`) are read. | M | 1 |
| UT05-F039 | Width independence | Text reflows to the window; nothing is formatted to 80 columns. Tables scroll sideways when they must. | M | 1 |
| UT05-F040 | Page source | "View source" shows the roff or Markdown source read-only. | C | 2 |
| UT05-F041 | Info manuals | Info files are read by an own reader in the same helper: nodes, menus, cross-references, indices. Nodes map to pages, the menu tree to the sidebar. (There is no Texinfo in `hbsd/src/contrib`; packages may install Info files.) | S | 2 |
| UT05-F042 | Info index search | The manual's own index nodes are searchable from the address field. | S | 2 |
| UT05-F043 | `man` hand-off from Terminal | A `help:man/<name>.<section>` address and an intent let SY03 open a page here on request (for example from a context menu on a word). | S | 2 |
| UT05-F044 | Lint view | For documentation authors: mandoc's lint messages (`-Tlint`, Verified option in `hbsd/src/contrib/mandoc/main.c`) for the open page. | C | 3 |
| UT05-F045 | Plain-text documents | `README`, licence and change-log files registered by packages open as text or Markdown. | S | 2 |

### 4.3 Search

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT05-F050 | Full-text index | An inverted index over every registered document: title, headings, body, code, manual name and description, with positions. Built locally by the index helper. | M | 1 |
| UT05-F051 | Ranking | BM25 with field weights (title > heading > manual name > body), a boost for the app the user came from, and for the user's language. | M | 1 |
| UT05-F052 | Query syntax | Words, "phrases", `-exclude`, `title:`, `in:man`, `in:app:<id>`, `section:8`, `lang:de`, prefix `word*`. Plain typing needs none of it. | S | 2 |
| UT05-F053 | As-you-type results | Results in under 50 ms with a snippet and the hit highlighted; grouped by collection. | M | 1 |
| UT05-F054 | Typo tolerance | Edit-distance 1–2 suggestions ("did you mean") from the index vocabulary. | S | 2 |
| UT05-F055 | Stemming and languages | Per-language tokenising and stemming from FORGE's text layer where available; others fall back to plain word split. CJK uses bigrams. | S | 2 |
| UT05-F056 | Filters | Collection, app, manual section, language, document type (task, concept, reference, tutorial). | M | 1 |
| UT05-F057 | Search inside a book | Scope the search to the current collection or book. | S | 2 |
| UT05-F058 | Semantic search | Optional: passage embeddings from `svc.model` are combined with BM25 so that questions in the user's own words find pages that use other words. Off unless `ai.local` is granted. | S | 3 |
| UT05-F059 | Index updates | The platform tells Help when a bundle or package with documents is installed, updated or removed; only those documents are re-indexed. | M | 1 |
| UT05-F060 | System search contribution | Titles, headings and manual names are offered to `svc.search` so SH01's search finds help pages. | S | 2 |
| UT05-F061 | Action search | Actions and settings that pages describe carry links ("Open this setting"); search results for a setting show both the page and the direct link. | S | 2 |
| UT05-F062 | Search history | Recent queries, local, clearable; off in private mode. | C | 2 |
| UT05-F063 | Index status | Preferences show index size, document count, last update, and "Rebuild". | M | 1 |

### 4.4 Context help and tours

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT05-F070 | F1 contract | An app (or the host on its behalf) emits a `help` request with `app_id`, an optional `topic` key and the focused control's id. Help resolves it to a page and anchor through the bundle's `help/index.tbl` (topic → file#anchor), falling back to the app's start page. | M | 1 |
| UT05-F071 | Small help window | Context help opens in a compact window (480×640) beside the asking app, not over it; "Open in full Help" expands. | S | 2 |
| UT05-F072 | Help for a control | "What is this?" mode (`Shift+F1`): the next click on a control opens the paragraph that documents it, when the app's index has it. | S | 2 |
| UT05-F073 | Help for errors | Apps can pass an error id; Help opens the troubleshooting entry for it. | S | 2 |
| UT05-F074 | Tutorials | Step-by-step pages with checkable steps and progress kept per tutorial. | S | 2 |
| UT05-F075 | Interactive tours | A tour script (declarative `tour.json` in a bundle's `help/`) lists steps: text, the target control's a11y id, and the condition that completes the step. The shell draws a highlight around the target; Help never drives another app's input. | S | 3 |
| UT05-F076 | Tour safety | A tour can only point and wait. It cannot click, type or read content. Targets are resolved through `svc.a11y` with a tour-only right that exposes ids and bounds, not text. | M | 3 |
| UT05-F077 | Tour controls | Next, Back, Skip, Stop; `Esc` always stops; progress is saved. | S | 3 |
| UT05-F078 | "Show me" links | A page link that opens the right app and view (through `svc.intent`) and highlights one control. | S | 3 |
| UT05-F079 | Keyboard shortcut pages | For each app, a generated page of its action table and current bindings (from the shared action table, PLATFORM.md section 5). | S | 2 |
| UT05-F080 | Capability explainer pages | For each capability in PLATFORM.md section 6 a page says what it allows; SH04's prompts and AD05 link to them. | M | 1 |

### 4.5 Command explainer

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT05-F090 | Explain a command line | Paste a command; it is split by shell rules (quotes, pipes, redirections, substitutions) and each word is labelled: command, subcommand, flag, flag argument, operand, redirection. | S | 2 |
| UT05-F091 | Flag lookup | Each flag is matched against the option list of the command's manual page (mdoc semantic markup makes this exact for mdoc pages); the description is shown beside it. | S | 2 |
| UT05-F092 | Unknown parts | Flags or commands with no manual entry are marked "not documented here"; nothing is guessed. | M | 2 |
| UT05-F093 | Risk notes | A rule list flags patterns that destroy data or change system state (recursive delete, raw device writes, pool destroy, piping a download to a shell) with a `--warn` note. Rules are data in the bundle. | S | 2 |
| UT05-F094 | Model-assisted summary | Optional: `svc.model` writes a two-line summary of the whole command using only the matched manual text, with citations; labelled "generated". | C | 3 |
| UT05-F095 | From Terminal | SY03 can send the current line through an intent; Help never reads the terminal. | S | 2 |
| UT05-F096 | Never runs anything | The explainer has no execution path. "Send to Terminal" pastes only. | M | 2 |

### 4.6 Question answering (optional, local)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT05-F100 | Ask | A question field on the home page and `Ctrl+J`. Retrieval finds passages (BM25 plus embeddings); a local model from `svc.model` writes a short answer from those passages only. | S | 3 |
| UT05-F101 | Citations | Every sentence of the answer carries numbered citations that link to the passage, which is highlighted on the page. An answer with no supporting passage is not shown; the app says "Not found in the documentation" and lists the nearest pages. | M | 3 |
| UT05-F102 | Sources panel | The passages used are listed under the answer with collection, page, version. | M | 3 |
| UT05-F103 | Labelled and off by default | Needs `ai.local` and an installed model (AI02). The feature is marked "generated by a local model; check the sources". Search works fully without it. | M | 3 |
| UT05-F104 | Documents are data | Retrieved text is passed to the model as quoted data. Instructions inside documents are not followed; the model has no tools and no capabilities. | M | 3 |
| UT05-F105 | Follow-up questions | Up to 5 turns kept in memory for the tab; not stored unless the user saves the answer as a note. | C | 3 |
| UT05-F106 | Hand-off | "Continue in Assistant" passes the question and sources to AI01. | C | 3 |
| UT05-F107 | No network | Retrieval and generation are local. If no model is installed the field explains how to add one in AI02. | M | 3 |

### 4.7 Languages

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT05-F110 | Language selection | Pages are shown in the first available language of the user's ordered language list. | M | 1 |
| UT05-F111 | Fallback | Order: exact locale (`de_AT`), language (`de`), the user's next languages, then English. The page header says when a fallback is shown ("Shown in English: no German version"). | M | 1 |
| UT05-F112 | Switch language per page | A dropdown lists the languages a page exists in. | S | 2 |
| UT05-F113 | Mixed search | Search covers the user's languages; results are labelled by language. | S | 2 |
| UT05-F114 | Outdated translation notice | When a translation's source version is older than the original, a notice links to the original. Needs a `source_rev` field in the page front matter. | C | 3 |
| UT05-F115 | Localised manual pages | Manual directories per language are honoured where packages ship them. | S | 2 |
| UT05-F116 | Right-to-left pages | Page direction follows the page language; code blocks stay left-to-right. | M | 2 |

### 4.8 Printing, export, settings, safety

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT05-F120 | Print | A page, a selection, or a whole book through `svc.portal.print`, with page numbers, running titles and a table of contents for books. | S | 2 |
| UT05-F121 | Export | Page as Markdown, plain text or PDF (print-to-PDF). Manual pages can export as Markdown through mandoc's Markdown output (`-Tmarkdown`, Verified option in `hbsd/src/contrib/mandoc/main.c`). | S | 2 |
| UT05-F122 | Reading settings | Text size, line width (60–100 characters), serif or sans, justified or ragged, theme follows the system. | M | 1 |
| UT05-F123 | Private mode | No history, no search history, no reading positions. | S | 2 |
| UT05-F124 | Signed documentation only | Pages come from signed bundles and packages on verified datasets. A document opened from a user file (a loose `.md` or man file) is shown with an "unverified document" banner and has links to app actions disabled. | M | 1 |
| UT05-F125 | Restricted Markdown | The renderer accepts a fixed Markdown subset: no raw HTML, no scripts, no remote resources, no embedded objects. Unknown constructs render as text. | M | 1 |
| UT05-F126 | Action links are confirmed | A page link that would open a setting or run an app action shows its target before acting ("Opens Settings › Network"); it can only invoke intents the target app declares. | M | 2 |
| UT05-F127 | Helper crash handling | A page that crashes the parser shows "This page could not be shown" with "View source"; other tabs are unaffected. | M | 1 |
| UT05-F128 | Feedback note | "Note a problem with this page" saves a local note with page, version and text selection; export for sending by the user's own means. No automatic sending. | C | 3 |
| UT05-F129 | Open loose files | Open a Markdown or manual source file from Files to read it (authors checking their work). | S | 2 |
| UT05-F130 | Session restore | Tabs, positions and the sidebar state restore after a crash. | M | 1 |

## 5. GUI

### 5.1 Window map

```
Help
├─ Main window (tabs)
│  ├─ Sidebar: Contents (collections tree) | Bookmarks | History | Search results
│  ├─ Page view
│  ├─ Outline / inspector (headings, page info, sources)
│  └─ Status bar (language, source package, index state)
├─ Home page
├─ Search results page
├─ Compact context-help window
├─ Command explainer view
├─ Ask view (answer with citations)
├─ Tour overlay card (drawn by the shell on behalf of Help)
├─ Bookmark editor popover
├─ Find-in-page bar
├─ Print options (through SH04)
└─ Preferences (Reading, Search and index, Languages, Local AI, Privacy)
```

### 5.2 Main window

Shown at 1200×760, a manual page open.

```
+------------------------------------------------------------------------------------------+
| [=] Help — zfs(8)       (1)[ zfs 8_______________________ Ctrl+L ]            [_][o][x] |
+------------------------------------------------------------------------------------------+
| [zfs(8) x] [Files: Tags x] [+]                                                      (2) |
+-----------+----------------------------------------------------------+-------------------+
| [Contents|| [<] [>]  Manual pages > 8 System manager > zfs       (4) | ON THIS PAGE  (7) |
| Bookmarks|+----------------------------------------------------------+ NAME              |
| History] (3)| SYNOPSIS                                      [pin] (5) | SYNOPSIS          |
| > Handbook|  zfs create [-Pnpuv] [-o property=value]… filesystem      | DESCRIPTION       |
| > Apps    |  zfs destroy [-Rfnprv] filesystem|volume                  | > SUBCOMMANDS     |
| v Manual  +----------------------------------------------------------+   create          |
|   1 User  | zfs(8)              System Manager's Manual        zfs(8) |   destroy         |
|   5 Formats|                                                          |   snapshot        |
|   8 System| NAME                                                      | EXAMPLES          |
|   > zfs   |   zfs — configure ZFS datasets                            | SEE ALSO          |
|     zpool | DESCRIPTION                                               +-------------------+
| > Info    |   The zfs command configures ZFS datasets within a ZFS    | PAGE          (8) |
| > Tutorials|  storage pool, as described in zpool(8).            (6)  | Package base 15   |
|           |   -r   Recursively apply to all descendants.              | Language English  |
|           |   $ zfs snapshot -r pool/home@monday        [Copy] [→Term]| Section 8         |
|           |                                                           | [Bookmark] [Print]|
+-----------+----------------------------------------------------------+-------------------+
| English (no German version)        base 15.0 · mandoc             Index: 18 402 documents|
+------------------------------------------------------------------------------------------+
```

1. Address and search field (`lineEdit` with a completion `popupMenu`). Typing words searches; `name section` jumps.
2. `tabBar`.
3. Sidebar with a three-way `tabBar`: Contents (`treeView`), Bookmarks (`treeView`), History (`listView` grouped by day).
4. Back, forward (`toolButton`s) and breadcrumb (`link` row).
5. Pinned synopsis panel (`panel`, collapsible), manual pages only.
6. Page view: the rich-content view (FUSE-WP-42; today's `markdownView` is too limited, see 5.7). Cross-references such as `zpool(8)` are links. Code blocks carry Copy and Send-to-Terminal `toolButton`s.
7. Outline (`treeView` of headings).
8. Page facts (`propertyGrid`, read-only) and two actions.

States: empty (Home page), loading (page skeleton for under 100 ms pages is skipped; a spinner in the tab for long books), error (parser failure card with View source), offline (always; external links marked), read-only (always: Help never edits documents), unverified document (banner, action links off), narrow (sidebar and outline become overlays; breadcrumb collapses to the last two parts).

### 5.3 Other views and dialogs

**Home page** (1200×760)

```
+------------------------------------------------------------------------------------------+
|                     [ Search or ask about this computer…            ]                    |
|                     [x] Handbook [x] Apps [x] Manual pages [x] Info      (1)             |
|                                                                                          |
|  Start here                         Continue reading                                     |
|   Getting around the desktop         Handbook › Storage › Snapshots      62%             |
|   Files, permissions and portals     zfs(8)                              SUBCOMMANDS     |
|   Staying private and secure                                                             |
|   Keyboard shortcuts                Tours                                                |
|                                      Tour of the desktop  [Start]                        |
|  Bookmarks                           Tour of Files        [Start]                        |
|   pf.conf(5) · Backup › Restore                                                          |
|                                                                                          |
|  Local question answering: off. [Set up…]                                (2)             |
+------------------------------------------------------------------------------------------+
```

1. Scope `checkBox`es. 2. One calm line; hidden once set up or dismissed.

**Search results page**

```
+------------------------------------------------------------------------------------------+
| "snapshot rollback"     214 results in 12 ms      Filter [All v] Section [Any v] [de|en] |
+------------------------------------------------------------------------------------------+
| HANDBOOK                                                                                 |
|  Rolling back to a snapshot                              Handbook › Storage              |
|   …to return a dataset to a **snapshot**, use **rollback**; newer snapshots are…         |
| APPS                                                                                     |
|  Backup: Restore from a snapshot                         Backup                          |
|   …choose the **snapshot** and press Restore. A **rollback** replaces…                   |
| MANUAL PAGES                                                                             |
|  zfs-rollback(8) — roll ZFS dataset back to snapshot     section 8                       |
|  zfs-snapshot(8) — create snapshots of ZFS datasets      section 8                       |
|  Did you mean: snapshots rollback                                                        |
+------------------------------------------------------------------------------------------+
```

A `modelListView` with group headers; `Enter` opens, `Ctrl+Enter` opens in a new tab; arrow keys move without leaving the search field.

**Compact context-help window** (480×640)

```
+--------------------------------------------------+
| [=] Help: Files — Tags              [⤢] [x]      |
+--------------------------------------------------+
| [<] [>]  Files > Tags                            |
| Tags                                             |
| A tag is a label you attach to a file…           |
|  1. Select the files.                            |
|  2. Press Ctrl+T.                                |
|  [Show me]                                       |
| Related: Searching by tag · Tag colours          |
+--------------------------------------------------+
| [Search this app's help…]                        |
+--------------------------------------------------+
```

`[⤢]` opens the page in the main window. The window is placed beside the asking app by SH05.

**Command explainer view**

```
+------------------------------------------------------------------------------------------+
| Command [ zfs destroy -r -n zroot/data@old | tee out.txt                    ] [Explain]  |
+------------------------------------------------------------------------------------------+
| zfs          command      configure ZFS datasets                         zfs(8)          |
| destroy      subcommand   Destroys the given dataset.                    zfs-destroy(8)  |
| -r           flag         Recursively destroy all children.              zfs-destroy(8)  |
| -n           flag         Do a dry-run ("no-op") deletion.               zfs-destroy(8)  |
| zroot/data@old operand    a snapshot name                                                |
| |            pipe         output of the left command goes to the right                   |
| tee          command      duplicate standard input                       tee(1)          |
| out.txt      operand      file written by tee                                            |
+------------------------------------------------------------------------------------------+
| ! "zfs destroy" removes data permanently. Here -n makes it a dry run: nothing is removed.|
|                                                    [Copy table]  [Send to Terminal]      |
+------------------------------------------------------------------------------------------+
```

`lineEdit` (monospace), `tableView`; the last column is a link. Unknown items show "not documented here".

**Ask view**

```
+------------------------------------------------------------------------------------------+
| Question [ How do I get back a file I deleted yesterday?                     ] [Ask]     |
+------------------------------------------------------------------------------------------+
| Generated by a local model from the documentation. Check the sources.             (1)   |
|                                                                                          |
| If the file was in the trash, restore it from Files › Trash [1]. If it has been          |
| emptied, open the folder in Files and use "Browse snapshots" to find yesterday's         |
| version, then copy it back [2]. Snapshots exist only if a schedule is on [3].            |
|                                                                                          |
| Sources                                                                                  |
|  [1] Files › Trash and undo                         Files 0.1                            |
|  [2] Files › Snapshots browser                      Files 0.1                            |
|  [3] Backup › Snapshot schedules                    Backup 0.1                           |
|                                              [Copy] [Continue in Assistant] [New question]|
+------------------------------------------------------------------------------------------+
```

1. Fixed label, always shown. Citation numbers are links that open the page with the passage highlighted. While the model works: a `progressBar` and the retrieved sources already listed.

**Tour overlay card** (drawn by the shell near the target control)

```
        ╔════════════════════════════════════╗
        ║ Step 2 of 6                        ║
        ║ This is the sidebar. Drag a folder ║
        ║ here to keep it handy.             ║
        ║            [Back] [Skip] [Next]    ║
        ╚════════════════════════════════════╝
   ┌ ─ ─ ─ ─ ─ ─ ─ ┐
   │ (target, ringed)│
   └ ─ ─ ─ ─ ─ ─ ─ ┘
```

**Find-in-page bar**: `lineEdit`, "3 of 17", previous/next `toolButton`s, `[Aa]` `[word]` `[.*]` toggles, `Esc` closes.

### 5.4 Menus

```
File    New Tab Ctrl+T · New Window Ctrl+N · Open File… Ctrl+O · Print… Ctrl+P
        Export Page ▸ (Markdown, Text, PDF) · Close Tab Ctrl+W · Quit Ctrl+Q
Edit    Copy Ctrl+C · Copy Link to Section Ctrl+Shift+C · Select All Ctrl+A
        Find in Page Ctrl+F · Find Next Ctrl+G(F3) · Add Note Ctrl+M · Preferences Ctrl+,
View    Sidebar F9 · Outline F4 · Contents Ctrl+1 · Bookmarks Ctrl+2 · History Ctrl+3
        Pin Synopsis · Text Size Ctrl++ / Ctrl+- / Ctrl+0 · Line Width ▸ · View Source Ctrl+U
        Language ▸ · Full Screen F11
Go      Back Alt+Left · Forward Alt+Right · Home Alt+Home · Up Alt+Up
        Previous Page Alt+PageUp · Next Page Alt+PageDown · Address Ctrl+L · Go to Section… Ctrl+Shift+G
Bookmarks  Bookmark This Page Ctrl+D · Bookmark Section · Show All Bookmarks Ctrl+Shift+O
Tools   Search Everything Ctrl+Shift+F · Explain a Command… Ctrl+E · Ask… Ctrl+J
        Tours ▸ · Keyboard Shortcuts of ▸ · What Is This? Shift+F1 · Rebuild Index
Help    Using Help F1 · About the Documentation on This Computer · About
```

`Ctrl+G` is "find next" per the reserved table; "Go to Section" is `Ctrl+Shift+G`.

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Ctrl+L` | Address and search field |
| `/` | Find in page (manual-reader habit); `n` / `N` next, previous |
| `Space` / `Shift+Space` | Page down / up |
| `j` / `k` | Line down / up (optional reader keys, setting) |
| `g` `g` / `G` | Top / bottom (optional reader keys) |
| `Alt+Left/Right` | Back / forward |
| `Ctrl+D` | Bookmark |
| `Ctrl+E` | Explain a command |
| `Ctrl+J` | Ask |
| `Tab` | Next link; `Enter` follows; `Ctrl+Enter` new tab |

Context menus: link (Open, Open in New Tab, Copy Address, Bookmark Target); text selection (Copy, Search Help For "…", Explain as Command, Add Note, Highlight); code block (Copy, Send to Terminal, Explain); page (Back, Forward, Bookmark, Print, View Source, Copy Link).

Drag and drop: a link to the tab bar (new tab) or to Bookmarks; a page tab out to a new window; a Markdown or manual file from SY02 onto the window (opens as an unverified document); selected text out as plain text and Markdown.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Home page. Empty bookmarks/history: one line each. |
| Loading | Index being built for the first time: search works on titles at once and says "Indexing… 42%". |
| Error | Missing page: near matches. Parser failure: card with source view. Index damaged: rebuilt automatically, titles-only search meanwhile. |
| Offline | Always. External links are text with "Open in Web Browser". |
| No model | Ask view explains how to add a model; search unaffected. |
| Locked | Not applicable. |
| Read-only | Always for documents; notes and bookmarks are the only user data. |
| Narrow (<640) | Sidebar and outline as overlays; compact window layout is used. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Tabs | `tabBar` |
| Sidebar | `tabBar`, `treeView`, `listView` |
| Address field | `lineEdit`, `popupMenu` |
| Page | rich-content view (FUSE-WP-42) inside `scrollArea` |
| Outline | `treeView` |
| Search results | `modelListView` |
| Explainer | `lineEdit`, `tableView` |
| Page facts | `propertyGrid` |
| Find bar | `lineEdit`, `toolButton` |

Missing: `markdownView` today renders headings 1–3, paragraphs, lists, code blocks and quotes, and strips inline bold and code; it has no links, images, tables, selection or anchors (stated in FUSE_BUILD_PLAN.md section 2 baseline). Help needs all of those plus definition lists, semantic inline spans and find-in-page highlights: this is FUSE-WP-42 (and FUSE-WP-18 for selection). Until then Help cannot ship (`UT05-GAP-1`). Tour highlight drawn by the shell: `UT05-GAP-2` (owned by SH01/SH05). No web content engine is used; G-14 is deliberately not a dependency.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | App host | Navigation, registry, search queries, bookmarks, history, explainer logic, retrieval and prompt assembly |
| UI | FUSE UI | App host | All windows |
| `doc-helper` | C++23 wrapping mandoc (C) plus own Info reader | One per page render, pooled; capability mode | Input: one read-only document handle. Output: a typed document tree (blocks, inline spans with semantic kinds, anchors, link targets) as a bus record. No file, network or exec |
| Markdown parser | C++23 in FUSE (FUSE-WP-42) | App host for signed bundle pages; `doc-helper` for unverified loose files | Markdown → the same document tree |
| `index-helper` | C++23 | Background, capability mode | Builds and updates the full-text index from document trees; holds a write handle to the index directory only |
| Tour runner | ForgeScript | App host | Reads `tour.json`, asks the shell to highlight a target, waits for the step condition |
| Model client | ForgeScript | App host | `svc.model` calls for embeddings and answers |

```
 app (F1) ──help request──► Help app host ──doc handle──► doc-helper (mandoc / Info) ──tree──┐
 documentation registry ───►   │                                                              │
 (bundles help/, man dirs)      ├──► rich-content view ◄────────────────────────────────────────┘
                                ├──► index-helper ──► ~/.cache/org.pbsd.help/index/
                                ├──► svc.model (embeddings, answer)       [ai.local, optional]
                                └──► shell (tour highlight), svc.intent (action links, Terminal)
```

Why a typed tree and not HTML: mandoc can emit HTML (`mdoc_html.c`, `man_html.c`), but rendering HTML needs a web engine (G-14) and brings its attack surface. The helper instead walks mandoc's parsed syntax tree (the structures behind `-Ttree`, `hbsd/src/contrib/mandoc/tree.c`) and emits Help's own tree format. This is a new, small output back end written in C++23 beside the C library (Proposed).

mandoc is C. As with UT01, containment is the sandbox: the helper gets one descriptor and returns one record.

### 6.2 Data model

Stores: `~/.local/share/org.pbsd.help/user.sqlite` (bookmarks, history, notes, positions, tutorial progress), `~/.cache/org.pbsd.help/index/` (full-text index segments, embeddings, registry cache), `~/.cache/org.pbsd.help/trees/` (rendered trees keyed by document hash).

| Entity | Fields |
|---|---|
| `Collection` | `id: string`, `kind: enum{handbook, app, man, info, tutorial, dev, notes}`, `title`, `owner_bundle: string?`, `root: handle`, `languages: list<string>`, `version: string`, `verified: bool` |
| `Document` | `doc_id: u64`, `collection_id`, `path: string` (relative), `format: enum{markdown, mdoc, man, info, text}`, `lang: string`, `title`, `section: string?`, `description: string?`, `hash: bytes32`, `source_rev: string?`, `doc_type: enum{task, concept, reference, tutorial}` |
| `DocTree` | `blocks: list<Block>`, `anchors: map<string, block_index>`, `outline: list<{level: u8, text, anchor}>`, `warnings: list<string>` |
| `Block` | `kind: enum{heading, para, list, list_item, def_term, def_body, code, table, row, cell, note, warning, image, rule, synopsis}`, `level: u8`, `spans: list<Span>`, `children: list<Block>`, `anchor: string?` |
| `Span` | `text: string`, `style: bitset{bold, italic, mono}`, `sem: enum{none, flag, arg, cmd, path, env, errno, xref, key, link, var}`, `target: string?` |
| `Address` | `help:<collection>/<path>#<anchor>`; manual pages `help:man/<name>.<section>`; Info `help:info/<manual>/<node>` |
| `TopicIndex` (per bundle, `help/index.tbl`) | `topic: string` → `path#anchor`; `control_id: string` → `path#anchor`; `error_id` → `path#anchor` |
| `Bookmark` | `id`, `address`, `title`, `folder_id`, `created: i64`, `order` |
| `HistoryEntry` | `address`, `title`, `visited: i64`, `scroll: f32` |
| `Note` | `id`, `address`, `quote: string`, `prefix: string`, `suffix: string`, `text`, `colour: u8`, `created` |
| `Posting` (index) | `term_id: u32`, `doc_id: u64`, `field: u8`, `positions: varint list` |
| `Passage` (semantic) | `doc_id`, `anchor`, `text_range`, `embedding: f32[n]` (n from the model), `model_id: string` |
| `Tour` (`tour.json`) | `id`, `title`, `app_id`, `steps: list<{text_key, target_a11y_id, advance: enum{next_button, target_activated, view_shown}, timeout_s}>` |
| `Answer` (memory only) | `question`, `sentences: list<{text, citations: list<passage_ref>}>`, `model_id`, `elapsed_ms` |

Page front matter (Markdown): `title`, `type`, `lang`, `source_rev`, `topics`. `user.sqlite` has `meta.schema_version`; caches are rebuilt on version change. Embeddings are dropped when the model id changes.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Markdown (restricted subset, front matter) | yes | export | FUSE rich-content parser (FUSE-WP-42) | App host for signed bundle content; doc-helper for loose files |
| mdoc, man, tbl, eqn (roff subset) | yes | no | mandoc from `hbsd/src/contrib/mandoc` (ISC-style licence, in the base tree, builds offline); own tree emitter | doc-helper |
| gzip-compressed pages | yes | no | zlib from base | doc-helper |
| `mandoc.db` | yes | no | mandoc's reader (`dba_read.c`, `mansearch.c`) or own reader of the documented format (`mandoc.db.5`) | index-helper |
| Info files (plain and split, with tag tables) | yes | no | own C++23 reader | doc-helper |
| `index.tbl`, `tour.json` | yes | no | own, strict | App host (signed bundle data) |
| PNG, SVG subset images | yes | no | platform decoders (FUSE-WP-27) | platform image sandbox |
| Full-text index segments | yes | yes | own | index-helper writes, app host reads (memory-mapped, bounds-checked reader) |
| `help:` addresses | yes | yes | own | App host |
| PDF | no | yes | print pipeline (G-16) | print service |
| `svc.model` requests | — | — | bus schema (FORGE-WP-29) | model service |

### 6.4 Integration

- Intents handled: `help` (the F1 contract: `app_id`, `topic`, `control_id`, `error_id`), `open`/`view` for `text/markdown`, `text/troff`, `application/x-info`, the `help:` URL scheme, `search`.
- Intents emitted: `open` (action links into other apps, external links to IN01), `print`, a paste request to SY03 ("Send to Terminal"), hand-off to AI01.
- Services: `svc.settings`, `svc.intent`, `svc.search`, `svc.portal.file`, `svc.portal.print`, `svc.clipboard`, `svc.a11y` (tour target lookup), `svc.model` (optional), `svc.notify` (index built, rarely).
- Needs from the platform: a **documentation registry** that lists installed collections and passes read-only directory handles to Help (new gap `UT05-GAP-3`), and the **help request** being wired to `F1` in the app host (part of G-07; DESIGN_SYSTEM.md section 6 reserves `F1`).
- Every app bundle may ship `help/*.md` (PLATFORM.md section 3). This spec adds to that directory, as a proposal: `help/index.tbl`, `help/tour.json`, `help/<lang>/…` for translations, `man/` for manual pages.
- Links: SH01 (search results, tour highlight), SH04 and AD05 (capability explainer pages), SY03 (command hand-off both ways), AD01 (first-run tour uses the tour runner), AI01 (hand-off), AI02 (model install), SY05 (release notes).
- Repo facts used: mandoc output modes `ascii, locale, utf8, tree, man, html, markdown, lint, ps, pdf` (`hbsd/src/contrib/mandoc/main.c:72-81`). mandoc's own PostScript and PDF outputs are not used; printing goes through the system pipeline.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Bookmarks, history, notes, index and tree caches |
| `fs.portal` | Loose documents the user opens; export destinations |
| `fs.path:/usr/share/man:r`, `fs.path:/usr/local/share/man:r`, `fs.path:/usr/share/doc:r`, `fs.path:/usr/local/share/doc:r`, `fs.path:/usr/local/share/info:r` | Read system documentation. Shown at install. Replaced by registry handles when `UT05-GAP-3` exists |
| `net.none` | No network |
| `proc.helper:doc-helper` | Contain roff and Info parsing |
| `proc.helper:index-helper` | Contain index building |
| `svc.settings`, `svc.intent`, `svc.portal.file` | Core operation, F1 contract |
| `svc.search` | Offer titles to system search |
| `svc.clipboard` | Copy text and code |
| `svc.portal.print` (`dev.print`) | Print pages |
| `svc.a11y` | Resolve tour targets (ids and bounds only) |
| `svc.model`, `ai.local` | Optional embeddings and answers; off until granted |
| `svc.notify` | Index status |
| `bg.run` | Index updates after installs without a window (short-lived) |

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The user's trust in Help | A malicious or compromised package shipping documentation | Pages that tell the user to weaken security, or links that trigger actions | Only signed packages register documents; action links show their target and need a click (F126); links can invoke only declared intents; the source package is shown on every page |
| Help process | Malformed roff, Info or Markdown | Parser memory corruption | mandoc and the Info reader run in `doc-helper` (one handle in, one record out); loose Markdown is parsed there too; tree records are validated before rendering |
| Other apps | A tour script | Driving another app's UI, reading its content | Tours can only highlight and wait; the tour right on `svc.a11y` exposes ids and bounds, never text or actions (F076) |
| The system | Command explainer or code blocks | Running a command by accident | No execution path anywhere; "Send to Terminal" pastes without a newline; risk notes (F093) |
| Answer integrity | Text inside a document written to steer the model | Prompt injection leading to false or harmful answers | Model has no tools; passages are quoted data; every sentence must cite a passage or is dropped; answers are labelled; sources are one click away |
| Reading habits | Other apps | Learn what help the user needed (for example, pages about a medical-device driver or about anonymity tools) | History in `fs.app`; private mode; nothing offered to `svc.search` except static titles |
| Privacy | Documents with remote images or links | Tracking on open | No network capability; remote resources are never fetched |
| Display | Text with bidi controls or look-alike characters in commands | User copies a command that is not what it looks like | Code blocks render control characters visibly; copy gives exactly the shown bytes |

Stored: bookmarks, history, notes, positions, search history (optional). Questions and answers are kept in memory only. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `reading.text_size` | u8 | 15 | Page text size |
| `reading.line_width_ch` | u8 | 80 | Maximum line width in characters |
| `reading.face` | enum | `sans` | sans, serif |
| `reading.justify` | bool | false | Justified text |
| `reading.reader_keys` | bool | true | `j`/`k`/`/`/`n` keys in pages |
| `reading.pin_synopsis` | bool | true | Pin SYNOPSIS on manual pages |
| `reading.open_links_in_new_tab` | bool | false | Default link target |
| `context.compact_window` | bool | true | F1 opens the compact window |
| `search.scopes` | list<enum> | all | Collections searched by default |
| `search.typo_tolerance` | bool | true | Suggestions for misspellings |
| `search.semantic` | bool | false | Use embeddings (needs `ai.local`) |
| `search.keep_history` | bool | true | Keep recent queries |
| `index.auto_update` | bool | true | Re-index after installs |
| `languages.order` | list<string> | system | Language preference; empty follows the system |
| `languages.show_fallback_notice` | bool | true | Say when a fallback language is shown |
| `ai.answers` | bool | false | Enable the Ask view |
| `ai.model` | string | auto | Model id from `svc.model` |
| `ai.max_passages` | u8 | 6 | Passages given to the model |
| `privacy.private_mode` | bool | false | No history, positions or query history |
| `privacy.history_days` | u16 | 90 | History retention |
| `tours.enabled` | bool | true | Allow tours to highlight controls |

## 9. Accessibility and languages

- The document tree maps to the a11y tree one to one: headings with levels, lists, tables with header cells, code blocks, notes, links. Screen-reader heading and landmark navigation works on every page, including manual pages.
- mdoc semantic spans are announced by kind when the user asks for detail ("flag: dash r").
- Reader keys are single letters; they are active only when the page view has focus and can be turned off (they clash with some assistive tools).
- Tours have a full text equivalent: every step is also a line in a tutorial page.
- Language fallback is described in 4.7. Complex scripts need the FULL text profile (G-03).
- Text size, line width and spacing are user settings; pages reflow at 300% without sideways scrolling, except wide tables and code.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 200 ms |
| F1 to the page shown | 300 ms cold, 100 ms warm |
| Open a manual page | 50 ms for a typical page; `zfs`-size pages 150 ms; cached trees 10 ms |
| Search as you type | Results in 50 ms over 20 000 documents |
| First full index build | Under 2 minutes for the base system plus 80 apps, in the background at low priority |
| Index size | Under 25% of the source text size |
| Memory | Host 120 MiB with 10 tabs; doc-helper 64 MiB cap; index-helper 256 MiB cap |
| Largest page | A 5 MiB manual or a 2 000-node Info manual; long pages are laid out lazily |
| Answer | First retrieved sources in 200 ms; generated text time depends on the model and is shown as progress |
| Idle | No wake-ups |

## 11. Testing and proof

- **Unit**: address parsing, fallback order, registry, topic index resolution, command-line splitting, flag matching, citation checks, BM25 scoring.
- **Corpus render test**: every manual page in the base system is parsed; none may fail; warnings are counted and tracked. Every bundle's `help/` is rendered and link-checked at build time (dead internal links fail the build).
- **Golden**: page view for a fixed set of Markdown, mdoc, man, tbl and Info samples; themes, scales, right-to-left.
- **Differential**: plain text extracted from Help's tree for each manual page is compared with `mandoc -Tutf8` output after whitespace normalisation; differences are reviewed.
- **Scripted UI**: T1–T12; the F1 contract from a test app.
- **Fuzz**: doc-helper on roff and Info inputs; the Markdown parser; the index reader; `tour.json` and `index.tbl` readers.
- **Search quality set**: a fixed list of queries with expected top-5 pages; tracked as a number, not a pass claim.
- **Answer checks**: a fixed question set; every sentence must cite a passage that contains its support (checked by string and embedding overlap); a set of injection documents must not change behaviour.
- **Sandbox**: helpers cannot open paths or sockets (G-06).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Tree validator (C++23) | Accepted trees are well-formed: bounded depth, valid anchors, valid UTF-8, links only of allowed kinds | T2 | PRISM | — |
| Markdown parser (FUSE) | No runtime errors; output is in the restricted subset | T2 | PRISM (FUSE_BUILD_PLAN.md lists Markdown at T2) | Owned by FUSE-WP-42 |
| mandoc (C) | None claimed | T4 | Sandbox, fuzzing, corpus test | Third-party C |
| Tree emitter and Info reader | Memory safety | T3 | Safe subset, fuzzing | Could reach T2 with effort |
| Index reader | In-bounds reads on any file content | T2 | PRISM; fuzzing | — |
| Link policy | A page can invoke only intents declared by the target and only after a click | T2 | PRISM on the policy function; tests | — |
| Citation gate | No sentence is shown without at least one citation into the retrieved set | T2 | PRISM on the gate; tests | Says nothing about truth of the answer |
| Model output | None | T4 | Question set | Models are not provable |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| UT05-W01 | Document tree format and validator | G-08 |
| UT05-W02 | doc-helper: mandoc build from the base tree, tree emitter for mdoc and man, tbl | G-06, G-07, FORGE-WP-27, FORGE-WP-28 |
| UT05-W03 | Page view on the rich-content path: links, anchors, tables, code blocks, selection, find | FUSE-WP-42, FUSE-WP-18 (`UT05-GAP-1`), G-03 |
| UT05-W04 | Documentation registry client; collections tree; language fallback | `UT05-GAP-3`, G-07 |
| UT05-W05 | Navigation: tabs, history, bookmarks, address field, breadcrumb | G-02 |
| UT05-W06 | index-helper and full-text search; apropos from `mandoc.db` | UT05-W01 |
| UT05-W07 | F1 contract, topic index, compact window | G-07, SH05 |
| UT05-W08 | Handbook skeleton and capability explainer pages (content) | — |
| UT05-W09 | Corpus render test and build-time link check | UT05-W02 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| UT05-W10 | Semantic styling, option index, pinned synopsis, eqn | UT05-W02 |
| UT05-W11 | Info reader | UT05-W01 |
| UT05-W12 | Command explainer with risk rules; Terminal hand-off | UT05-W10, SY03 |
| UT05-W13 | Query syntax, typo tolerance, stemming, filters, system search contribution | FORGE-WP-23, G-09 |
| UT05-W14 | Printing and export | G-16, UT06 |
| UT05-W15 | Tutorials with progress; shortcut pages from action tables | FUSE-WP-14 |
| UT05-W16 | Action links with confirmation; "what is this" mode | SH04 |
| UT05-W17 | Loose-file opening with the unverified banner; reading positions; private mode | UT05-W03 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| UT05-W18 | Tour runner and shell highlight | `UT05-GAP-2`, SH01, G-04 |
| UT05-W19 | Passage embeddings and semantic search | G-15, FORGE-WP-29, FORGE-WP-31 |
| UT05-W20 | Ask view with the citation gate | UT05-W19, AI02 |
| UT05-W21 | Notes and highlights; outdated-translation notice | UT05-W03 |
| UT05-W22 | Model-assisted command summary; hand-off to AI01 | UT05-W20, AI01 |
| UT05-W23 | Lint view for authors | UT05-W02 |

## 13. Open questions and risks

- Help depends on the rich-content view (FUSE-WP-42). The current `markdownView` cannot show links or tables. This is the critical path for a P1 app.
- PLATFORM.md lists G-14 (web content engine) as needed for Help. This spec avoids it on purpose. If the owner wants HTML documentation rendered, that decision must be revisited; the recommendation is to convert such documents to Markdown at package build time.
- The documentation registry does not exist (`UT05-GAP-3`). The fixed `fs.path` grants are a stopgap and should go.
- mandoc is C and stays contained. An own roff parser is not planned.
- The handbook itself must be written. It is content work with no code dependency and can start at once.
- Embedding and answer models for documentation-sized corpora must fit the memory budget of `svc.model`; which model classes exist is defined in FORGE-WP-31 (not read in detail here).
- Whether `F1` is delivered by the app host or by each app is not fixed in PLATFORM.md; this spec assumes the host emits the request with the focused control id.
- Info manuals are GFDL content installed by packages; none ship in the base tree read here.
