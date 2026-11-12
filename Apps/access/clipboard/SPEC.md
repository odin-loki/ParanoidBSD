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

## 5. GUI

### 5.1 Window map

```
Clipboard (service runs in the background; indicator in SH02)
├─ History popup (Super+V)
│   ├─ tabs: History · Pinned · <clip folders>
│   ├─ Paste-as menu (Tab / Ctrl+Enter)
│   ├─ Item context menu
│   └─ Full view (Space)
├─ Manager window (Super+Alt+V; main window)
│   ├─ History · Pinned · Saved clips ▸ folders · Apps · Access log · Rules · Devices · Settings
│   └─ dialogs: Edit clip · New saved clip · Merge items · Add rule · Pair device
│               · Import/Export · Clear…
├─ Indicator menu (in SH02)
├─ Toasts: Ready to paste · Pasted from … · Clipboard changed by … · Secret cleared
├─ First-run sheet
└─ Paste prompt (drawn by SH04; content defined here)
```

### 5.2 Main window (Manager)

Shown at 1000×640. Three-pane master-detail.

```
+------------------------------------------------------------------------------------------+
| [=] Clipboard — History            [🔍 search Ctrl+F]        [⏸ Pause] [net] [▁][▢][✕]  | (1)
+----------------+---------------------------------------------+---------------------------+
| > History  142 | Today                                       | Text · 3 lines · 96 chars | (4)
|   Pinned     6 | > 14:02 Text Editor   Kind regards, Dana…   | From  Text Editor         |
|   Saved clips  |   13:58 Web Browser   🔗 example.org/pricing| Time  today 14:02         |
|    ▸ Replies   |   13:40 Screen Capt.  🖼 1280×720 PNG       | Formats text/plain,       |
|    ▸ Addresses |   13:31 Files         📄 3 files            |         text/html         |
|   ───────────  |   13:02 Draw          ■ #4C8DFF             | Sensitivity none          |
|   Apps         |   12:47 Passwords     ●●●●●● private · 13 m | +-----------------------+ |
|   Access log   | Yesterday                                   | | Kind regards,         | |
|   Rules        |   17:20 Terminal      git rebase -i HEAD~3  | | Dana                  | |
|   Devices      |   16:05 Web Browser   The quick brown fox… | | Support               | |
|   Settings     |                                             | +-----------------------+ |
|                |                                         (3) | [Paste] [Paste as v] [Pin]|
| (2)            |                                             | [Edit] [Mark sensitive]   |
|                |                                             | [Send to v] [Delete]      |
+----------------+---------------------------------------------+---------------------------+
| 142 items · 3.1 MB · recording · secrets clear after 30 s          sync: off             | (5)
+------------------------------------------------------------------------------------------+
```

1. Header bar: search `lineEdit`, pause `toolButton`. The `[net]` badge appears only while a sync socket is open.
2. Sidebar, 220 px, `treeView` (sections and clip folders) with counts.
3. Item list, `listView` over the store model, grouped by day; each row: time, source app, type icon with a word in the a11y name, one-line preview. Multi-select with Shift and Ctrl. Private rows are masked with their remaining lifetime.
4. Inspector, 280 px (`F4`): metadata `propertyGrid`, preview (`codeEdit` read-only for text, `image` for images, `listView` for files, colour `panel`), action `button`s.
5. Status bar.

States: empty ("Nothing copied yet. Copy something, then press Super+V."); loading (store unlock from the keyring: list shows a `progressBar` row, under 200 ms expected); error (store damaged: banner with `[Repair]`, F118); locked (window hidden; store key dropped from memory if "lock store with session" is on); read-only (managed policy: forced settings show a lock; history pane works); offline (no effect; sync section says "No paired device reachable"); paused (banner "Recording is paused" with `[Resume]`); narrow (under 640 px the inspector becomes a sheet and the sidebar an overlay).

### 5.3 Other views and dialogs

**History popup** (420×520, at the caret; touch density uses 44 px rows).

```
+--------------------------------------------------+
| [🔍 type to search_______________]   [⏸] [⚙]   | (1)
| [History] [Pinned] [Replies] [Addresses]         | (2)
+--------------------------------------------------+
| 1  Kind regards, Dana…                Text Ed.   | (3)
| 2  🔗 example.org/pricing             Browser    |
| 3  🖼 Screenshot 1280×720             Capture    |
| 4  📄 report.odt and 2 more           Files      |
| 5  ■ #4C8DFF                          Draw       |
| 6  ●●●●●●●● (private, 13 min left)    Passwords  |
| 7  git rebase -i HEAD~3               Terminal   |
+--------------------------------------------------+
| Enter paste · Tab paste as · Ctrl+P pin · Del    | (4)
+--------------------------------------------------+
```

1. Search `lineEdit` has focus; typing filters; pause and settings `toolButton`s. 2. `tabBar`. 3. `listView`; digits paste; rows show source app short name. 4. Hint `label`. States: empty; secret on clipboard (a top row "🔒 Secret on the clipboard · clears in 22 s [Clear now]" that cannot be opened or pinned); queue active (header "Queue: 3 left [Stop]").

**Paste-as menu** (popup beside the item).

```
+----------------------------------------------+
| Paste as                                     |
| > Plain text                 Super+Shift+V   |
|   Formatted (HTML)                           |
|   Markdown                                   |
|   ─────────────                              |
|   Clean link     example.org/pricing         | (1)
|   UPPER CASE · lower case · Title Case       |
|   Trim and join lines                        |
|   Visible characters only                    |
|   More ▸                                     |
+----------------------------------------------+
| Preview: example.org/pricing                 | (2)
+----------------------------------------------+
```

1. Items depend on the type; each shows a short result. 2. Live preview of the highlighted transform (first 3 lines). `popupMenu` + `label`.

**Paste prompt** (drawn by SH04; this spec fixes its content).

```
+--------------------------------------------------------------+
| ⚠ Security prompt from the system                            |
| Notes (signed: system ✓) wants to read the clipboard         |
| without you pasting.                                         |
|                                                              |
| The clipboard holds: text, copied from Web Browser at 14:02. | (1)
|                                                              |
| [Deny]  [Allow once]  [Allow this session]  [Always allow]   | (2)
| [ ] Do not ask again for Notes (deny)                        |
+--------------------------------------------------------------+
```

1. Kind and source only; never the content. If the clipboard holds a secret the line reads "a secret from Passwords" and "Always allow" is not offered. 2. Default button is Deny.

**Apps page** (manager content).

```
| App              Read without paste   Copy    Record copies     Last background read      |
| > Notes          [Ask        v]       [x]     [Yes          v]  never                     |
|   Terminal       [Deny       v]       [x]     [Yes          v]  never                     |
|   Remote Desktop [Allow      v] ⚠     [x]     [Expire 1 h   v]  today 13:10 (12 today)    |
|   Passwords      [Deny       v]       [x]     [Never        v]  never                     |
|   Web Browser    [Ask        v]       [x]     [Yes          v]  denied today 11:02        |
|  [Reset to defaults]  [Show in Permissions…]                                              |
|  Every app can always read the clipboard when you paste into it.                          |
```

`tableView` with `comboBox` and `checkBox` cells. The ⚠ marks any app with "Allow"; its tooltip explains.

**Devices page** (sync).

```
| Sync between my devices [OFF]     Uses the local network only. End-to-end encrypted.      |
|  Device        Last seen     Send   Receive   Key fingerprint                             |
| > desk-pc      today 14:00   [x]    [x]       3F9A 22C1 0B7E 5D40                          |
|   tablet       3 days ago    [ ]    [x]       91AC 0E55 7F12 C3B8                          |
|  [Pair a device…]  [Unpair]                                                               |
|  Mode (•) manual: “Send to” on an item   ( ) automatic for items up to [ 1 MiB ]          |
|  Never synced: secrets, private items, files, apps on the never-record list.              |
```

**Pair device** (dialog): a QR code `image` and an 8-digit code with a 2-minute `progressBar`; the other device shows the fingerprint; both sides confirm with `[They match]`.

**Rules page**.

```
| Sensitivity rules                                                Action        On         |
|   PEM / OpenPGP private key blocks                               secret        [x]        |
|   Card numbers (Luhn check)                                      private       [x]        |
|   Seed phrases (12, 18, 24 words from the word list)             secret        [x]        |
|   Common access-token prefixes                                   private       [x]        |
|   Copied from a password field                                   secret        [x]        |
|   User: ^INV-[0-9]{6}$                                           ignore        [x]        |
|  [Add rule…] [Test text against rules…]                                                   |
| Link cleaning   [x] Clean copied links automatically    [Edit parameter list…]            |
```

**Edit clip** (dialog, 520×400): name `lineEdit`, folder `comboBox`, text `codeEdit` (Plain), placeholder buttons, `[x] Mask in lists`, `[Make a typing shortcut…]`, `[Cancel] [Save clip]`.

### 5.4 Menus

```
Manager [=] (F10)
├─ File ▸ New saved clip Ctrl+N · Import clips… · Export clips… · Close Ctrl+W
├─ Edit ▸ Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy to clipboard Ctrl+C · Edit item F2
│        · Merge selected Ctrl+M · Delete Delete · Select all Ctrl+A · Find Ctrl+F
├─ Item ▸ Paste Enter · Paste as ▸ … · Pin Ctrl+P · Mark sensitive Ctrl+Shift+S
│        · Send to ▸ <devices> · Annotate (Screen Capture) · Open with ▸ …
├─ History ▸ Pause recording Ctrl+Shift+P · Start paste queue Ctrl+Q
│           · Clear last hour · Clear history… · Clear everything… Super+Alt+Delete
├─ View ▸ Sidebar F9 · Inspector F4 · Show whitespace · Group by day/app/type
└─ Help ▸ Clipboard help F1 · How pasting works here · About

Indicator menu (SH02)
├─ <five most recent items>
├─ Clear secret now (when one is held)
├─ Pause recording ▸ 5 min · 30 min · 1 h · until resumed
├─ Paste queue ▸ Start · Stop
├─ Clear clipboard and history
└─ Open Clipboard…                         Super+Alt+V

Item context menu
├─ Paste · Paste as ▸ · Copy to clipboard
├─ Pin / Unpin · Save as clip… · Edit…
├─ Mark sensitive · Forget now
├─ Send to ▸ <devices>
├─ <type actions: Open link · Show in Files · Annotate · Copy as rgb()…>
└─ Delete
```

### 5.5 Keyboard and pointer

| Keys | Action | Keys | Action |
|---|---|---|---|
| `Super+V` | history popup | `Super+Shift+V` | paste top item as plain text |
| `Super+Ctrl+1…9` | paste pinned slot | `Super+Alt+V` | manager window |
| `Super+Ctrl+V` | next item of the paste queue | `Super+Alt+Delete` (popup open) | clear clipboard and history |
| In popup: `Enter` | paste | `1`–`9` | paste nth |
| `Tab` / `Ctrl+Enter` | paste-as menu | `Space` | full view |
| `Ctrl+P` | pin | `Delete` | delete |
| `Ctrl+Q` | start or stop queue | `Ctrl+Tab` | next tab |
| `Ctrl+E` | edit | `Ctrl+Shift+S` | mark sensitive |

Global chords are registered with the shell and arrive as actions. Pointer: click pastes (popup) or selects (manager); middle click on a row pastes as plain text; right click opens the item menu. Drag and drop: drag an item out of the manager into any app as a normal typed drag (the drop target receives it under the drop rule, not the clipboard rule); drag history items onto Pinned or a folder; drop text, an image or files on Saved clips to create a clip. Touch: swipe a row left to delete, right to pin.

### 5.6 States

| State | Behaviour |
|---|---|
| Recording | Normal. |
| Paused | No new history; current clipboard still works; indicator shows pause. |
| Secret held | Countdown; popup shows the locked row; history untouched. |
| Session locked | Secrets cleared; popup and manager closed; optional full clear (F010). |
| Store locked (keyring locked) | History unavailable, current clipboard works; banner "Unlock to see history". |
| History off (setting or policy) | Popup shows pinned and saved clips only. |
| Service restarting | Pastes of the current item still work through the compositor's own selection; popup shows "Starting…" for under 500 ms. |
| No text-input and no Paste action | Fallback toast (F104). |
| Sync on, peer unreachable | Items queue in memory for 10 minutes for manual sends only, then are dropped; nothing is stored for later. |
| Secure surface focused | Popup refused (F014). |

### 5.7 Widgets

| Region | Widget | Note |
|---|---|---|
| Sidebar | `treeView` | in tree |
| Item list, popup list | `listView` / `modelListView` over `SqliteModel` (PLATFORM V8) | variable row height for images: FUSE-WP-28 |
| Inspector | `propertyGrid`, `codeEdit` (read-only), `image`, `listView`, `panel` | in tree |
| Apps, log, rules, devices | `tableView` with in-cell `comboBox`, `checkBox` | in-cell editors: FUSE-WP-28 |
| Paste-as menu, item menu | `popupMenu` | in tree |
| Popup tabs | `tabBar` | in tree |
| Pair dialog | `dialog`, `image` (QR drawn by own code into a bitmap), `progressBar` | in tree |
| Formatted preview of rich text | — | G-10; plain preview until then |
| Toasts | `ui::toast` | FUSE-WP-30 |

Gaps:

| ID | Gap | Evidence | Lands in |
|---|---|---|---|
| AX05-GAP-1 | Compositor rule that binds the data-control interface to one authorised service, and enforces "receive needs a paste-gesture serial" | `data_control.cppm` names `zwlr_data_control_manager_v1` version 2 with `GetDataDevice` and `GetPrimarySelectionDevice`; `data_device.cppm` and `primary_selection.cppm` are opcode tables; `security_context.cppm` names `wp_security_context_manager_v1` (`PBSD:pbsd/compositor/`). No policy code read. | compositor |
| AX05-GAP-2 | Clipboard in FUSE is UTF-8 text only (hooks `set`/`get`), with Win32 and X11 backends | `FUSE:Source/FUSE/UI/Core/include/fuse/ui/ui.hpp:66-72`, `FUSE:Source/FUSE/Core/include/fuse/platform/text_services.hpp` | FUSE-WP-09 |
| AX05-GAP-3 | FORGE's clipboard wrapper shells out to `wl-copy`/`wl-paste` or `xclip`/`xsel` on FreeBSD; unusable in capability mode | `FORGE:docs/os-wrapper.md` (clipboard) | FORGE-WP-24.1 (`svc.clipboard` client) |
| AX05-GAP-4 | Platform MIME types for sensitivity level, provenance and portal file handles | FUSE-WP-09 names them as "defined by the platform plan" | PLATFORM (G-09) |
| AX05-GAP-5 | A network capability limited to the local link for device sync; PLATFORM section 6 has only `net.host` and `net.user` | PLATFORM section 6 | PLATFORM |
| AX05-GAP-6 | A `Paste` action in the a11y action set; today it has Focus, Click, Increment, Decrement, SetValue | `FUSE:Source/FUSE/UI/A11y/include/fuse/ui/a11y/a11y.hpp:19` | FUSE-WP-55, FUSE-WP-56 |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Clipboard service (`svc.clipboard`) | C++23 | `clipboard-service` helper, capability mode, autostart | Sole data-control client; holds the current item; applies sensitivity and per-app policy; owns the history store; answers the compositor's "may this client read?" queries; timers for secrets. Native because it holds secrets and parses untrusted offers (PLATFORM section 2 (a), (c)) |
| Store | SQLite + blob files, encrypted | inside `clipboard-service` | History, pins, saved clips |
| App logic | ForgeScript | app host | Popup, manager, view models, transforms on text, rule editor |
| UI | FUSE UI | app host | All surfaces |
| HTML-subset helper | C++23 | `html-helper`, capability mode, no handles | Parses clipboard HTML for plain-text extraction and Markdown conversion (untrusted bytes) |
| Sync helper | C++23 | `sync-helper`, capability mode with only its sockets; started only when sync is on | Local discovery, encrypted transfer; the only process with a network capability |
| Thumbnails | platform | `svc.thumbnail` sandboxes | Image decode and re-encode |

```
 app A ── copy (serial) ─▶ compositor ── data-control ─▶ clipboard-service ──▶ store (encrypted)
 app B ◀─ paste (serial checked; policy asked) ─ compositor ◀── offer ──┘          ▲
                                                     │                             │ history API
                         SH04 paste prompt ◀─────────┘ (no gesture)                │ (svc.clipboard:manage)
 app host: popup / manager (ForgeScript + FUSE UI) ────────────────────────────────┘
      │ text paste ─▶ AX04 panel commit ─▶ focused field
      │ other paste ─▶ service sets selection ─▶ svc.a11y act:paste ─▶ focused element
      └ transforms on text in script; HTML ─▶ html-helper; images ─▶ svc.thumbnail
 clipboard-service ◀──▶ sync-helper ◀── E2E encrypted, local link ──▶ paired device
```

The UI process never holds the data-control interface. It talks to the service with the right `svc.clipboard:manage`, which only this bundle may hold. Ordinary apps hold plain `svc.clipboard` (copy, paste under the gesture rule, mark sensitivity).

### 6.2 Data model

Store: `~/.local/share/org.pbsd.clipboard/clips.db` (SQLite, page-level encryption with XChaCha20-Poly1305, key in `svc.keyring`) and `blobs/<blake3>` for items over 64 KiB (each encrypted with a per-blob key wrapped by the store key). `schema_version` table; forward-only migration with a copy kept. The store directory is tagged "no snapshot, no backup" by default through `svc.backup` rules.

| Entity | Fields |
|---|---|
| `item` | `id: i64`, `created: i64`, `last_used: i64`, `use_count: u32`, `source_bundle: string`, `origin_kind: enum{none, url_host, file, device}`, `origin: string?`, `kind: enum{text, rich, image, files, colour, link, table, other}`, `sensitivity: enum{none, private}`, `expires: i64?`, `pinned: bool`, `pin_order: u16?`, `content_hash: blake3`, `preview: string` (at most 256 graphemes; empty when private), `size: u64`, `device_id: uuid?`, `note: string?` |
| `format` | `item_id: i64`, `mime: string`, `inline: bytes?`, `blob: blake3?`, `size: u64` |
| `file_ref` | `item_id: i64`, `handle_token: bytes32`, `name: string`, `is_dir: bool`, `size: u64?` |
| `tag` | `item_id: i64`, `tag: string` |
| `clip` (saved) | `id: uuid`, `name: string`, `folder_id: uuid?`, `item_id: i64`, `masked: bool`, `order: u32` |
| `folder` | `id: uuid`, `name: string`, `in_popup: bool`, `order: u32` |
| `app_policy` | `bundle_id: string`, `read_no_gesture: enum{ask, allow, deny}`, `copy: bool`, `record: enum{yes, never, expire}`, `expire_s: u32?`, `always_plain: bool`, `clean_links: bool` |
| `rule` | `id: i64`, `builtin: string?`, `pattern: string?`, `is_regex: bool`, `action: enum{ignore, private, secret, expire}`, `expire_s: u32?`, `enabled: bool` |
| `device` | `id: uuid`, `name: string`, `public_key: bytes32`, `paired: i64`, `last_seen: i64`, `send: bool`, `receive: bool` |
| `current` (memory only) | `offer_types: string[]`, `source_bundle: string`, `sensitivity: enum{none, private, secret}`, `set_time: i64`, `clear_at: i64?`, `one_paste: bool`, `data_fd: sealed memory descriptor` |

Secrets exist only in `current`, in locked memory, and are zeroed on clear. Limits: 64 formats per item; MIME strings at most 255 bytes; preview built from the plain form after rule checks.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Data-control, data-device, primary-selection (Wayland) | yes | yes | own client code (FUSE-WP-09.6 restricted client) | clipboard service |
| `svc.clipboard` interface (apps) and `:manage` interface (this UI) | yes | yes | schema compiler output (G-08) | service; app host |
| `text/plain` in UTF-8 and legacy charsets | yes | yes | `forge::text` decode, validate, sanitise (Verified, `FORGE:docs/text-locale.md`) | service (validation), app host (transforms) |
| `text/html` | yes (subset) | no | own strict tokenizer; no scripts, no fetch | `html-helper` |
| `text/markdown`, `text/csv`, TSV | yes | yes | own code | app host (ForgeScript) |
| `image/png` and other image types | thumbnails only | PNG, JPEG on transform | suite image codecs (owner ME01) | `svc.thumbnail` sandbox |
| URL parsing for link cleaning | yes | yes | own URL parser (WHATWG rules subset) | app host; T2 target because input is untrusted |
| Saved clip export (JSON) | yes | yes | `forge::json` | app host |
| Sync wire format (framed records, XChaCha20-Poly1305) | yes | yes | sync engine (owner OF02); primitives per FORGE-WP-22 | `sync-helper` |
| QR code for pairing | no | yes | own encoder | app host |
| User transform scripts | yes | no | `forge::script::engine`, `Caps` none | child engine in the app host |

### 6.4 Integration

- Intents: handles `view` and `pick` for "choose a clip"; emits `open` (links, files), `edit` (image to ME05), `search`.
- Services provided: `svc.clipboard` (for all apps), `svc.clipboard:manage` (this bundle only), actions `clipboard.popup`, `clipboard.paste_plain`, `clipboard.paste_slot(n)` for SH01, AX03, AX06.
- Services used: `svc.keyring` (store key, device keys), `svc.settings`, `svc.notify`, `svc.audit`, `svc.thumbnail`, `svc.intent`, `svc.portal.file`, `svc.backup` (exclusion rule), `svc.a11y` (`track`, `act.paste`), AX04's `input-method.panel`.
- Other apps: SE04 (marks secrets; `one_paste` requests), SH04 (paste prompt), SH02 (indicator, capture-active events), AD05 (policy rows and revocation), AD04 (reads the audit entries), SY02 (file handles), IN01 (origin host, private windows), SY03 (provenance flags for paste warnings), ME05 (annotate, OCR), AX04 (panel commit, snippet hand-off), AX03 (clipboard key), AX06 (voice actions), OF02 (sync engine), VM01 and VM04 (subject to per-app policy).
- Verified in repos: FUSE's clipboard is a pair of hooks for UTF-8 text with a context-local default (`FUSE:Source/FUSE/UI/Core/include/fuse/ui/ui.hpp:66-72`), and platform functions `setClipboardText` and `clipboardText` for Win32 and X11 (`FUSE:Source/FUSE/Core/include/fuse/platform/text_services.hpp`); X11 INCR transfers are unsupported there. FORGE has `forge::os::clipboard` `available`, `get`, `set`, implemented on FreeBSD by running `wl-copy`/`wl-paste` or `xclip`/`xsel` (`FORGE:docs/os-wrapper.md`). FORGE's crypto today is Ed25519, XChaCha20-Poly1305, BLAKE2b and Argon2id through Monocypher (as recorded in FORGE_BUILD_PLAN, FORGE-WP-22). The compositor tree has `data_control`, `data_device`, `primary_selection`, `abstract_data_source` and `security_context` modules (`PBSD:pbsd/compositor/`); those read are opcode tables. FUSE_BUILD_PLAN FUSE-WP-09 already plans typed offers, primary selection and a restricted data-control client for this app. Everything else is Proposed.

## 7. Security and privacy

### 7.1 Trust model

On most desktops any running program can read the clipboard at any time, and a clipboard manager makes that worse by keeping everything. Here:

1. **Apps have no ambient clipboard access.** The compositor delivers clipboard bytes to an app only for a paste gesture in that app (F003). Anything else is a prompt (F005) and an audit entry.
2. **One service sees everything, and it is small.** `clipboard-service` is the single holder of the data-control interface. It is system-signed, has `net.none`, runs in capability mode with its store directory and its bus endpoint, and is in the trusted base for clipboard content. It is a T2 target with T1 targets on its policy core.
3. **The UI is less trusted than the service.** The popup and manager hold `svc.clipboard:manage`: they can list, read, pin, delete and select history items, and nothing else. They never see items at level `secret`: the service does not return them.
4. **Privileged reach into other apps is narrow and goes through `svc.a11y`.** This app needs two things from other apps' UI: where the caret is (`svc.a11y:track`, geometry only) and to trigger Paste in the focused element (`svc.a11y:act.paste`, one action, refused on secure surfaces). It has no `tree.read`, `text.read`, general `act` or `input.synth`. Text pastes avoid even that by using AX04's panel commit.
5. **The network is a separate process and off by default.** Only `sync-helper` can open sockets, only after the user enables sync and grants the prompt, and it receives only items the service has already encrypted for a paired device.

All rights are granted through SH04 prompts, shown and revocable in AD05, and logged in `svc.audit`.

### 7.2 Capabilities requested

| Capability | Why |
|---|---|
| `fs.app` | Encrypted store |
| `fs.portal` | Import and export of saved clips |
| `net.none` by default; `net.user` only for `sync-helper` when sync is on | Device sync on the local link; each first contact is logged (PLATFORM section 6). A link-local capability would be narrower (AX05-GAP-5) |
| `net.route:clear` | Sync never uses VPN or Tor routes |
| `proc.helper:clipboard-service`, `proc.helper:html-helper`, `proc.helper:sync-helper` | Service, HTML parsing, sync |
| `svc.clipboard`, `svc.clipboard:manage` | History access for the UI |
| `svc.a11y` (`track`, `act.paste`) | Popup placement; paste of non-text items |
| `svc.input-method` | Direct paste of text through AX04's panel |
| `svc.keyring` | Store key; device keys |
| `svc.settings`, `svc.notify`, `svc.audit`, `svc.intent`, `svc.thumbnail`, `svc.backup` | Settings, toasts, access log, item actions, image previews, exclusion rule |
| `bg.run`, `bg.autostart` | The service must run for the whole session |

### 7.3 Threat model

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Clipboard content | Any app in the background | Poll the clipboard for passwords, addresses, tokens | Read on paste only (F003); prompt (F005); audit (F008) |
| Clipboard content | Focused malicious app | Read as soon as it gets focus | Focus alone is not a gesture; needs paste serial |
| The user's paste | Background app | Replace a copied bank or wallet address | Copy needs a gesture (F002); background change warning (F009) |
| Secrets | The history itself | Passwords kept on disk for days | Secrets never stored (F057); auto-clear (F058); detection rules (F062); never-record apps (F064) |
| History store | Offline attacker, other local user | Read the store files | Encrypted store, key in keyring, home encryption; excluded from backups and snapshots by default |
| History store | Compromised UI process | Dump history | Possible for non-secret items: accepted; UI has `net.none` and no file access outside `fs.app` and portal picks; export of bulk history does not exist (F041) |
| The user | Malicious web page or document | Copy text with hidden characters or a hidden command; "pastejacking" | Invisible character mark (F080); provenance flags let SY03 warn; full view shows whitespace |
| Service | Hostile source app | Offer 10 GiB, never finish, send malformed types | Size caps (F021); timeouts; non-blocking transfers; fuzzing (FUSE-WP-09 gate `fuse_app_wl_offer_hostile` covers the client side) |
| Service | Hostile HTML or image | Parser exploit | `html-helper` and `svc.thumbnail` sandboxes with no handles |
| Other apps | This app | Use `act.paste` to paste into the wrong place | Only on the element focused when the popup opened; cancelled if focus changed |
| Prompts | Any app | Trigger many paste prompts to wear the user down | One prompt per app per minute; "Deny always" one click; default is Deny |
| Sync | Network attacker on the LAN | Read or inject clips | End-to-end encryption with paired keys; unknown peers ignored; fingerprint check at pairing |
| Sync | Stolen paired device | Receives future clips | Unpair revokes at once; secrets and private items never sync |
| Screen | Recorder or onlooker | Read masked items when revealed | Reveal blocked during capture (F065); `no-capture` windows (F068) |
| User transforms | Malicious script | Exfiltrate text | No capabilities, time limit, output only replaces the pasted text (F084) |

Stored: non-secret history (encrypted), pins, saved clips, policy, rules, device keys (in the keyring). Not stored: secrets, primary selections (by default), content of prompts. Leaves the machine: nothing, unless sync is on, and then only non-sensitive items to paired devices on the local link.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `history.enabled` | bool | true | Record history |
| `history.max_items` | u16 | 200 | 0–5 000 |
| `history.max_age_days` | u16 | 7 | 0 = no age limit |
| `history.persist` | bool | true | Keep across restarts |
| `history.item_max_bytes` | u32 | 16 MiB | Larger items are not kept |
| `history.store_max_bytes` | u64 | 512 MiB | |
| `history.dedupe` | bool | true | |
| `history.restore_after_paste` | bool | false | F031 |
| `history.clear_on_lock` | bool | false | Secrets are always cleared on lock |
| `history.lock_store_with_session` | bool | true | Drop the store key on lock |
| `secret.timeout_s` | u16 | 30 | 5–300 |
| `secret.one_paste` | bool | false | Clear after first paste |
| `private.expire_s` | u32 | 3600 | |
| `rules.builtin.<name>` | bool | true | Detection rules |
| `apps.never_record` | string[] | SE04, SE05 bundle ids | |
| `policy.default_read_no_gesture` | enum{ask, deny} | `ask` | Default for new apps |
| `notice.cross_app_paste` | bool | false | F007 |
| `notice.background_change` | bool | true | F009 |
| `links.auto_clean` | bool | false | F075 |
| `links.parameters` | string[] | bundled list | Tracking parameter names and patterns |
| `paste.plain_apps` | string[] | SY03 bundle id | Always paste plain text |
| `primary.enabled` | bool | true | Primary selection |
| `primary.record` | bool | false | F088 |
| `popup.position` | enum{caret, pointer, centre} | `caret` | |
| `popup.rows` | u8 | 9 | Visible rows |
| `popup.tabs` | uuid[] | — | Folders shown as tabs |
| `sync.enabled` | bool | false | |
| `sync.mode` | enum{manual, auto} | `manual` | |
| `sync.auto_max_bytes` | u32 | 1 MiB | |
| `sync.use_received` | bool | false | Received item becomes current |
| `images.strip_metadata_on_paste` | bool | false | F055 |
| `backup.include_store` | bool | false | |

## 9. Accessibility and languages

- The popup is a list with position and type in each row's name: "Text, Kind regards Dana, from Text Editor, 2 of 9". Private rows are named "private item from Passwords, hidden".
- Every action in the popup has a key; the popup works at 300% scale by showing fewer rows, never by shrinking text.
- The secret countdown is announced once at copy ("secret copied, clears in 30 seconds") and at clear, through `svc.a11y:announce`; not every second.
- Search is accent and case insensitive and uses grapheme-aware matching (`forge::text`).
- Previews of right-to-left and mixed text use the paragraph's base direction (`forge::text::direction`); the list mirrors in RTL locales.
- Case transforms are locale-aware (Turkish dotted and dotless i handled by `forge::text` case tailoring, Verified in `FORGE:docs/text-locale.md`).
- Touch: 44 px rows, swipe actions have menu equivalents.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| `Super+V` to popup first frame | under 80 ms |
| Search over 5 000 items per keystroke | under 10 ms |
| Copy to item visible in history (text under 64 KiB) | under 20 ms, never blocking the source app |
| Paste of a text item through the panel | under 10 ms after `Enter` |
| Policy check on a paste (compositor to service and back) | under 1 ms |
| Service memory, 200 text items | under 20 MB |
| Largest item kept | 16 MiB by default; setting up to 256 MiB |
| Service idle | zero wake-ups (timers only while a secret or expiry is pending) |
| Store open (unlock, 5 000 items) | under 200 ms |
| Thumbnail for a 4K PNG | under 150 ms in `svc.thumbnail` |

## 11. Testing and proof

Nothing has run. All of this is plan.

- Unit (ForgeScript): every text transform with Unicode edge cases; link cleaning over a table of real-world URL shapes; colour parsing and formatting; dedupe; expiry arithmetic; queue order; rule matching.
- Unit (C++): policy decision function; secret timers; store encryption round trip; size caps; eviction order.
- Golden: popup and manager renders for each item kind, masked and unmasked, in each theme.
- Scripted UI: two fixture apps; copy in A, paste in B with and without a gesture; assert the prompt content never contains clipboard bytes; assert a marked secret is absent from the store file, the list API and search; kill the source app and paste.
- Security tests: background polling client; focus-then-read client; copy without a serial; oversized and never-ending offers; a secret followed by a crash of the service (memory file sealed and zeroed on restart); reveal during capture; `act.paste` after focus change.
- Fuzz: offer handling in the service; HTML helper; URL parser; saved-clip JSON import; sync frames; user rule regexes (FORGE step cap).
- Differential: URL parser against the WHATWG URL test data; clean-link rules against a recorded list of tracked and clean pairs.
- Sync: two test instances on a virtual link; pairing with a wrong code; replayed and altered frames; unpair during transfer.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Read policy core | Bytes of the current item reach a client only if (a) a valid paste serial for that client exists, or (b) policy is allow, or (c) a prompt was answered allow; and never for a secure surface | T1 | State-machine model shared with the compositor rule; proof; refinement by PRISM | Compositor is a scaffold |
| Secret handling | An item at level `secret` is never written to the store, never returned by the manage interface, never passed to the sync helper | T1 | Information-flow invariant on the service's item paths | — |
| Secret clearing | After `clear_at`, or lock, the sealed memory is zeroed and the selection withdrawn | T1 | Timer state machine proof | Timing assumptions |
| Offer and record parsers in the service | No runtime errors on any input; bounded memory | T2 | PRISM bounded checks; fuzzing | — |
| HTML helper, URL parser | No runtime errors | T2 | PRISM; fuzzing | — |
| Store encryption wrapper | Uses authenticated encryption for every page and blob; no nonce reuse | T2 | PRISM on the wrapper; review; primitives are FORGE's | Primitive proofs are FORGE's scope |
| Sync helper | No runtime errors on any frame | T2 | PRISM; fuzzing | Sync engine owned by OF02 |
| App logic and transforms (ForgeScript) | Memory safety by the interpreter; transforms terminate within the limit | T3 | Unit tests; step limits | FORGE-WP-01 |
| Detection rules | — | T4 | Measured false positive and negative rates on a test set | Heuristic |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| AX05-W01 | Agree the compositor clipboard rules: single data-control holder, paste-gesture serial, policy query | AX05-GAP-1 |
| AX05-W02 | Platform MIME types for sensitivity, provenance and file handles | AX05-GAP-4, G-09 |
| AX05-W03 | Clipboard service: current item, lazy transfer, survive source exit, size caps | AX05-W01, FUSE-WP-09, FORGE-WP-27, FORGE-WP-28 |
| AX05-W04 | Per-app policy, paste prompt contract with SH04, audit entries | AX05-W03, SH04, AD05, `svc.audit` |
| AX05-W05 | Sensitivity levels, secret timer, clear on lock, SE04 marking | AX05-W02, SE04, `svc.keyring` |
| AX05-W06 | Encrypted history store with text and multi-format items, dedupe, expiry, pins | AX05-W03, FORGE-WP-22 |
| AX05-W07 | `svc.clipboard` and `:manage` interfaces and ForgeScript clients | G-08, FORGE-WP-24.1 |
| AX05-W08 | History popup: list, search, digits, pin, delete, plain paste | G-02, G-07, AX05-W07 |
| AX05-W09 | Text paste through AX04's panel; fallback toast | AX04-W10 |
| AX05-W10 | Primary selection under the same rules | AX05-W01, FUSE-WP-09.3 |
| AX05-W11 | Manager window: history, pinned, apps page, settings; indicator | AX05-W08, SH02 |
| AX05-W12 | Crash recovery and journaling | AX05-W06 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| AX05-W13 | Images, files, colours, links, rich text kinds; previews through `svc.thumbnail` | `svc.thumbnail`, SY02, AX05-W06 |
| AX05-W14 | Paste of non-text items by the a11y Paste action | AX05-GAP-6, FUSE-WP-56 |
| AX05-W15 | Saved clips, folders, placeholders, import and export, hand-off to AX04 | AX05-W11, AX04-W23 |
| AX05-W16 | Paste-as menu, case and whitespace transforms, clean link, invisible character mark | AX05-W08 |
| AX05-W17 | Detection rules, user rules, private level, masking, reveal-block during capture | AX05-W05, SH02 |
| AX05-W18 | Access log page, paste notices, background change warning, provenance flags | AX05-W04, SY03, IN01 |
| AX05-W19 | Paste queue, merge, edit item, restore after paste | AX05-W08 |
| AX05-W20 | HTML helper: plain extraction | FORGE-WP-28 |
| AX05-W21 | Annotate hand-off to ME05; item actions through intents | ME05, `svc.intent` |
| AX05-W22 | Store repair; first-run sheet; AX03 key | AX05-W12, AX03 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| AX05-W23 | Device sync: pairing UI, sync helper, manual and automatic modes | OF02 sync engine, AX05-GAP-5, FORGE-WP-21, FORGE-WP-22 |
| AX05-W24 | Encode, structure and text-tool transforms; tables | AX05-W16 |
| AX05-W25 | HTML to Markdown; formatted preview | AX05-W20, G-10 |
| AX05-W26 | Image transforms and metadata strip | `svc.thumbnail` |
| AX05-W27 | Custom ForgeScript transforms | FORGE-WP-08 |
| AX05-W28 | Managed policy file; statistics; tags and notes | AX05-W11 |
| AX05-W29 | Primary selection recording option; OCR hand-off; voice actions | ME05, AX06 |

## 13. Open questions and risks

1. **Who owns the service.** `svc.clipboard` is listed as a platform service (PLATFORM section 8, G-09). This spec ships it as a helper of this bundle so that policy and UI evolve together. If the platform moves it into the shell, the interfaces in section 6.4 stay and this bundle keeps only the UI.
2. **Compatibility of the paste rule.** Standard Wayland lets the focused client read the selection at any time. Apps from the compatibility personalities (VM03) and some toolkits read on focus to enable their Paste button. They need only the type list (F004), which is allowed; those that read bytes early will hit the prompt. A per-app "allow while focused" policy may be needed.
3. **Vouching for popup pastes.** F103 treats the user's choice in the popup as the paste gesture for the target app. The shell must vouch for that choice; the mechanism (a one-use token from the compositor bound to the target surface) needs design with AX05-W01.
4. **Secure delete on ZFS.** Overwriting a file on a copy-on-write file system does not destroy old blocks, and snapshots keep them. Mitigation is encryption (old blocks are ciphertext) plus excluding the store from snapshots; destroying the per-item key is the real deletion. To verify against the dataset layout of the OS plan.
5. **Detection rules.** Heuristics will miss secrets and flag harmless text. The honest promise is: marked secrets are safe; unmarked ones are best effort.
6. **Compositor modules are scaffolds.** The data-control, data-device and primary-selection modules read are opcode tables of about 20 lines; no selection logic was read.
7. **Sync engine does not exist yet.** It is assigned to OF02 in the roadmap. Phase 3 here waits on it and on a local-link network capability.
8. **Rich text preview.** Without G-10 the preview is plain text; users may paste formatted content without seeing what formatting it carries. The "formatted" badge is the stopgap.
9. **Saved clips versus AX04 snippets.** Two places for reusable text may confuse. The split is: here you pick a clip; in AX04 you type an abbreviation. The hand-off (F040) links them; a later merge of the two stores is possible.
