<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Clipboard

By Odin Loch

| | |
|---|---|
| ID | `AX05` |
| Bundle | `org.pbsd.clipboard` |
| Category | access |
| Priority | P2 |
| Backing repos | FUSE (`Source/FUSE/Core/include/fuse/platform/text_services.hpp`, `Source/FUSE/UI/Core/include/fuse/ui/ui.hpp`), FORGE (`docs/os-wrapper.md`, `docs/text-locale.md`), this repo (`pbsd/compositor`) |
| Status | Planned. Nothing built. |

## 1. Purpose

Clipboard is the user's view of `svc.clipboard`: a history of what was copied, pins, saved clips, search, paste transformations, and the rules that keep secrets out of the history. It also shows and edits the per-app clipboard policy of the OS, where an app reads the clipboard only when the user pastes. It replaces Klipper of the interim Plasma plan (PLATFORM V20).

## 2. Scope

In scope: the history popup; the manager window; pins and saved clips (snippets); search; content types (text, rich text, images, files, colours); sensitivity rules; per-app access policy and the content of paste prompts; transformations; optional end-to-end encrypted sync between the user's own devices on the local network; primary selection handling; quick paste shortcuts; hand-off of images to ME05; the behaviour contract of `svc.clipboard`.

Out of scope:

| Item | Owner |
|---|---|
| Drawing the paste prompt and other capability prompts | SH04 |
| Abbreviation-triggered text expansion while typing | AX04 (a saved clip can be sent there as a snippet) |
| Password generation, secret storage, the "copy password" action | SE04 (it marks what it copies as secret; this app obeys) |
| Image annotation and editing | ME05 |
| Drag and drop between apps | compositor, FUSE-WP-09 |
| Clipboard policy inside VMs and remote sessions | VM01, VM04 (they are ordinary apps under this policy on the host side) |
| The device-sync engine (pairing, transport, keys) | OF02's shared sync engine (ROADMAP section 4); this app is a client |
| The list of all capabilities of an app | AD05 (this app edits only the clipboard rows and links there) |

## 3. Users and top tasks

Personas: **Dana**, support worker, pastes the same ten replies all day. **Leo**, developer, copies several things then pastes them in order. **Asha**, low vision, wants a large, searchable list instead of remembering what she copied. **Kim**, journalist, must be sure passwords and source names never linger. **Ravi**, has a laptop and a desktop and wants to copy on one and paste on the other without any cloud. **Admin**, sets clipboard policy for a shared workstation.

| # | Task | Target steps |
|---|---|---|
| 1 | Paste something copied earlier | 2 (`Super+V`, `Enter` or a digit) |
| 2 | Find an old clip by a word in it | 3 (`Super+V`, type, `Enter`) |
| 3 | Paste as plain text | 1 (`Super+Shift+V`) |
| 4 | Pin a clip so it stays | 2 (`Super+V`, `Ctrl+P`) |
| 5 | Paste pinned clip number 3 | 1 (`Super+Ctrl+3`) |
| 6 | Copy a password and know it will vanish | 0 (SE04 marks it; a countdown shows in the indicator) |
| 7 | Clear the clipboard and history now | 1 (`Super+Alt+Delete` in the popup, or indicator menu) |
| 8 | See which app read the clipboard, and when | 2 (open manager, Access log) |
| 9 | Stop an app from reading the clipboard in the background | 2 (prompt: Deny always; or Apps page toggle) |
| 10 | Clean tracking parameters from a copied link | 0 (automatic rule) or 2 (Paste as › Clean link) |
| 11 | Send a clip to my other device | 2 (item menu, device) or 0 (auto sync on) |
| 12 | Copy five things, paste them in order | `Super+V`, `Ctrl+Q` starts a queue; then paste five times |
| 13 | Annotate a copied screenshot before pasting | 2 (item menu, Annotate) |
| 14 | Paste a colour as `rgb()` instead of hex | 2 (Paste as › format) |

## 4. Features

### 4.1 The OS clipboard model (contract of `svc.clipboard`)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F001 | One clipboard service | `svc.clipboard` is the only client the compositor lets use the clipboard-manager (data-control) interface. No app, including this app's UI, can watch the clipboard directly. | M | 1 |
| AX05-F002 | Copy needs a gesture | An app can set the clipboard only with the serial of a recent input event in its own window. | M | 1 |
| AX05-F003 | Read on paste only | An app receives clipboard data only when the user makes a paste gesture in it: the paste shortcut, a paste menu item, or middle click. The toolkit passes the gesture's serial; the compositor checks it. | M | 1 |
| AX05-F004 | Type list is not secret, content is | A focused app may see which types are on offer (so it can enable its Paste item) but not the bytes. | M | 1 |
| AX05-F005 | Paste prompt | A read with no gesture (background, at start-up, on a timer, on focus) shows an SH04 prompt: app, signer, what kind of content is on the clipboard (never the content), choices Deny, Allow once, Allow this session, Always. | M | 1 |
| AX05-F006 | Per-app policy | For each app: read without gesture (ask, allow, deny), copy (allow, deny), record its copies in history (yes, no, expire after N), may mark content sensitive (always yes). Defaults: ask, allow, yes. | M | 1 |
| AX05-F007 | Paste notice | Optional toast when an app reads content that came from a different app: "Text Editor pasted from Web Browser". Off by default; always on for reads allowed without a gesture. | S | 2 |
| AX05-F008 | Access log | Every read without a gesture, every denial and every policy change goes to `svc.audit`; gesture pastes are counted per app but their content is never logged. | M | 1 |
| AX05-F009 | Background change warning | If an app replaces the clipboard without a gesture (allowed by policy), a toast names it. Guards against address-swapping malware. | M | 2 |
| AX05-F010 | Clear on lock | Option: clear the current clipboard when the session locks; secrets are always cleared on lock. | M | 1 |
| AX05-F011 | Lazy transfer | Data stays with the source app until pasted or until the service takes a copy for history; large items are taken as passed descriptors, not copies in messages (PLATFORM section 7). | M | 1 |
| AX05-F012 | Survive source exit | The service keeps the current item so paste still works after the source app closes. | M | 1 |
| AX05-F013 | Provenance | Each item records source bundle id, time, and optional origin (a URL host from IN01, a file handle from SY02). Apps receiving a paste can ask for the provenance flags (not the origin) to warn the user, for example SY03 on multi-line pastes from the browser. | S | 2 |
| AX05-F014 | Secure surfaces | Nothing is pasted from history into the greeter, lock screen or prompts; the popup does not open there. | M | 1 |

### 4.2 History

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F015 | History list | Most recent first; default 200 items, 0–5 000; default age limit 7 days. | M | 1 |
| AX05-F016 | Popup | `Super+V` opens the popup at the caret (or the pointer); arrows move; `Enter` pastes; `Esc` closes. | M | 1 |
| AX05-F017 | Number keys | `1`–`9` paste the first nine visible items at once. | M | 1 |
| AX05-F018 | Search | Typing filters by content, source app name, type word ("image", "link", "colour") and tag; fuzzy with `forge::text::fuzzy_filter`; case and accent insensitive. | M | 1 |
| AX05-F019 | Filters | By type, by app, by date, pinned only, synced only. | S | 2 |
| AX05-F020 | Deduplicate | Copying the same content again moves the old item to the top instead of adding one. | M | 1 |
| AX05-F021 | Size limits | Per item 16 MiB by default (text 4 MiB); larger items are usable once but not kept; total store cap 512 MiB with oldest unpinned items removed first. | M | 1 |
| AX05-F022 | Preview | Text: first lines, with whitespace marks on demand. Image: thumbnail. Files: names and count. Colour: swatch and value. Link: host and cleaned form. | M | 1 |
| AX05-F023 | Full view | `Space` opens the whole item: text in a read-only view with wrap and monospace toggles, image at fit or 100%, file list with paths. | M | 2 |
| AX05-F024 | Delete | `Delete` removes an item with an Undo toast; multi-select in the manager. | M | 1 |
| AX05-F025 | Clear | Clear history (pins stay), clear everything, clear the last hour. `Super+Alt+Delete` while the popup is open clears clipboard and unpinned history. | M | 1 |
| AX05-F026 | Pause recording | A toggle stops history recording until turned back on, or for 5, 30, 60 minutes. The indicator shows it. | M | 1 |
| AX05-F027 | Persist | History is kept across restarts (default) or held in memory only. Stored encrypted with a key from `svc.keyring`. | M | 1 |
| AX05-F028 | Edit item | Edit a text item in place; the edit is a new item unless "replace" is ticked. | S | 2 |
| AX05-F029 | Merge | Select several text items and merge with a separator (newline, space, comma, custom). | S | 2 |
| AX05-F030 | Paste queue | Start a queue, copy several things, then each paste gives the next one in order (or reverse). The indicator shows "3 left". | S | 2 |
| AX05-F031 | Paste and keep position | After pasting an older item the clipboard returns to what it held before (setting). | S | 2 |
| AX05-F032 | Item actions | Context actions by type: open link (intent), show file in SY02, open image in ME01, copy colour in another format, search for text (SH01). Actions go through `svc.intent`; nothing runs commands. | S | 2 |
| AX05-F033 | Tags and notes | Free tags and a one-line note on any item. | C | 3 |
| AX05-F034 | Statistics | Counts per type and per source app, store size; local only. | C | 3 |

### 4.3 Pins and saved clips

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F035 | Pin | `Ctrl+P` pins an item: it never expires and shows in the pinned section. | M | 1 |
| AX05-F036 | Order and slots | Pinned items have an order; the first nine have quick slots `Super+Ctrl+1…9`. | M | 1 |
| AX05-F037 | Saved clips | Named clips of any type in folders; created from a history item or written by hand. These are the "snippets" of this app. | M | 2 |
| AX05-F038 | Folders | Nested one level; drag to reorder; a folder can be shown as a tab in the popup. | S | 2 |
| AX05-F039 | Placeholders in text clips | `{date}`, `{time}`, `{clipboard}` (the current clipboard text), `{cursor}`; same syntax as AX04. | S | 2 |
| AX05-F040 | Send to text expansion | "Make a typing shortcut…" hands a text clip to AX04's snippet editor with the text filled in. | S | 2 |
| AX05-F041 | Import and export | Saved clips as JSON (text and metadata inline, binary as base64) through the file portal. History is not exportable in bulk. | S | 2 |
| AX05-F042 | Per-clip sensitivity | A saved clip can be marked "mask in lists"; its preview shows dots until hovered or focused with reveal on hold. | S | 2 |
| AX05-F043 | Usage order | Sort saved clips by name, by last used, or by manual order. | C | 2 |

### 4.4 Content types

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F044 | Plain text | `text/plain;charset=utf-8`; other charsets decoded with `forge::text::decode`; invalid bytes replaced and flagged. | M | 1 |
| AX05-F045 | Rich text | `text/html` and the suite's rich-text type are kept beside the plain form. Preview is rendered as plain text plus a "formatted" badge; a restricted formatted preview needs G-10. HTML is never executed or fetched. | M | 2 |
| AX05-F046 | Markdown and code | `text/markdown` kept; code detected by the source app's hint; monospace preview. | S | 2 |
| AX05-F047 | Images | `image/png` kept as is; JPEG, WebP, BMP, TIFF, SVG-subset offers are decoded to a thumbnail by `svc.thumbnail` in its sandbox. The original bytes are kept for paste. | M | 2 |
| AX05-F048 | Files | File copies carry portal handles, not paths. History keeps the handle tokens; on paste the service checks they are still valid. Expired entries show greyed with the reason. | M | 2 |
| AX05-F049 | Cut files | A cut (move) offer is honoured once and is never kept in history as a move. | M | 2 |
| AX05-F050 | Colours | Detected from the suite's colour type and from text that is exactly a colour (`#RGB`, `#RRGGBB`, `#RRGGBBAA`, `rgb()`, `hsl()`); shown as a swatch; paste as hex, rgb, hsl, or float triple. | S | 2 |
| AX05-F051 | Links | Text that is one URL is typed "link"; shows host; offers cleaned form (4.6). | M | 2 |
| AX05-F052 | Tables | `text/csv` and tab-separated text previewed as a small grid; paste as CSV, TSV, or Markdown table. | C | 3 |
| AX05-F053 | App-private types | Unknown types are kept as opaque bytes only if the source app's policy says "record all types" and the item has an open format beside it (PLATFORM section 9); otherwise only the open formats are kept. | M | 2 |
| AX05-F054 | Multi-format items | One item holds all offered formats; the popup shows the best preview; "Paste as" lists the formats. | M | 1 |
| AX05-F055 | Metadata strip for images | Option: remove EXIF and other metadata from images on paste (re-encode in the thumbnail sandbox). | S | 3 |

### 4.5 Sensitivity

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F056 | Sensitivity flag | A source marks an offer with the platform type `application/x-pbsd-sensitive` carrying a level: `secret` or `private`. FUSE passes the type through (FUSE-WP-09). | M | 1 |
| AX05-F057 | Secrets never enter history | Items at level `secret` are never written to the history store, never shown in previews, never synced, never searchable. | M | 1 |
| AX05-F058 | Secret auto-clear | A secret is cleared from the clipboard after a time (default 30 s, 5–300 s) or after its first paste (setting; SE04 may ask for "one paste"). | M | 1 |
| AX05-F059 | Countdown | While a secret is on the clipboard the indicator shows a lock and a countdown; click clears now. | M | 1 |
| AX05-F060 | SE04 integration | Everything SE04 copies (passwords, TOTP codes, recovery codes, key passphrases) is marked `secret`. TOTP codes clear after 30 s or at code rollover. | M | 1 |
| AX05-F061 | Private level | Items at level `private` enter history masked, expire after 1 hour, and never sync. For addresses, account numbers and similar. | S | 2 |
| AX05-F062 | Detection rules | Unmarked text that matches a local rule is treated as `private` (or `secret` for key material): PEM private key blocks, OpenPGP secret key blocks, common token prefixes, card numbers passing the Luhn check, 12/18/24-word seed phrases, text copied from a password-purpose field. Rules are listed and each can be turned off. | M | 2 |
| AX05-F063 | User rules | The user can add rules: pattern (text or capped regex), action (ignore, private, secret, expire after N). | S | 2 |
| AX05-F064 | Per-app "never record" | Apps whose copies never enter history. Default list: SE04, SE05. Private windows of IN01 mark their copies `private`. | M | 1 |
| AX05-F065 | Mask in previews | `private` items show dots; reveal on hold; reveal is blocked while a screen capture or share is active (indicator event from SH02). | M | 2 |
| AX05-F066 | Mark by hand | "Mark as sensitive" on any item: removes it from sync, masks it, sets expiry. "Forget now" deletes it. | M | 1 |
| AX05-F067 | Secure delete | Deleted and expired items are removed from the store and their blobs overwritten; the store is on a copy-on-write file system, so the spec states the limit: snapshots may keep old blocks (section 13). History is excluded from snapshots and backups by default. | M | 1 |
| AX05-F068 | No capture | The popup and manager windows are marked `no-capture` while showing any unmasked `private` item. | S | 2 |
| AX05-F069 | Sensitive paste notice | Pasting a `secret` into an app other than the one focused when it was copied for is not blocked, but the notice (F007) is forced on. | S | 2 |

### 4.6 Transformations and paste-as

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F070 | Paste as plain text | `Super+Shift+V`: the top item, text only, no formatting. | M | 1 |
| AX05-F071 | Paste-as menu | `Ctrl+Enter` or `Tab` on an item: lists formats and transformations with a live preview of the first lines. | M | 2 |
| AX05-F072 | Case | Upper, lower, title, sentence case; locale-aware through `forge::text::upper`, `lower`, `title`. | S | 2 |
| AX05-F073 | Whitespace | Trim, collapse spaces, remove blank lines, join lines, split by separator, tabs to spaces. | S | 2 |
| AX05-F074 | Clean link | Remove tracking parameters from URLs (`utm_*`, `fbclid`, `gclid`, `mc_eid`, `igshid` and others from a bundled, editable list); unwrap known redirect wrappers using local rules only. No network request is ever made. | M | 2 |
| AX05-F075 | Auto clean links | Option: apply F074 to every copied link; the original stays available under "Paste as › Original". | S | 2 |
| AX05-F076 | Quotes and punctuation | Straight to typographic quotes and back; dashes; remove soft hyphens and zero-width characters. | C | 3 |
| AX05-F077 | Encode and decode | URL-encode, URL-decode, Base64, hex, HTML entities. | S | 3 |
| AX05-F078 | Structure | Pretty-print or minify JSON; sort lines; unique lines; number lines; quote as Markdown; wrap at N columns. | S | 3 |
| AX05-F079 | Text tools | To ASCII (`forge::text::to_ascii`), remove accents (`remove_marks`), slug (`slugify`), normalise to NFC. | C | 3 |
| AX05-F080 | Invisible character warning | Text with direction overrides, zero-width or homoglyph-prone invisible characters shows a mark in the preview; "Paste as › Visible characters only" strips them. | M | 2 |
| AX05-F081 | Image transforms | Paste as PNG or JPEG; scale to 50% or 25%; strip metadata (F055). Done in the thumbnail sandbox. | C | 3 |
| AX05-F082 | Rich to Markdown | Convert HTML clipboard content to Markdown text; own converter over a strict HTML subset parser in a helper. | C | 3 |
| AX05-F083 | Default per app | Per app: always paste plain text (for example in SY03), always clean links. | S | 2 |
| AX05-F084 | Custom transforms | User transforms as ForgeScript functions `string -> string`, run with no capabilities and a 100 ms limit; listed and enabled one by one. | C | 3 |
| AX05-F085 | Calculator and conversions | Not here: text selected for calculation is handed to OF07 by intent. | C | 3 |

### 4.7 Primary selection

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F086 | Primary selection support | Select-to-copy and middle-click paste work between apps through the primary-selection interface. The same read-on-gesture rule applies (the gesture is the middle click). | M | 1 |
| AX05-F087 | Not recorded | Primary selections are not kept in history by default: they change with every selection and often hold text the user never meant to copy. | M | 1 |
| AX05-F088 | Record on request | Option: record primary selections over N characters held for over 1 s; shown in a separate tab. | C | 3 |
| AX05-F089 | Turn off | Option: disable the primary selection system-wide (middle click then does nothing or pastes the clipboard). | S | 2 |
| AX05-F090 | Keep separate | No automatic copying between primary and clipboard; an explicit command "Copy selection to clipboard" exists. | M | 1 |
| AX05-F091 | Sensitive fields | Selections in password-purpose fields never become the primary selection (toolkit rule, FUSE-R-102). | M | 1 |

### 4.8 Sync between own devices (optional)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F092 | Off by default | No network capability is used until the user turns sync on; then `net.user` is requested through the normal prompt. | M | 3 |
| AX05-F093 | Pairing | Devices pair once by a QR code or a short code shown on one and typed on the other; long-term keys go to `svc.keyring`. Uses the suite's device-sync engine. | M | 3 |
| AX05-F094 | Local network only | Discovery and transfer on the local link only; no relay, no cloud, no account. If the devices cannot reach each other, nothing is sent. | M | 3 |
| AX05-F095 | End-to-end encryption | Each item is encrypted and authenticated for the paired device before it leaves; forward secrecy from an ephemeral exchange per session. Primitives through SE04's engine and FORGE crypto (XChaCha20-Poly1305, X25519 where present; FORGE-WP-22). | M | 3 |
| AX05-F096 | Modes | Manual ("Send to ▸ device" per item) or automatic (every new non-sensitive item up to a size limit, default 1 MiB). | M | 3 |
| AX05-F097 | What never syncs | `secret` and `private` items, items from never-record apps, files (only text, links, colours and images sync). | M | 3 |
| AX05-F098 | Receiving | A received item appears in history marked with the device name; it becomes the current clipboard only if "use received items at once" is on. | M | 3 |
| AX05-F099 | Device list | Paired devices with last seen time, key fingerprint, per-device send and receive toggles, and Unpair (revokes the key at once). | M | 3 |
| AX05-F100 | Route rule | Sync sockets use the `clear` route on the local link only; never over VPN or Tor routes (they are not local). | M | 3 |
| AX05-F101 | Indicator | The header shows `[net]` while a sync socket is open, listing the peer address (DESIGN_SYSTEM section 3). | M | 3 |

### 4.9 Paste delivery and shortcuts

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F102 | Direct paste of text | Choosing a text item commits it into the focused field through AX04's panel connection: no fake keys, and it works with any layout. | M | 1 |
| AX05-F103 | Paste of other types | The service makes the item the current clipboard and asks `svc.a11y` to run the Paste action of the focused element; the app then reads it under the gesture rule (the user's choice in the popup counts as the gesture, vouched for by the shell). | M | 2 |
| AX05-F104 | Fallback | If neither path is possible the item is set as the clipboard and a toast says "Ready to paste". | M | 1 |
| AX05-F105 | Global shortcuts | `Super+V` popup, `Super+Shift+V` plain paste, `Super+Ctrl+1…9` pinned slots, `Super+Alt+V` manager, `Super+Ctrl+V` paste queue next. | M | 1 |
| AX05-F106 | Popup placement | At the caret rectangle from the text-input state or `svc.a11y:track`; else at the pointer; else centre. | M | 1 |
| AX05-F107 | Return focus | The popup never takes focus away permanently: on close the previous window and field have focus again within one frame. | M | 1 |
| AX05-F108 | On-screen keyboard key | AX03's clipboard key opens the popup in touch density. | S | 2 |
| AX05-F109 | Voice | AX06 commands "paste", "paste number three", "show clipboard" call this app's actions. | C | 3 |
| AX05-F110 | Image annotate hand-off | "Annotate" on an image item emits `edit` to ME05 with the image handle; ME05's result returns as a new clipboard item. | S | 2 |
| AX05-F111 | Copy from screen text | "Copy text from image" on an image item hands off to ME05's OCR; result returns as text. | C | 3 |

### 4.10 Manager, settings, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX05-F112 | Manager window | Three panes: sections, list, detail (section 5.2). | M | 1 |
| AX05-F113 | Apps page | Table of apps with their clipboard policy (F006), last read and counts; edit in place; link to AD05. | M | 1 |
| AX05-F114 | Access log page | Reads without gesture, denials, background changes, policy edits, from `svc.audit`; filter by app and date. | M | 2 |
| AX05-F115 | Rules page | Sensitivity rules and transformation defaults. | M | 2 |
| AX05-F116 | Indicator | SH02 panel icon: paused, secret countdown, queue count, sync state; menu with recent five items and Clear. | M | 1 |
| AX05-F117 | Crash recovery | The service restarts within 500 ms; the current clipboard is restored from its protected memory file descriptor; history is journaled. | M | 1 |
| AX05-F118 | Store repair | A damaged store is moved aside and a new one started; pins are recovered where readable; the user is told. | S | 2 |
| AX05-F119 | Managed policy | A system policy file can force settings (history off, sync off, secret timeout) for shared machines; forced keys show a lock. | S | 3 |
| AX05-F120 | First-run sheet | Explains the paste rule, the secret countdown and the three shortcuts. | S | 2 |
| AX05-F121 | No telemetry | Nothing about clipboard use leaves the machine except user-enabled sync to paired devices. | M | 1 |
