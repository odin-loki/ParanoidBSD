<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Feeds

By Odin Loch

| | |
|---|---|
| ID | `IN02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.feeds` |
| Category | internet |
| Priority | P2 |
| Backing repos | none. Uses FUSE widgets, the planned network service (FORGE-WP-21) and the planned restricted rich-content view (FUSE-WP-42). |
| Status | Planned. Nothing built. |

## 1. Purpose

Feeds collects articles and podcast episodes from RSS, Atom and JSON Feed sources the user chooses and keeps them for reading offline. It is for people who follow sites, blogs, release notes and podcasts without accounts or tracking. It replaces Akregator, Liferea, NewsFlash and gPodder.

## 2. Scope

In scope:

- Subscribing to feeds, refreshing them, reading, starring, tagging, filtering and searching articles.
- Full-text fetch with a script-free reader view; offline copies of articles and their images.
- Podcasts: subscriptions, episode list, downloads, queue, hand-off of playback.
- Read later list for single web pages.
- OPML import and export.
- Per-feed network route.
- Optional sync with a self-hosted server through Fever, Google-Reader-compatible and Miniflux APIs.
- Optional local summaries through `svc.model`.

Out of scope:

| Item | Owner |
|---|---|
| Playing audio | ME02 Music (Feeds hands over the file or stream and receives position updates) |
| Playing video | ME03 Video |
| Rendering full web pages with script | IN01 Web Browser ("Open in browser") |
| Receiving mail; IMAP and JMAP | CO01 Mail (it hands newsletters to Feeds by intent) |
| Large or resumable downloads beyond episodes | IN03 Transfers (optional hand-off for enclosures) |
| Route set-up (VPN, Tor) | SE03 Anonymity |
| Hosting a feed server | none; the user's own server |

## 3. Users and top tasks

Personas:

- **Reader**: follows 30–300 feeds, reads each morning, wants keyboard speed and offline copies.
- **Podcast listener**: subscribes to 10–50 shows, wants new episodes downloaded on Wi-Fi and a queue.
- **Researcher**: follows journals and release feeds, uses rules, tags and search, exports to notes.
- **Private reader**: follows some feeds only over Tor; wants no image trackers and no leak of the reading pattern.

| # | Task | Target steps |
|---|---|---|
| T1 | Subscribe to a site by pasting its address | `Ctrl+N`, paste, Enter, Enter (feed found and added) |
| T2 | Read the next unread article | `N` (1) |
| T3 | Mark a whole feed or folder read | `Shift+A` (1, with Undo toast) |
| T4 | Star an article and tag it | `S`, `T`, type tag, Enter (4) |
| T5 | Get the full text of a truncated article | `F` (1) |
| T6 | Save a web page to read later from the browser | share to Feeds (2) |
| T7 | Search all stored articles | `Ctrl+F`, type (2) |
| T8 | Import subscriptions from another reader | File > Import OPML, pick file, Import (3) |
| T9 | Play the newest episode of a show | select episode, `P` (2) |
| T10 | Make a rule that hides articles matching a word | Rules, New, fill, Save (4) |
| T11 | Put one feed on the Tor route | feed properties, Route, Tor (3) |
| T12 | Connect to a self-hosted Miniflux server | Accounts, Add, type URL and token, Connect (4) |
| T13 | Read on a train with no network | open app; everything already stored (0) |

## 4. Features

Priority: M must, S should, C could. Phase: 1 usable core, 2 full feature, 3 advanced.

### 4.1 Subscriptions

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F001 | Add feed by URL | Dialog takes a feed URL or a site URL. The fetch helper downloads once and the parser helper reports the format and title before anything is stored. | M | 1 |
| IN02-F002 | Feed discovery | For a site URL: read `<link rel="alternate">` types `application/rss+xml`, `application/atom+xml`, `application/feed+json`; then try `/feed`, `/rss`, `/atom.xml`, `/index.xml`, `/feed.json`. Lists all found; user picks. At most 6 requests. | M | 1 |
| IN02-F003 | Formats | RSS 0.9x, 1.0 (RDF), 2.0; Atom 1.0; JSON Feed 1.0 and 1.1. Namespaces: Dublin Core, content, Media RSS, podcast namespace (chapters, transcript, value ignored), iTunes tags, Atom threading. | M | 1 |
| IN02-F004 | Feed properties | Title override, site link, folder, tags, refresh interval, route, retention, full-text mode, image policy, notify on new, authentication, user notes. | M | 1 |
| IN02-F005 | Folders | Nested folders to 4 levels. A feed is in exactly one folder. Folder shows the sum of unread. | M | 1 |
| IN02-F006 | Feed tags | A feed may carry any number of tags; tags work as cross-folder views. | S | 2 |
| IN02-F007 | Rename, move, delete | `F2`, drag and drop, `Delete` with Undo toast (articles kept 30 s for undo, then purged unless starred). | M | 1 |
| IN02-F008 | Authentication | HTTP Basic and bearer token per feed; secret stored in `svc.keyring`; only sent over HTTPS to the feed's host. | S | 2 |
| IN02-F009 | Feed health | Per feed: last success, last error, HTTP status, consecutive failures, average items per week, parse warnings. A "Broken feeds" smart view lists failures. | M | 2 |
| IN02-F010 | Moved and gone | HTTP 301 and 308 update the stored URL after two consecutive sightings and tell the user. 410 pauses the feed and asks. | S | 2 |
| IN02-F011 | Pause feed | Paused feeds are not refreshed; shown dimmed with the word "Paused". | S | 2 |
| IN02-F012 | Duplicate subscription check | Adding a URL already present (after normalising scheme, case and trailing slash) offers to go to the existing feed. | S | 1 |
| IN02-F013 | Subscribe from the browser | Handles the `feed:` scheme and a "subscribe" intent from IN01 with the URL and the container's route as a suggestion. | S | 2 |
| IN02-F014 | Bundled starter list | None. No default feeds, no directory lookups. The empty state offers Add and Import only. | M | 1 |
| IN02-F015 | Local folder feed | A folder picked through the portal whose `.xml`/`.json` feed files are read as feeds (for generated or archived feeds). No network. | C | 3 |

### 4.2 Refresh and fetching

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F016 | Manual refresh | `F5` refreshes the selection; `Ctrl+F5` refreshes all. Progress in the status bar with a cancel button. | M | 1 |
| IN02-F017 | Scheduled refresh | Global interval (default 60 min) with per-feed override: 15 min to 7 days, or manual only. Jitter of ±10% so requests are not on the minute. | M | 1 |
| IN02-F018 | Conditional requests | `If-None-Match` and `If-Modified-Since` always sent; 304 costs no parse. | M | 1 |
| IN02-F019 | Respect server hints | `Cache-Control: max-age`, `Retry-After`, RSS `ttl`, `skipHours`, `skipDays`, and HTTP 429 back-off (doubling to 24 h). | M | 2 |
| IN02-F020 | Concurrency limits | At most 6 feeds at once, 1 per host; 30 s timeout; 10 MiB body cap per feed (setting); gzip and Brotli bomb guard at 50 MiB inflated. | M | 1 |
| IN02-F021 | Background refresh | With `bg.run` granted, refresh continues with no window. Without it, refresh happens on start and while open. | S | 2 |
| IN02-F022 | Metered and battery rules | No scheduled refresh on a metered link or below 20% battery unless allowed; uses `svc.power` and the network service's link state. | S | 2 |
| IN02-F023 | Request hygiene | Fixed generic user agent with no version of the OS; no cookies stored or sent unless the feed needs a login; no referrer; HTTPS first, HTTP only with a per-feed consent. | M | 1 |
| IN02-F024 | Item identity | Key is `guid`/`id`; else link; else hash of title plus date. Updated items replace content and are flagged "Updated" with a diff view (C). | M | 1 |
| IN02-F025 | Date sanity | Missing or future dates are replaced by the fetch time; the original string is kept. | M | 1 |
| IN02-F026 | Error handling | Parse errors keep the old items and record the error. A feed failing 10 times in a row is flagged, not deleted. | M | 1 |
| IN02-F027 | WebSub and push | Not supported: it needs a public callback address. Stated in help. | M | 1 |
| IN02-F028 | Refresh log | Table of the last 500 fetches: time, feed, status, bytes, new items, route, duration. | S | 2 |

### 4.3 Reading

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F029 | Three-pane layout | Sources sidebar, article list, article view. Also two-pane (list above view) and list-only with article in a full-width page. | M | 1 |
| IN02-F030 | Article list | Virtualised `modelListView`: title, feed, date, excerpt (2 lines), unread dot, star, enclosure icon, tag chips. Compact, normal and card (with thumbnail) row styles. | M | 1 |
| IN02-F031 | Sort and group | Newest or oldest first; group by day, by feed, or none. Per source remembered. | M | 1 |
| IN02-F032 | Filter bar | All, Unread, Starred, Has enclosure, Today; quick text filter on title. | M | 1 |
| IN02-F033 | Article view | Feed content is sanitised in a helper to the restricted rich format and shown by the restricted rich-content view (FUSE-WP-42). No script, no forms, no frames, no remote styles. | M | 1 |
| IN02-F034 | Image policy | Default: remote images are not loaded; placeholders show the host and size if known. Per feed and global setting: never, on click, proxied. | M | 1 |
| IN02-F035 | Proxied images | When allowed, images are fetched by the image fetch helper: no cookies, no referrer, fixed user agent, feed's route, 1×1 and tracking-host images dropped, size cap 8 MiB, decoded in the helper to pixels. | M | 2 |
| IN02-F036 | Tracker removal | Sanitiser strips tracking pixels (images at or under 3×3 px or with known tracker hosts), `utm_*` and similar parameters from links, and redirect wrappers where the target is in the URL. | M | 1 |
| IN02-F037 | Link safety | Links show the real target in the status bar; opening goes to IN01 by intent. Option: open in a disposable browser session. | M | 1 |
| IN02-F038 | Mark read behaviour | On open (default), on scroll past, or manual only. `M` toggles. Mark above or below as read. | M | 1 |
| IN02-F039 | Next unread navigation | `N` and `Shift+N` jump to next or previous unread across feeds in sidebar order; `Space` pages down then goes to the next unread. | M | 1 |
| IN02-F040 | Star | `S`. Starred articles are never purged by retention. | M | 1 |
| IN02-F041 | Article tags | `T` opens a tag input with completion. Tags appear in the sidebar. | S | 2 |
| IN02-F042 | Notes and highlights | Highlight a selection (4 colours) and attach a note; stored with offsets and quoted text so they survive re-fetch. | C | 3 |
| IN02-F043 | Typography settings | Font, size, line width, line height, theme (follow system, light, dark, sepia), justified off by default. | S | 1 |
| IN02-F044 | Code and maths | Code blocks in monospace with horizontal scroll; MathML shown where the rich view supports it, else as source. | C | 3 |
| IN02-F045 | Tables and embeds | Tables render in the rich view. Embedded players (`iframe`, `video`) become a card: "Video at host — Open in Video / Open in browser". | M | 2 |
| IN02-F046 | Open original | `O` opens the article link in IN01; `Shift+O` in a disposable session. | M | 1 |
| IN02-F047 | Article properties | Inspector: feed, author, dates (published, updated, fetched), link, GUID, categories, enclosures, route used, size, sanitiser report (what was removed). | S | 2 |
| IN02-F048 | Read aloud | Sends article text to the system speech service used by AX01. | C | 3 |
| IN02-F049 | Unread counts | Per feed, folder, tag and total; total shown as a badge on the launcher through `svc.notify` (setting). | M | 1 |
| IN02-F050 | Reading position | Scroll position per article is kept for 30 days. | S | 2 |

### 4.4 Full text and offline

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F051 | Full-text fetch | `F` fetches the article page with the fetch helper and extracts the main content in the extraction helper (density and structure heuristics, plus per-site rules). Result replaces the excerpt, original kept. | M | 2 |
| IN02-F052 | Per-feed full-text mode | Off, on demand, always (at refresh). "Always" is limited to 20 pages per refresh per feed. | S | 2 |
| IN02-F053 | Site extraction rules | Optional signed data file of CSS-selector rules per host; user rules editable per feed (content selector, strip selectors). | S | 3 |
| IN02-F054 | Offline store | Article bodies are stored on arrival. Images are stored only if the image policy allowed fetching them. | M | 1 |
| IN02-F055 | Retention | Per feed: keep N days (default 90), keep N items, or keep all. Starred, tagged, noted and read-later items are exempt. Purge runs daily. | M | 1 |
| IN02-F056 | Storage report | Bytes per feed (text, images, enclosures) in a `tableView`; delete cached images or enclosures per feed. | S | 2 |
| IN02-F057 | Read later | A list for single pages: added by intent `share` from IN01 or any app, by `Ctrl+L` paste, or by `L` on an article. The page is fetched once, extracted and stored. | M | 2 |
| IN02-F058 | Read-later archive option | When the browser passes a `.pagearc` archive (IN01), Feeds stores and shows that instead of fetching again. | C | 3 |
| IN02-F059 | Offline indicator | When no route is up, the status bar says "Offline" and refresh actions are disabled with the reason. | M | 1 |
| IN02-F060 | Export article | Save as Markdown, HTML (sanitised, single file), EPUB (selection of articles as one book), PDF through print; send to OF02 Notes. | S | 2 |

### 4.5 Podcasts

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F061 | Podcast detection | A feed whose items carry audio or video enclosures is shown in the Podcasts section with show artwork (subject to image policy). | M | 2 |
| IN02-F062 | Episode list | `tableView`: title, date, duration, size, state (new, queued, downloading, downloaded, in progress, played). | M | 2 |
| IN02-F063 | Episode download | Manual or automatic (newest N, default 0 = manual). Resume with range requests; checksum is recorded; stored under the app data directory or a portal folder the user picked. | M | 2 |
| IN02-F064 | Download rules | Only on unmetered links; size limit per episode; keep at most N per show; delete after played (after 24 h). | S | 2 |
| IN02-F065 | Play hand-off | `P` sends the local file handle (or the stream URL plus route if not downloaded) to ME02 for audio or ME03 for video by intent `open` with a `podcast` hint carrying episode id, start position and chapters. | M | 2 |
| IN02-F066 | Position return | The player reports position and "finished" back over a small IPC interface `org.pbsd.feeds.episode-state`. Feeds stores `position_ms` and `played`. | M | 2 |
| IN02-F067 | Queue | Ordered "Up next" list; add, reorder by drag, remove. Passed to the player as a playlist. | S | 2 |
| IN02-F068 | Show notes and chapters | Notes rendered like an article. Chapters from the podcast namespace JSON or from the media file (read by the player) listed with times. | S | 3 |
| IN02-F069 | Transcripts | If the feed links a transcript (SRT, VTT, JSON), it is fetched on demand and shown beside the notes; searchable. Local transcription is ME06's function; offer "Transcribe locally" hand-off (C). | C | 3 |
| IN02-F070 | Per-show settings | Playback speed, skip intro and outro seconds, auto-download, retention; passed to the player as hints. | S | 3 |
| IN02-F071 | Enclosure hand-off to Transfers | Large enclosures may be given to IN03 with the feed's route; state is mirrored back. | C | 3 |
| IN02-F072 | Streaming without download | Allowed only on explicit action; shows `[net]`; the player fetches through the same route. | S | 2 |
| IN02-F073 | No listening telemetry | Tracking prefixes in enclosure URLs (redirect chains through measurement hosts) are shown; option to try the final URL directly. No play counts are sent anywhere. | S | 3 |
| IN02-F074 | Podcast search | Only over subscribed shows and stored notes. No directory service is queried. | M | 2 |

### 4.6 Filters, rules and smart views

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F075 | Rules | Conditions on title, body, author, category, feed, folder, link host, has enclosure, age, language (detected locally). Operators: contains, not contains, equals, matches regular expression (linear-time engine), before, after. All or any. | M | 2 |
| IN02-F076 | Rule actions | Mark read, star, tag, hide (kept but not counted), delete, notify, add to read later, fetch full text, download enclosure, move to queue. | M | 2 |
| IN02-F077 | Rule scope and order | Global, per folder or per feed; ordered list; "stop processing" flag; rules run at arrival and can be run on existing items. | M | 2 |
| IN02-F078 | Rule test | A "Test" pane lists which stored articles a draft rule would match before saving. | S | 2 |
| IN02-F079 | Smart views | Saved searches shown in the sidebar: Unread, Today, Starred, Read later, plus user-defined (same condition language as rules). | M | 2 |
| IN02-F080 | Duplicate folding | Articles with the same link or same normalised title within 48 h across feeds are folded under one row ("also in 2 feeds"). | S | 3 |
| IN02-F081 | Mute words | Quick list of words that hide matching titles everywhere; a shortcut for the most common rule. | S | 2 |
| IN02-F082 | Notifications | Per feed or by rule only; never for all feeds by default. Sent through `svc.notify`, grouped per refresh, honours do-not-disturb. | S | 2 |

### 4.7 Search

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F083 | Full-text search | SQLite FTS5 index over title, author, body text, tags, notes, transcripts. Results in the article list with matched terms marked. | M | 2 |
| IN02-F084 | Search syntax | Words, "phrases", `-exclude`, `feed:`, `folder:`, `tag:`, `author:`, `is:unread`, `is:starred`, `has:enclosure`, `before:`, `after:`. | S | 2 |
| IN02-F085 | Find in article | `Ctrl+F` when the article view has focus; next `Ctrl+G`. | M | 1 |
| IN02-F086 | System search | Starred and read-later articles are offered to `svc.search` (setting; off for feeds on the Tor route). | S | 3 |
| IN02-F087 | Save search as smart view | One button in the search bar. | S | 2 |

### 4.8 Import, export and sharing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F088 | OPML import | OPML 1.0 and 2.0 with nested outlines as folders; preview tree with tick boxes; duplicates marked; no network until the user confirms. | M | 1 |
| IN02-F089 | OPML export | All or a folder; includes the extension attributes `pbsd:route`, `pbsd:interval`, `pbsd:fulltext` in a namespace; an option leaves them out. | M | 1 |
| IN02-F090 | Starred export | JSON Feed file or a folder of Markdown files with front matter. | S | 2 |
| IN02-F091 | Full backup | One archive with database dump, rules, settings and optionally cached media; restore from it. Also covered by `svc.backup`. | S | 2 |
| IN02-F092 | Share article | Intent `share` with title, link (cleaned) and optional excerpt: to CO01 Mail, CO05 Messenger, OF02 Notes. | M | 2 |
| IN02-F093 | Copy as | Link, Markdown link, title and link, quote with source. | S | 1 |
| IN02-F094 | Print | Article through `svc.portal.print` with the reader typography. | S | 2 |
| IN02-F095 | Newsletter intake | Handles intent `share` of a mail message from CO01 (already sanitised to the rich format by Mail). Stored in the "Newsletters" source, grouped by sender. Feeds never reads a mailbox. | S | 3 |
| IN02-F096 | Newsletter rules in Mail | Documented recipe: a CO01 filter that forwards a sender's messages to Feeds automatically. The rule lives in Mail. | C | 3 |

### 4.9 Network routes and privacy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F097 | Per-feed route | Clear, VPN profile or Tor, from the routes SE03 offers. Default set per folder or globally. The feed's page fetches, images and enclosures use the same route. | M | 2 |
| IN02-F098 | Fail closed | A feed whose route is down is skipped and marked "Route down"; it is never fetched on another route. | M | 2 |
| IN02-F099 | Route isolation | One fetch helper instance per route; connection pools, DNS and TLS session caches are not shared across routes. | M | 2 |
| IN02-F100 | Timing spread on Tor | Feeds on the Tor route are refreshed at independent random times, not as a batch, to weaken linking by timing. | S | 3 |
| IN02-F101 | First-contact log | Each new host contacted is recorded in `svc.audit` with the feed that caused it. | M | 1 |
| IN02-F102 | Endpoint view | The `[net]` badge lists hosts contacted in this session per route, with bytes. | M | 1 |
| IN02-F103 | Lock | Optional: the app's stores live in an SE05 vault; when locked the window shows only "Unlock". | C | 3 |

### 4.10 Sync with self-hosted services

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F104 | Account types | Local only (default); Fever API; Google-Reader-compatible API (as offered by several self-hosted servers); Miniflux API v1. Stored in `svc.accounts`, secrets in `svc.keyring`. | S | 3 |
| IN02-F105 | Capability per server | Adding an account asks the shell for `net.host:<server>:<port>`; the sync helper can reach only that host. | M | 3 |
| IN02-F106 | Synced state | Subscriptions, folders or categories, read and starred state. With a server account the server fetches feeds; Feeds downloads items from it. | S | 3 |
| IN02-F107 | Mixed mode | Local feeds and server accounts side by side; each account is a top-level source. | S | 3 |
| IN02-F108 | Offline changes | Read and star changes are queued in an outbox and replayed in order; conflicts resolve to "read wins", "starred wins". | S | 3 |
| IN02-F109 | Server capability table | Features each API lacks (for example tags or folders in Fever) are disabled in the UI for that account with a tooltip. | S | 3 |
| IN02-F110 | TLS requirements | HTTPS required; self-signed servers need the certificate pinned by fingerprint at set-up. | M | 3 |

### 4.11 Local AI (optional)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F111 | Summary | `Ctrl+Shift+M`: 3–5 sentence summary of the open article through `svc.model`; shown above the article in a labelled, collapsible panel; cached with the model id. | C | 3 |
| IN02-F112 | Daily digest | One page listing today's unread grouped by topic with one-line summaries; generated on demand only. | C | 3 |
| IN02-F113 | Translate article | Local translation through `svc.model` when a model for the pair is installed. | C | 3 |
| IN02-F114 | Suggested tags | Proposes tags from the user's existing tag set; never applied without confirmation. | C | 3 |
| IN02-F115 | AI is off by default | `ai.local` is not requested until the user turns a feature on; article text goes only to the local model service. | M | 3 |

### 4.12 Settings, accessibility and recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN02-F116 | Preferences | Pages: General, Reading, Refresh, Images and privacy, Podcasts, Routes, Accounts, Storage, Shortcuts. | M | 1 |
| IN02-F117 | Single-key shortcuts | Vim-style and reader-style single keys in the list and article; can be turned off for screen-reader users. | M | 1 |
| IN02-F118 | Session restore | Selected source, article, scroll and filters return after restart or crash. | M | 1 |
| IN02-F119 | Database integrity | Start-up `PRAGMA quick_check`; on failure offer restore from the last `svc.backup` snapshot or rebuild from feeds (keeps subscriptions). | M | 2 |
| IN02-F120 | Undo | Mark-all-read, delete, move, tag changes and rule runs are undoable for the session (`Ctrl+Z`). | M | 1 |
| IN02-F121 | Command palette | Every action, feed and folder name is searchable in `Ctrl+K`. | M | 1 |
| IN02-F122 | Statistics | Per feed: items per week, share read, share starred; helps prune. All local. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Feeds window
├─ Header bar: menu, title, search, [net] badge
├─ Sidebar (F9): smart views · folders and feeds · tags · podcasts · read later · newsletters · accounts
├─ Article list: filter bar, list, selection bar
├─ Article view: toolbar, summary panel (optional), content, enclosure bar, find bar
├─ Inspector (F4): article properties | feed properties
└─ Status bar: refresh progress, offline or route state, counts
Dialogs and sheets
├─ Add feed · Feed properties · Folder properties · Import OPML · Export
├─ Rules manager · Rule editor · Smart view editor
├─ Add account (sync) · Storage report · Refresh log
├─ Podcast view (episode table + queue) · Episode properties
└─ Preferences
Shell-drawn: file chooser, print, share sheet, capability prompts (net.host for a sync server)
```

### 5.2 Main window

Shown at 1280×800, three-pane, normal density.

```
+----------------------------------------------------------------------------------------+
|[=] Feeds — Tech / Kernel news           [search ⌘K]                [net] [▁] [▢] [✕]  | (1)
+--------------------+-----------------------------+-------------------------------------+
| [+ Add] [⟳]        | [All|Unread|Starred] [filter]| [✓][☆][Full][Tag][Open][⋯]         | (2)
| Unread          42 +-----------------------------+-------------------------------------+
| Today           11 | ● Scheduler rework lands    | Scheduler rework lands in 15        | (5)
| Starred          7 |   Kernel news · 09:12       | Kernel news · A. Writer · 12 Nov    |
| Read later       3 |   The long-planned change…  | Route: Clear · images: 3 not loaded |
| ------------------ | ● Capsicum in practice    ☆ |   [Load images once]                |
| ▾ Tech          31 |   Sec blog · 08:40          +-------------------------------------+
|   > Kernel news  9 |   How capability mode…      |                                     |
|     Sec blog     4 |   Release 4.2 is out      ♪ | The long-planned change to the      |
|     Releases    18 |   Releases · Yesterday      | scheduler landed this week. It      | (6)
| ▸ Science        6 |   Fixes and one new…        | replaces…                           |
| ▸ Local          5 |   ── Yesterday ──           |                                     |
| ------------------ |   Quiet week in the tree    | [image: cdn.example · 640×360]      |
| Tags               |   Kernel news · Mon         |                                     |
|   #zfs  #papers    |   …                         |                                     |
| Podcasts         2 |                             +-------------------------------------+
| Newsletters      1 |                             | ♪ episode.ogg 48 min [Play][Download]| (7)
+--------------------+-----------------------------+-------------------------------------+
| Refreshing 12 of 64 ▓▓▓░░░░ [Cancel]                    Offline: no · 42 unread        | (8)
+----------------------------------------------------------------------------------------+
  (3) sidebar            (4) article list
```

States: empty (no feeds), loading (first refresh), error (feed broken: banner in the list), offline, route down (per feed), locked (vault), read-only (server account unreachable: local changes queue), narrow.

1. Header bar. Title is "folder / feed". `[net]` lists hosts by route.
2. Toolbars. Sidebar: Add `button`, Refresh `toolButton`. List: filter `tabBar` and `lineEdit`. Article: mark read, star, full text, tag, open original, overflow (`toolButton`s).
3. Sidebar, 220 px, `treeView` with count column; sections separated by `separator`. Broken feeds show a `--warn` triangle and the word in the tooltip.
4. Article list, 320 px (resizable `splitter`), `modelListView`. Unread: dot plus `--fg-0` bold title. Read: `--fg-1`. Group headers are sticky.
5. Article header: title 18 pt, feed, author, date; privacy line stating route and how many images were not loaded, with a `link` to load them once.
6. Article body: restricted rich-content view, max line width from settings, centred.
7. Enclosure bar, 40 px, only when the item has enclosures.
8. Status bar: `progressBar`, cancel, offline and route state, counts.

Narrow (<640): one pane at a time: sidebar → list → article, with Back (`Alt+Left`). Medium: sidebar collapses to icons.

### 5.3 Other views and dialogs

#### 5.3.1 Add feed (dialog, 520×360)

```
+--------------------------------------------------------------+
| Add a feed                                                   |
| Address [ https://blog.example.org                         ] |
| Route   [ Clear v ]      (this request goes to blog.example) |
|                                              [Find feeds]    |
+--------------------------------------------------------------+
| Found 2 feeds                                                |
| (•) Example Blog — Atom · 20 items · updated today           |
| ( ) Example Blog comments — RSS · 50 items                   |
| Folder  [ Tech v ]   Refresh [ Default (60 min) v ]          |
| [ ] Fetch full text   Images [ Not loaded v ]                |
|                                   [Cancel] [Add feed]        |
+--------------------------------------------------------------+
```

`dialog`, `lineEdit`, `comboBox`, `radioButton` list, `checkBox`, `button`. States: searching (spinner, Cancel), none found (inline message, "Add this address anyway"), TLS error (reason, no bypass for HTTPS; offer HTTP only if the user typed `http://`), duplicate.

#### 5.3.2 Feed properties (inspector or dialog, 480×560)

```
+----------------------------------------------------------+
| Kernel news                                    [Pause]   |
| Title    [ Kernel news                 ]                 |
| Address  [ https://news.example/atom.xml ] [Copy]        |
| Folder   [ Tech v ]     Tags [ #kernel × ] [ + ]         |
| Refresh  [ Every 30 min v ]                              |
| Route    [ Clear v ]                                     |
| Full text [ On demand v ]   Images [ Proxied v ]         |
| Keep     [ 90 days v ]   [x] Never delete starred        |
| Notify   [ ] On new articles                             |
| Login    [ None v ]                                      |
+----------------------------------------------------------+
| Health   ✓ Last fetch 09:12 · 304 Not Modified           |
|          7.2 items/week · 0 failures · Atom 1.0          |
|          [Refresh log]  [Storage: 4.1 MiB]               |
+----------------------------------------------------------+
```

`propertyGrid`-style form built from `lineEdit`, `comboBox`, `checkBox`; health block uses the trust badge pattern.

#### 5.3.3 Rule editor (dialog, 640×480)

```
+------------------------------------------------------------------------+
| Rule  [ Hide sponsored posts            ]   Scope [ All feeds v ]      |
| When ( ) all  (•) any  of these match                                  |
|  [ Title v ] [ contains v ]        [ sponsored          ]  [−]         |
|  [ Body  v ] [ matches regex v ]   [ (?i)paid\s+post    ]  [−]         |
|  [+ condition]                                                         |
| Then                                                                   |
|  [ Hide v ]                                               [−]          |
|  [ Mark read v ]                                          [−]          |
|  [+ action]            [x] Stop processing later rules                 |
+------------------------------------------------------------------------+
| Test: 14 of 2 310 stored articles match        [Show matches]          |
|                               [Cancel] [Run on existing] [Save rule]   |
+------------------------------------------------------------------------+
```

`comboBox`, `lineEdit`, `button`, `checkBox`, `label`. An invalid regular expression shows the error under the field on blur.

#### 5.3.4 Podcast view (content area, 1060×700)

```
+--------------------------------------------------------------------------------------+
| [art] The Systems Show                         [Refresh] [Show settings] [Unsubscribe]|
|       Weekly · 212 episodes · Route: VPN home-vpn                                    |
+--------------------------------------------------------+-----------------------------+
| Title                    Date     Length  Size  State  | Up next                     |
| > 212 Capability systems 10 Nov   58:12   41 M  ▣ done | 1. 212 Capability systems   |
|   211 Allocators         03 Nov   47:02   33 M  ▶ 12:40| 2. News hour 88             |
|   210 Filesystems        27 Oct   62:30   45 M  ↓ 40%  | 3. 211 Allocators           |
|   209 Listener mail      20 Oct   31:10   22 M  new    | [Clear] [Play queue]        |
+--------------------------------------------------------+-----------------------------+
| Show notes · Chapters · Transcript                                                   |
| In this episode we talk about …                                                      |
| [Play] [Add to queue] [Download] [Mark played]                                       |
+--------------------------------------------------------------------------------------+
```

`tableView`, `listView` (queue, drag reorder), `tabWidget` for notes, chapters, transcript; `image` for artwork (placeholder unless images allowed).

#### 5.3.5 Import OPML (dialog, 560×520)

```
+----------------------------------------------------------+
| Import subscriptions from feeds.opml                     |
| 64 feeds in 6 folders · 3 already subscribed             |
| [x] ▾ Tech (22)                                          |
| [x]     Kernel news           news.example               |
| [ ]     Sec blog              already subscribed         |
| [x] ▸ Science (9)                                        |
| Route for imported feeds [ Clear v ]                     |
| [ ] Refresh all after import (64 requests)               |
|                              [Cancel] [Import 61 feeds]  |
+----------------------------------------------------------+
```

`treeView` with check boxes. Nothing is fetched unless the refresh box is ticked.

#### 5.3.6 Add sync account (dialog, 520×380)

```
+----------------------------------------------------------+
| Connect to your own server                               |
| Type    ( ) Fever  (•) Miniflux  ( ) Google Reader API   |
| Server  [ https://rss.home.example             ]         |
| Token   [ ••••••••••••••••            ] [reveal]         |
| Route   [ Clear v ]                                      |
| This lets Feeds reach rss.home.example:443 only.         |
|                             [Cancel] [Test and connect]  |
+----------------------------------------------------------+
```

After "Test and connect" the shell shows the `net.host` prompt; on success the account appears as a source.

### 5.4 Menus

```
File     Add feed… Ctrl+N · New folder Ctrl+Shift+N · Add to read later… Ctrl+L ·
         Import OPML… · Export OPML… · Export starred… · Back up… · Print… Ctrl+P · Close Ctrl+W · Quit Ctrl+Q
Edit     Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Ctrl+C · Copy link Ctrl+Shift+C · Select all Ctrl+A ·
         Find in article Ctrl+F · Search all Ctrl+Shift+F · Preferences Ctrl+,
View     Sidebar F9 · Inspector F4 · Layout ▸ (Three-pane, Two-pane, List) · Row style ▸ ·
         Sort ▸ · Group ▸ · Filter ▸ · Zoom in Ctrl++ · Zoom out Ctrl+- · Reset Ctrl+0 · Full screen F11
Article  Next unread N · Previous unread Shift+N · Toggle read M · Star S · Tag… T · Full text F ·
         Read later L · Open original O · Open in disposable session Shift+O · Load images I ·
         Summarise Ctrl+Shift+M · Share… · Export ▸ · Delete Delete
Feed     Refresh F5 · Refresh all Ctrl+F5 · Mark all read Shift+A · Properties… Alt+Enter ·
         Pause · Rename F2 · Move to ▸ · Unsubscribe… · Refresh log · Broken feeds
Podcast  Play P · Add to queue Q · Download D · Mark played · Play queue · Show settings…
Tools    Rules… Ctrl+R · Smart views… · Accounts… · Storage… · Statistics
Help     Feeds help F1 · Keyboard shortcuts · About
```

### 5.5 Keyboard and pointer

| Keys | Action | Keys | Action |
|---|---|---|---|
| `J` / `K` | next, previous article | `H` / `;` | previous, next feed |
| `Space` / `Shift+Space` | page down or next unread, page up | `Enter` | open article (list-only layout) |
| `G` then `U`, `S`, `L` | go to Unread, Starred, Read later | `/` | focus quick filter |
| `Ctrl+Shift+A` | mark above as read | `Esc` | close find, leave article |

Context menus: feed (Refresh, Mark all read, Properties, Pause, Rename, Move, Copy address, Unsubscribe); folder (Refresh, Mark all read, New folder, Rename, Delete); article (Open, Open original, Toggle read, Star, Tag, Read later, Full text, Copy link, Share, Mark above read, Mark below read, Delete, Make rule from this); link in article (Open in browser, Open in disposable session, Copy link, Copy clean link, Add to read later); image placeholder (Load this image, Always load from this feed, Copy address); episode (Play, Queue, Download, Delete download, Mark played, Properties).

Drag and drop: feeds and folders within the sidebar; an OPML file onto the window starts import; a URL or text link onto the sidebar starts Add feed; a URL onto "Read later" adds it; an article out of the list offers `text/uri-list`, `text/markdown` and `text/plain`; episodes onto the queue.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | "No feeds yet. Add a feed or import an OPML file." with two buttons. No network. |
| Loading | First refresh shows per-feed progress in the sidebar (spinner replaces the count). |
| Error | Feed row shows `--warn` icon; list banner gives the HTTP or parse error with "Retry" and "Properties". |
| Offline | Status "Offline"; reading, search, rules and export all work; refresh is disabled with the reason. |
| Route down | Affected feeds are marked; banner offers "Open Anonymity". |
| Locked | Vault locked (F103): single Unlock button. |
| Read-only | Sync server unreachable: changes queue; a chip shows "3 changes waiting". |
| Narrow | One pane at a time (5.2). |

### 5.7 Widgets

| Region | FUSE widget | Note |
|---|---|---|
| Sidebar | `treeView` | count column, drag and drop (FUSE-WP-28, FUSE-WP-33) |
| Article list | `modelListView` over `SqliteModel` | sticky group headers needed (FUSE-WP-28) |
| Article view | restricted rich-content view | FUSE-WP-42 on FUSE-WP-35; gaps G-10, G-14 (restricted path) |
| Tag input | tag input | listed as missing in the FUSE plan; IN02-GAP-1 until built |
| Episode table, logs, storage | `tableView` | |
| Forms | `lineEdit`, `comboBox`, `checkBox`, `radioButton`, `spinBox`, `toggleSwitch` | |
| Statistics | `chart` | |
| Panes | `splitter`, `scrollArea`, `tabWidget`, `dialog`, `popupMenu`, `progressBar` | |

Gaps: **IN02-GAP-1** tag input widget; **IN02-GAP-2** a background timer service so refresh does not need a resident app process (today `bg.run` keeps the app alive); **IN02-GAP-3** an agreed "episode state" interface with ME02 and ME03.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Subscriptions, scheduler, rules engine, read state, queue, OPML, view models. |
| UI | FUSE UI | app host | Windows and dialogs of section 5. |
| Fetch helper | C++23 (safe subset) on the platform protocol helper (FORGE-WP-21) | one per route | HTTP requests for feeds, pages, images, enclosures, sync APIs. Holds sockets from the network service only. Returns bytes as descriptors. |
| Parse helper | C++23 (safe subset) | pool, no network, no files | XML (own non-validating parser, no DTD, no external entities), JSON, feed normalisation, HTML sanitising to the rich format, tracker stripping. |
| Extraction helper | C++23 (safe subset) | on demand, no network | Main-content extraction from fetched pages; same code as IN01's reader helper (shared source, separate binary). |
| Image helper | C++23 (safe subset) | on demand, no network | Decodes fetched images to pixels, rejects tracking sizes, re-encodes to a cache format. |
| Sync helper | C++23 (safe subset) | one per account | Fever, Google-Reader-compatible, Miniflux clients; reaches one host. |
| Store | SQLite (through FUSE `SqliteModel`, Verified V8) | app host | Database and FTS index. |

```
 app host: logic (ForgeScript) + UI (FUSE) + SQLite
   │ bus                       ┌─ fetch helper [route Clear] ── network service ── internet
   ├───────────────────────────┼─ fetch helper [route Tor]   ── network service ── SE03 route
   │  bytes (fd)               └─ sync helper [rss.home.example:443]
   ├─ parse helper (no net) ──► normalised items + rich-format bodies
   ├─ extraction helper (no net) ─► full-text bodies
   ├─ image helper (no net) ──► pixels to cache
   └─ svc.intent ─► ME02 / ME03 (play) · IN01 (open) · CO01 (newsletters in)
```

Bytes from the network never enter the app host unparsed: fetch helper writes to an anonymous descriptor, the parse helper reads it and returns schema-checked records.

### 6.2 Data model

Store: `~/.local/share/org.pbsd.feeds/feeds.db` (SQLite, WAL), `schema_version` in `meta`. Media under `media/<feed_id>/`. Cached images under `~/.cache/org.pbsd.feeds/img/`.

| Entity | Fields |
|---|---|
| `account` | `id u32`, `kind enum{local,fever,greader,miniflux}`, `name text`, `base_url text`, `keyring_ref text`, `route enum`, `cert_pin blob32?`, `last_sync i64` |
| `folder` | `id u32`, `account_id u32`, `parent_id u32?`, `name text`, `position u32`, `default_route enum?` |
| `feed` | `id u32`, `account_id u32`, `folder_id u32?`, `url text`, `site_url text`, `title text`, `title_override text?`, `format enum{rss,atom,jsonfeed,newsletter,local}`, `etag text?`, `last_modified text?`, `interval_s u32?`, `route enum{inherit,clear,vpn,tor}`, `route_profile text?`, `fulltext enum{off,demand,always}`, `images enum{never,click,proxied}`, `keep_days u32?`, `keep_items u32?`, `notify bool`, `paused bool`, `auth_kind enum{none,basic,bearer}`, `keyring_ref text?`, `is_podcast bool`, `last_ok i64?`, `last_error text?`, `fail_count u32`, `next_due i64` |
| `item` | `id u64`, `feed_id u32`, `guid text`, `link text`, `title text`, `author text`, `published i64`, `updated i64?`, `fetched i64`, `summary_rich blob`, `body_rich blob`, `fulltext_rich blob?`, `lang text?`, `read bool`, `starred bool`, `hidden bool`, `read_later bool`, `scroll_pos u32`, `remote_id text?`, `content_hash blob32` |
| `enclosure` | `id u64`, `item_id u64`, `url text`, `mime text`, `length u64?`, `duration_s u32?`, `local_path text?`, `sha256 blob32?`, `state enum{none,queued,downloading,done,failed}`, `position_ms u64`, `played bool` |
| `tag`, `item_tag`, `feed_tag` | `tag(id, name, colour)`, link tables |
| `rule` | `id u32`, `name text`, `scope_kind enum{global,folder,feed}`, `scope_id u32?`, `match enum{all,any}`, `conditions json`, `actions json`, `stop bool`, `position u32`, `enabled bool` |
| `smart_view` | `id u32`, `name text`, `query text`, `position u32` |
| `queue` | `position u32`, `enclosure_id u64` |
| `note` | `id u64`, `item_id u64`, `start u32`, `end u32`, `quote text`, `colour u8`, `text text` |
| `fetch_log` | `time i64`, `feed_id u32`, `status u16`, `bytes u32`, `new_items u16`, `route enum`, `ms u32` |
| `outbox` | `id u64`, `account_id u32`, `op enum{read,unread,star,unstar,subscribe,unsubscribe,move}`, `target text`, `created i64` |
| `item_fts` | FTS5 virtual table over `title, author, text, tags, notes` |

The "rich format" blobs are the native binary rich format of FUSE-WP-35 (Proposed). Migration: copy the database, migrate forward, keep the copy until the next successful start.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| RSS 0.9x/1.0/2.0, Atom 1.0, Media RSS, iTunes and podcast namespaces | yes | — | own XML parser and mappers | parse helper |
| JSON Feed 1.0/1.1 | yes | yes (starred export) | own code | parse helper |
| OPML 1.0/2.0 | yes | yes | own code | parse helper (read), app host (write) |
| HTML in items and pages | yes | sanitised HTML export | own tree-building parser (shared with Track A of IN01 6.5) | parse and extraction helpers |
| HTTP/1.1, HTTP/2, TLS | yes | yes | FORGE-WP-21 protocol helper | fetch helper |
| PNG, JPEG, GIF, WebP, AVIF | yes | cache format | platform image decoders | image helper |
| Fever API, Google-Reader-compatible API, Miniflux API v1 | yes | yes | own clients (JSON, form posts) | sync helper |
| SRT, WebVTT, podcast transcript JSON, chapters JSON | yes | — | own code | parse helper |
| Markdown, EPUB 3 | — | yes | own writers | app host (Markdown), parse helper (EPUB packaging) |
| Mail message hand-off (rich format from CO01) | yes | — | schema-checked record | app host |

### 6.4 Integration

- Intents handled: `open` for `feed:` URLs and OPML files; `share` of a URL (read later) and of a mail message (newsletter); "subscribe" from IN01. Emitted: `open` (links to IN01, episodes to ME02 and ME03), `share`, `print`.
- MIME: view and import `text/x-opml+xml`, `application/rss+xml`, `application/atom+xml`, `application/feed+json`; export `text/x-opml+xml`, `text/markdown`, `application/epub+zip`, `application/feed+json`.
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.accounts`, `svc.power`, `svc.model`, `svc.backup`, `svc.audit`, `svc.media` (read-only now-playing to show state).
- IPC offered: `org.pbsd.feeds.episode-state` v1: `report(episode_id u64, position_ms u64, finished bool)`; callable only by ME02 and ME03.
- Other apps: IN01 (open links, subscribe, `.pagearc`), ME02 and ME03 (playback), CO01 (newsletters), IN03 (large enclosures), SE03 (routes), SE05 (vault), OF02 (notes), AI02 (models).
- Backing repos: none. Verified facts used: `SqliteModel` exists (PLATFORM V8). Everything else is Proposed.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Database, media, cache. |
| `fs.portal` | OPML import and export, exports, optional episode folder. |
| `net.user` | Feed addresses are typed by the user; each first contact is logged. |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-feed routes; one fetch helper per route. |
| `net.host:<server>:<port>` | Added at run time per sync account, by prompt. |
| `proc.helper:fetch`, `:parse`, `:extract`, `:image`, `:sync` | Section 6.1. |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.accounts`, `svc.power`, `svc.backup`, `svc.audit`, `svc.media` | Section 6.4. |
| `svc.model`, `ai.local` | Summaries and translation; requested only when enabled. |
| `bg.run` | Scheduled refresh and episode downloads with no window. Off by default. |

Threat model:

| | |
|---|---|
| Assets | The subscription list (it reveals interests), reading history, feed logins, the route separation, the system behind the parsers. |
| Adversaries | A hostile or compromised feed; a network observer; trackers embedded in feed content; a hostile sync server; a hostile OPML file. |
| Attack surface | XML, JSON and HTML parsers; image decoders; HTTP client; sync API responses; enclosure files (handed to players); regular expressions in rules. |

Mitigations: all hostile bytes are parsed in helpers with no network and no file access; XML parser has no DTD or entity expansion; size and depth caps everywhere; article content cannot run script or load anything by itself; images are off by default and proxied when on; links are cleaned and opened by the browser, optionally disposable; routes fail closed and are isolated per helper; refresh timing has jitter; rule regular expressions use a linear-time matcher; enclosures get the same provenance and quarantine tags as IN01 downloads and are opened by players that parse in their own sandboxes.

Stored: section 6.2, on the user's disk only. Sent: requests to feed hosts, page hosts (full text), image hosts (only if allowed), enclosure hosts and the user's sync server. Nothing else. Residual: a feed server always learns the fetch time and the route's exit address; conditional requests carry an ETag that can act as an identifier for that one feed (it is dropped on Tor-route feeds when the "strict" setting is on).

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `layout.mode` | enum three/two/list | three | Pane layout. |
| `list.row_style` | enum compact/normal/card | normal | Article rows. |
| `read.mark_on` | enum open/scroll/manual | open | F038. |
| `read.font`, `read.size_pt`, `read.width_ch`, `read.theme` | string, int, int, enum | system, 15, 70, system | F043. |
| `keys.single_key` | bool | true | F117. |
| `refresh.interval_min` | int 15–10080 | 60 | Global interval. |
| `refresh.on_start` | bool | true | Refresh when the app opens. |
| `refresh.background` | bool | false | Needs `bg.run`. |
| `refresh.metered` | bool | false | Allow scheduled refresh on metered links. |
| `refresh.max_body_mib` | int 1–100 | 10 | F020. |
| `images.default` | enum never/click/proxied | never | F034. |
| `privacy.strip_trackers` | bool | true | F036. |
| `privacy.strict_tor` | bool | true | No ETag, spread timing on Tor feeds. |
| `privacy.open_links_disposable` | bool | false | F037. |
| `route.default` | enum clear/vpn/tor | clear | New feeds. |
| `retention.days` | int 0–3650 | 90 | 0 keeps all. |
| `podcast.auto_download` | int 0–10 | 0 | Newest N per show. |
| `podcast.keep_per_show` | int 0–100 | 5 | 0 keeps all. |
| `podcast.delete_after_played` | bool | true | After 24 h. |
| `podcast.episode_dir` | portal handle | app data | Where episodes go. |
| `notify.badge_unread` | bool | false | F049. |
| `search.contribute` | bool | false | F086. |
| `ai.summaries`, `ai.translate` | bool, bool | false, false | 4.11. |

## 9. Accessibility and languages

- The article view exposes headings, links, lists, tables and images with alternative text to `svc.a11y`; AX01 reading commands work as in documents.
- Single-key shortcuts can be turned off (`keys.single_key`), since they clash with screen-reader browse keys; every action also has a modified shortcut or a palette entry.
- Unread, starred and error states are spoken as words, not conveyed by weight or colour alone.
- New-article arrival is a polite live region ("12 new articles"), at most once per refresh.
- Article text direction follows the item's language and the first strong character; needs the FULL text profile (G-03).
- Dates use the locale; relative dates ("2 h ago") switch to absolute after 24 h.

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame | 150 ms; list filled within 250 ms |
| Database size supported | 2 000 feeds, 2 million items, 20 GiB media |
| List scroll | 4 ms frame with 100 000 rows in the model |
| Refresh of 300 feeds | under 60 s on a 20 Mbit/s link; mostly 304 responses |
| Parse of a 10 MiB feed | under 300 ms in the helper |
| Full-text search | first results in 100 ms over 2 million items |
| Memory | app host 120 MiB; each helper under 64 MiB |
| Idle | zero wake-ups between scheduled refreshes |

## 11. Testing and proof

Nothing has run. Targets only.

- Unit: URL normalisation, item identity, scheduler (intervals, jitter, back-off), rules engine, search syntax parser, OPML writer, outbox replay, retention.
- Golden renders: all views of section 5 in three themes; article view with a fixed corpus.
- Scripted UI through the a11y tree: tasks T1–T13.
- Corpus tests: a stored set of several thousand real-world feeds with known quirks; expected item counts and fields.
- Fuzz: XML parser, JSON parser, HTML sanitiser (oracle: output contains no script, no event attribute, no remote reference when images are off), OPML, sync API responses, transcript formats.
- Differential: feed parsing against a reference open-source feed parser on the corpus; extraction against a reference readability implementation.
- Network tests: with a route cut, no packet for a feed bound to it leaves on any interface; no request is made for an image when policy is "never".

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| XML and JSON parsers | No runtime errors; bounded depth and memory | T2 | PRISM job; fuzz | — |
| HTML sanitiser | Output is in the allow-list grammar (no script, no remote load) | T1 for the output check, T2 for the parser | PRISM proof of the serialiser; fuzz with oracle | HTML tree-builder size |
| Route binding in fetch helper | Requests leave only on the bound route | T1 (inherited from FORGE-WP-21 rule matcher) | As FORGE-WP-21 | Route mechanism Unverified |
| Rules engine | Terminates; linear-time matching | T2 | PRISM; property tests | — |
| Image helper | No runtime errors on hostile images | T2 | PRISM; fuzz | Decoder complexity; third-party decoders would be T4 |
| Sync clients | Parser safety | T2 | PRISM; fuzz | — |
| App logic | Memory safety by construction | T3 | ForgeScript guarantees; unit tests | — |

## 12. Build plan

| ID | Work item | Depends on |
|---|---|---|
| | **Phase 1: usable core** | |
| IN02-W01 | Store schema, migrations, `SqliteModel` views | G-02, G-07 |
| IN02-W02 | Fetch helper on the platform protocol helper; conditional requests; limits | FORGE-WP-21, G-08 |
| IN02-W03 | Parse helper: XML, JSON, RSS, Atom, JSON Feed mapping | FORGE-WP-28 |
| IN02-W04 | HTML sanitiser to the rich format; tracker stripping | FUSE-WP-35, FUSE-WP-42 |
| IN02-W05 | Three-pane UI, article list, article view, keyboard set | G-02, G-10, FUSE-WP-28 |
| IN02-W06 | Add feed with discovery; folders; properties; OPML import and export | W02, W03 |
| IN02-W07 | Scheduler, manual refresh, error handling, read and star state, undo | W01, W02 |
| IN02-W08 | Session restore, preferences, find in article | W05 |
| | **Phase 2: full feature** | |
| IN02-W09 | Full-text extraction helper; per-feed modes; read later | W04 |
| IN02-W10 | Image helper and proxied images | W02 |
| IN02-W11 | Routes per feed, fail closed, per-route helpers | SE03, IN01-GAP-4 |
| IN02-W12 | Rules, smart views, mute words, notifications | W07, G-09 |
| IN02-W13 | FTS search and syntax | W01 |
| IN02-W14 | Podcasts: detection, downloads, queue, hand-off and episode-state interface | ME02, ME03, G-11 |
| IN02-W15 | Background refresh, power and metered rules | `svc.power`, IN02-GAP-2 |
| IN02-W16 | Exports (Markdown, EPUB), share, print, storage report, health | G-16 |
| | **Phase 3: advanced** | |
| IN02-W17 | Sync helper: Miniflux, then Google-Reader-compatible, then Fever | `svc.accounts`, W07 |
| IN02-W18 | Newsletter intake from CO01 | CO01 |
| IN02-W19 | Local summaries, translation, tag suggestions | G-15, AI02 |
| IN02-W20 | Notes and highlights, transcripts, chapters, duplicate folding, statistics | W05, W14 |
| IN02-W21 | Vault lock, Tor timing spread, site extraction rule packs | SE05, W11 |

## 13. Open questions and risks

1. The restricted rich-content view (FUSE-WP-42) decides how good articles look. Complex layouts (floats, figures, wide tables) may degrade; "Open original" is the escape.
2. The episode-state interface needs agreement with ME02 and ME03, whose specs own playback.
3. Without a background timer service (IN02-GAP-2), scheduled refresh needs the app resident with `bg.run`.
4. SE03's backing repo is a consortium mixnet (`ANONYMOUS:README.md`); whether a Tor route will exist is not established here. Tor-route features degrade to "VPN or clear" if it does not.
5. Google-Reader-compatible servers differ in detail; a per-server quirks table will be needed.
6. Shared source between this app's extraction helper and IN01's reader helper needs a common library location in the build; not decided.
7. EPUB export quality depends on converting the rich format back to XHTML; scope may shrink to Markdown and sanitised HTML first.
