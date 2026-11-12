<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Messenger

By Odin Loch

| | |
|---|---|
| ID | `CO05` (from CATALOGUE.md) |
| Bundle | `org.pbsd.messenger` |
| Category | comms |
| Priority | P1 |
| Backing repos | FUSE (widgets; `videoWidget` is a frame-source stub, PLATFORM.md V3), FORGE (ForgeScript; Monocypher primitives), ANONYMOUS (AEGIS mixnet transport, used through SE03). No chat protocol code exists in any repo read. |
| Status | Planned. Nothing built. |

## 1. Purpose

Messenger is one client for three open chat networks: Matrix, XMPP and IRC. It gives end-to-end encryption with clear verification, keeps an encrypted local history, and can route any account through the anonymous transports that SE03 controls. It replaces NeoChat, Konversation and Kopete-class tools from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Matrix client-server API with end-to-end encryption (Olm, Megolm), device verification, cross-signing, secret storage and key backup.
- XMPP with OMEMO, and the extensions of a modern client (section 6.3).
- IRC with IRCv3 capabilities and SASL.
- Rooms, spaces, direct chats, threads, replies, reactions, edits, deletions, mentions, read state.
- Media and files with sandboxed preview. Voice messages.
- Voice and video calls and screen share (Matrix and XMPP Jingle). The real-time media stack does not exist and must be built or imported; section 6.5 states what.
- Disappearing messages, local encrypted history, search, export.
- Multi-account, per-account network route, notification policy, moderation tools, bridge awareness.
- Contact trust states shared with CO03.

Out of scope:

| Item | Owner |
|---|---|
| Circuits, VPN profiles, mixnet membership, route health | SE03 Anonymity (Messenger selects a route per account) |
| Storage of long-term keys outside the messenger protocols, hardware tokens | SE04 Passwords and Keys (Messenger keeps protocol keys in `svc.keyring`) |
| Address book, trust scale definition | CO03 Contacts (Messenger reads and writes trust records) |
| Mail | CO01 Mail |
| Media decode and encode pipeline, camera and microphone capture | G-11 pipeline owned by ME03; device portals SH04 |
| Opening received files | The app registered for the type, through `svc.intent` |
| Proprietary networks (Signal, WhatsApp, Telegram, Discord, Slack) | Not planned. Reached only through server-side bridges the user's server runs. |
| Running a homeserver, XMPP server or IRC bouncer | Not planned |
| Group calls with a selective forwarding unit (MatrixRTC, Jitsi) | Phase 3, after one-to-one calls work |
| Bots, widgets, embedded web apps in rooms | Not planned (no web view) |

## 3. Users and top tasks

Personas:

- **Rae, daily user.** One Matrix account for friends and family; expects it to behave like a phone messenger.
- **Sol, journalist.** A second Matrix account and an XMPP account over an anonymous route; verifies every contact; uses disappearing messages.
- **Ines, community admin.** Moderates large Matrix spaces and several IRC channels.
- **Tam, IRC regular.** Twelve channels on three networks, keyboard only.

| # | Task | Target steps |
|---|---|---|
| T1 | Read and answer the newest message | 2 (click the room or `Alt+Down`, type, Enter) |
| T2 | Start a direct chat with a contact | 2 (`Ctrl+N`, pick) |
| T3 | Jump to any room | 2 (`Ctrl+K`, type) |
| T4 | Send a file or image | 1 drag or paste, Enter |
| T5 | Verify a contact | 3 (shield, Start, compare emoji and confirm) |
| T6 | Verify a new device of one's own | 2 (accept request, confirm emoji or scan) |
| T7 | Reply, react or edit | 1 key on the focused message (`R`, `E`, `+`) |
| T8 | Start a voice call | 1 click, then the microphone prompt on first use |
| T9 | Search history | 2 (`Ctrl+F`, type) |
| T10 | Add an account | 3 (network, address, sign in) |
| T11 | Mute a room | 2 |
| T12 | Remove a spammer from a room | 2 (context menu, Ban with reason) |
| T13 | Open a received file safely | 2 (click, "Open in disposable sandbox") |
| T14 | Turn on disappearing messages | 2 |
| T15 | Restore history on a fresh install | 2 (sign in, enter the recovery key or verify from another device) |

## 4. Features

### 4.1 Accounts and connection

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F001 | Multi-account | Any number of accounts across the three networks; each has a colour, an avatar and a route. Endpoints in `svc.accounts`; tokens and keys in `svc.keyring`. | M | 1 |
| CO05-F002 | Matrix sign-in | Server discovery by `.well-known/matrix/client`; password login; SSO and OIDC (MSC3861) through the trusted browser hand-off (IN01 disposable profile, loopback redirect); login by QR from another device (MSC4108) in Phase 3. | M | 1 |
| CO05-F003 | Matrix registration | Only where the server allows it without a web captcha; otherwise a link opens in IN01. | C | 3 |
| CO05-F004 | XMPP sign-in | SRV and host-meta discovery, STARTTLS or direct TLS (XEP-0368), SASL SCRAM-SHA-256(-PLUS), SASL2 and Bind2 with FAST tokens when offered; in-band registration (XEP-0077) where allowed. | M | 2 |
| CO05-F005 | IRC connection | TLS only by default (plain needs an explicit per-network override); SASL PLAIN, EXTERNAL (client certificate from SE04), SCRAM-SHA-256; NickServ fall-back; server password; bouncer support (`znc.in/*` and soju capabilities, `chathistory`). | M | 2 |
| CO05-F006 | Per-account route | `clear`, `vpn:<profile>`, `tor`, or `mixnet` (AEGIS through SE03, Phase 3). Enforced by the network service; fail-closed; route badge on the account. | M | 2 |
| CO05-F007 | Route hygiene | On anonymous routes: no discovery on the clear route, a separate circuit identity per account, no device name beyond a random label, time zone hidden from the protocol where possible, typing and read receipts off by default, calls disabled (they would expose addresses or fail on latency). | M | 2 |
| CO05-F008 | Onion and alternate endpoints | A server may be given as an onion address or a per-route URL; the account record stores one endpoint per route. | S | 2 |
| CO05-F009 | Sync loop | Matrix sliding sync (MSC4186) when offered, else `/sync` v2 with lazy-loaded members and a filter; XMPP stream management (XEP-0198) with resume; IRC keep-alive and auto-rejoin. Back-off with jitter 2 s to 5 min. | M | 1 |
| CO05-F010 | Background connection | With `bg.run`: stays connected with no window for notifications; honours `svc.power` and metered state. | M | 1 |
| CO05-F011 | Sessions and devices list | Own devices per Matrix account with name, last seen, verification state; rename; sign out one or all others (needs re-authentication). XMPP: own OMEMO devices. | M | 2 |
| CO05-F012 | Account data portability | Export account settings without secrets; Matrix account deactivation and XMPP account removal behind a typed confirmation. | S | 3 |
| CO05-F013 | Presence | Online, away, do-not-disturb, invisible where the protocol allows; automatic away on idle is off by default; per-account. | S | 2 |
| CO05-F014 | Profile | Display name, avatar (metadata stripped), status message; per-room display name and avatar on Matrix; XMPP vCard4 and PEP avatar. | M | 1 |
| CO05-F015 | Server capability view | Supported versions and features, upload limit, retention policy, TLS details. | S | 2 |
| CO05-F016 | Offline mode | All history readable; messages queue in the outbox and send in order; a clock icon marks queued items. | M | 1 |

### 4.2 Conversations and navigation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F017 | Unified room list | All accounts in one list, sections: Favourites, People, Rooms, Channels (IRC), Low priority, Invites; or grouped by account; unread and mention badges. | M | 1 |
| CO05-F018 | Spaces | Matrix spaces as a rail of icons and a tree; XMPP bookmarks groups and IRC networks appear as peer groupings. Space home lists child rooms with join buttons. | M | 2 |
| CO05-F019 | Room list ordering | Activity, alphabetical, or manual; unread first option; pinned rooms. | M | 1 |
| CO05-F020 | Quick switcher | `Ctrl+K`: rooms, people, spaces; unread first; fuzzy match. | M | 1 |
| CO05-F021 | Direct chats | One-to-one and small groups; Matrix `m.direct`, XMPP roster, IRC queries. Started from the CO03 picker or an address. | M | 1 |
| CO05-F022 | Create room | Name, topic, private or public, encryption on by default for private rooms (cannot be turned off later on Matrix), history visibility, parent space, alias; XMPP MUC creation with config form (XEP-0045); IRC `JOIN`. | M | 2 |
| CO05-F023 | Join and explore | Join by address, alias, link (`matrix:`, `xmpp:`, `irc:`, `ircs:`, `https://matrix.to` parsed locally); room directory search per server; knock (request to join); previews where the server allows. | M | 2 |
| CO05-F024 | Invites | Listed separately; preview shows inviter, their trust state and whether you share rooms; Accept, Decline, Decline and block. Invites from strangers can be hidden by policy. | M | 1 |
| CO05-F025 | Timeline | Virtualised message list with day separators, grouped consecutive messages per sender, unread marker line, jump to latest, jump to date. | M | 1 |
| CO05-F026 | Threads | Matrix threads (`m.thread`) in a side panel with a thread list per room; XMPP reply chains shown as threads when `thread` IDs exist. | S | 2 |
| CO05-F027 | Read state | Read receipts (public or private per account), fully-read marker, per-thread receipts; XMPP chat markers (XEP-0333); "mark unread". | M | 1 |
| CO05-F028 | Typing notices | Send and show; sending is a setting per account (default on for clear route, off for anonymous). | S | 2 |
| CO05-F029 | Member list | Roles, presence, device trust shield, search; IRC modes shown as roles (`@`, `+`). | M | 1 |
| CO05-F030 | Room info panel | Topic, address, encryption state, members, pinned messages, files and media gallery, links, notification setting, retention and disappearing timer. | M | 2 |
| CO05-F031 | Pinned messages | Pin and unpin (Matrix state, XMPP not supported: local pins). | S | 2 |
| CO05-F032 | Favourites, low priority, tags | Matrix room tags; local equivalents elsewhere. | S | 1 |
| CO05-F033 | Tabs and split | Rooms open in tabs; two rooms side by side by dragging a tab to an edge; a room can be popped out to a window. | S | 2 |
| CO05-F034 | Draft per room | Unsent text and attachments are kept per room and restored after restart. | M | 1 |
| CO05-F035 | Activity view | All mentions and replies to you across accounts, newest first. | S | 2 |
| CO05-F036 | IRC niceties | Server buffer, nick completion, `/` commands (`/join /part /msg /me /nick /mode /topic /kick /ban /whois /away /quote`), join and part noise folded, mIRC colour codes rendered (stripped on request), CTCP ACTION, VERSION answered with a fixed generic string, DCC refused. | M | 2 |
| CO05-F037 | Slash commands everywhere | `/me`, `/shrug`, `/plain`, `/invite`, `/kick`, `/ban`, `/nick`, `/topic`, `/join`, `/leave`, `/verify`; completion popover with descriptions. | S | 2 |

### 4.3 Messages

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F038 | Composer | Multi-line; Enter sends, Shift+Enter new line (swappable); Markdown input with live styling; optional rich toolbar (G-10); character and upload size hints. | M | 1 |
| CO05-F039 | Formatting | Bold, italic, strike, code, code block with language, quote, lists, links, spoiler, headings. Sent as Matrix `org.matrix.custom.html` with a plain fallback, XMPP XEP-0393 styling, IRC formatting codes. | M | 1 |
| CO05-F040 | Safe rendering | Incoming formatted bodies pass an allow-list in the message sanitiser helper and become typed runs for the G-10 view. No remote fetch, no CSS, no scripts. Maths (`data-mx-maths`) shown as source text. | M | 1 |
| CO05-F041 | Replies | Reply with a quoted excerpt; click jumps to the original; fallbacks stripped. XMPP XEP-0461. IRC: `nick:` prefix and `+draft/reply` tag when available. | M | 1 |
| CO05-F042 | Reactions | Emoji and short text reactions (Matrix annotations, XEP-0444); picker from AX04's emoji data; who-reacted list. | M | 1 |
| CO05-F043 | Edits | Edit own messages (`m.replace`, XEP-0308); "edited" marker; edit history viewer. | M | 1 |
| CO05-F044 | Delete | Redact for everyone (Matrix redaction, XEP-0424) or hide locally; the placeholder states which. | M | 1 |
| CO05-F045 | Mentions | `@user`, `@room` with completion; intentional mentions (`m.mentions`); highlight colour and badge. | M | 1 |
| CO05-F046 | Emoji and stickers | `:shortcode:` completion; custom room emoji packs (image packs, MSC2545) shown and sendable; sticker packs from room state only, no web picker. | S | 2 |
| CO05-F047 | Links | Real target on hover; confirmation per host on first open; tracking parameters stripped on copy and open. | M | 1 |
| CO05-F048 | Link previews | Off by default. Options: never; server-generated (Matrix `preview_url`, leaks the URL to the homeserver, never used in encrypted rooms); local fetch through the fetch proxy on the account's route. | S | 2 |
| CO05-F049 | Polls | Matrix polls (MSC3381): create, vote, end, results. | S | 3 |
| CO05-F050 | Location share | Static location (`m.location`) typed or picked on IN04; display as text with "Open in Maps". Live location not planned. Needs no `dev.location` unless the user asks to use the current position. | C | 3 |
| CO05-F051 | Message actions | Copy text, copy link (permalink), quote, forward to another room (re-encrypts media), select several, view source (decrypted JSON or stanza), report. | M | 1 |
| CO05-F052 | Send states | Queued, sending, sent, delivered (XEP-0184 where available), read; failed with Retry and Delete; unknown-device and untrusted-device send blocks explained inline. | M | 1 |
| CO05-F053 | Scheduled send | Hold locally until a time; needs the app running (`bg.run`); Matrix delayed events (MSC4140) used when offered. | C | 3 |
| CO05-F054 | Translate and summarise | Through `svc.model`, on request, shown as machine output beside the original; never sent to a server. Off by default. | C | 3 |
| CO05-F055 | Spell check | G-17 service in the composer. | S | 2 |
| CO05-F056 | Voice messages | Record (microphone portal), waveform preview, send as Opus in Ogg (`m.audio` with voice flag, XEP-0363 upload); playback with speed control. | S | 2 |
| CO05-F057 | Local transcription of voice messages | Optional, through `svc.model`. | C | 3 |

### 4.4 Media and files

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F058 | Send files and media | Portal picker, drag, paste; several at once with captions; progress and cancel; resumable where the server allows. | M | 1 |
| CO05-F059 | Encrypted attachments | Matrix: AES-256-CTR with SHA-256 per the spec, keys inside the event. XMPP: `aesgcm:` links (XEP-0454) and OMEMO media sharing. | M | 1 |
| CO05-F060 | Metadata stripping | EXIF, GPS and other metadata removed from images, audio and video before upload by default; a per-send switch keeps it. | M | 1 |
| CO05-F061 | Image handling before send | Optional scale-down, format re-encode, blur-hash and thumbnail generation, all in the media helper. | S | 2 |
| CO05-F062 | Inline previews | Images, GIF and stickers decoded in the media helper sandbox; video poster frame; audio player; files as chips with sniffed type. Auto-download limit per network type (default: images under 5 MiB in rooms you joined; nothing from invites). | M | 1 |
| CO05-F063 | Media viewer | Full-window viewer with zoom, next and previous in the room, save, forward, open with. Video and audio play through the G-11 pipeline. | M | 2 |
| CO05-F064 | Dangerous type gate | Executables, scripts, shortcuts, macro documents: never opened directly; Save or "Open in disposable sandbox" only. Extension and content mismatch is flagged. | M | 1 |
| CO05-F065 | Open in disposable sandbox | The target app runs in a throw-away domain with `net.none` and only the file handle. Default for files from unverified senders. | M | 2 |
| CO05-F066 | Save | Through `svc.portal.file`; saved files carry provenance attributes (sender, room, event ID, hash). | M | 1 |
| CO05-F067 | Media gallery per room | Grid of images and videos, list of files and links, with search by name and sender. | S | 2 |
| CO05-F068 | Authenticated media | Matrix authenticated media endpoints (spec 1.11) used; no unauthenticated thumbnail fetches. | M | 1 |
| CO05-F069 | Storage management | Per-room and total cache size, clear media older than N days, keep-for-ever pins. | S | 2 |

### 4.5 End-to-end encryption and verification

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F070 | Matrix E2EE | Olm (double ratchet over Curve25519, AES-256-CBC, HMAC-SHA-256) for device-to-device; Megolm for rooms; one-time and fallback keys; key requests and forwarding by policy; session rotation by message count and time; device lists tracked. All in the crypto helper. | M | 1 |
| CO05-F071 | Cross-signing | Master, self-signing and user-signing keys; bootstrap on first login; reset with warnings. | M | 1 |
| CO05-F072 | Device verification | SAS emoji and decimal compare (`m.sas.v1`), QR code show and scan (scan through ME04's `scan` intent or a screenshot image), in-room and to-device flows. | M | 1 |
| CO05-F073 | Secret storage and recovery | SSSS (4S) with a recovery key or passphrase; the recovery key is offered to SE04 for safekeeping and shown once; secrets gossip between own verified devices. | M | 1 |
| CO05-F074 | Key backup | Server-side Megolm backup (`m.megolm_backup.v1.curve25519-aes-sha2`): enable, restore, progress, mismatch warnings. | M | 1 |
| CO05-F075 | Manual key export and import | Megolm session export file (passphrase-protected), for moving between clients. | S | 2 |
| CO05-F076 | OMEMO | XEP-0384 (0.3 `eu.siacs.conversations.axolotl` for reach, and the current namespace with SCE where peers support it): bundles on PEP, sessions, pre-key handling, device list, group chats in non-anonymous member-only MUCs. | M | 2 |
| CO05-F077 | OMEMO trust | Blind trust before verification (optional), then manual fingerprint or QR verification; ATM (XEP-0450) trust messages in Phase 3. | M | 2 |
| CO05-F078 | Other XMPP encryption | OpenPGP for XMPP (XEP-0373/0374) through SE04 keys, Phase 3; OTR not supported. | C | 3 |
| CO05-F079 | Room encryption state | Header shield: all devices verified (ok), some unverified (warn), verified user has a new unverified device (danger), not encrypted (plain icon and the words "Not encrypted"). IRC rooms always say "Not end-to-end encrypted". | M | 1 |
| CO05-F080 | Per-message authenticity | Each message knows the sending device and its trust at receipt; warnings for unverified device, unknown device, device deleted, or key forwarded by another device. | M | 1 |
| CO05-F081 | Send policy | Per account and per room: send to all devices; refuse unverified devices of verified users (default); only verified devices. Blocked sends say which device and why. | M | 2 |
| CO05-F082 | Trust states shared with CO03 | On verification success or identity change Messenger writes a trust record (`matrix` or `xmpp` scheme) through `org.pbsd.contacts.trust`, and reads the state to badge contacts. Identity change of a verified user raises the alert in both apps. | M | 2 |
| CO05-F083 | Undecryptable messages | Clear reason ("sent before you joined", "your device was not given the key", "sender's device unverified and policy blocked it"); automatic key request; manual retry. | M | 1 |
| CO05-F084 | Key history sharing on invite | Share room keys with an invited user when history visibility allows (MSC3061), with a prompt. | C | 3 |
| CO05-F085 | Dehydrated devices | Receive keys while no device is online (MSC3814). | C | 3 |
| CO05-F086 | Security overview | One page per account: cross-signing state, backup state, recovery set-up, unverified sessions, send policy, with a "fix" button on each gap. | M | 2 |
| CO05-F087 | Verification UX rules | Plain words, one question per step, emoji with names in the user's language, a "They don't match" button as prominent as "They match", cancel reasons shown, no jargon in the main path. | M | 1 |

### 4.6 Calls

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F088 | One-to-one voice calls | Matrix VoIP v1 (`m.call.*`) and XMPP Jingle (XEP-0166/0167/0176/0320/0353). Opus audio. Needs the real-time stack of section 6.5. | M | 2 |
| CO05-F089 | Video calls | Camera through `svc.portal.camera`; VP8 baseline, AV1 or VP9 when both sides offer; resolution and frame rate adapt to bandwidth. | S | 2 |
| CO05-F090 | Screen share | Screen or window chosen in the shell picker (`svc.portal.screen`); a visible indicator; cursor optional; never shares notifications. | S | 3 |
| CO05-F091 | Call UI | Incoming call notification with Answer, Decline; in-call window with mute, camera, share, hold, device pickers, network quality, timer; picture-in-picture mini window. | M | 2 |
| CO05-F092 | Call privacy | TURN-relay-only mode (hides the IP address from the peer) on by default for contacts that are not verified and always on VPN routes; calls disabled on `tor` and `mixnet` routes. STUN and TURN servers come from the account's server only. | M | 2 |
| CO05-F093 | Call encryption display | DTLS-SRTP fingerprint bound to the signalling session; for encrypted Matrix rooms the signalling is E2EE so the fingerprint is authenticated by device trust; the call window shows the peer's device trust. | M | 2 |
| CO05-F094 | Group calls | MatrixRTC with a selective forwarding unit and frame encryption (SFrame-style, keys over Megolm); XMPP via an external conference is not planned. | C | 3 |
| CO05-F095 | Audio processing | Echo cancellation, noise suppression, automatic gain; push-to-talk option; test call (loopback) screen. | M | 2 |
| CO05-F096 | Media session integration | `svc.media` shows the call; hardware mute and hang-up keys work; `svc.power` inhibits sleep during a call. | S | 2 |
| CO05-F097 | Call history | Local list of calls with duration; missed-call notifications. | S | 2 |

### 4.7 History, search, export

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F098 | Local encrypted history | All events stored decrypted inside a database encrypted with a per-account key from `svc.keyring` (page-level XChaCha20-Poly1305). Megolm sessions kept so history can be re-verified. | M | 1 |
| CO05-F099 | Back-pagination and archive | Scroll loads older history from the server (`/messages`, XEP-0313 MAM, IRC `chathistory`) and stores it. | M | 1 |
| CO05-F100 | Full-text search | Local index (inside the encrypted store) over all rooms including encrypted ones; filters `in:`, `from:`, `has:file/image/link`, `before:`, `after:`, `account:`; results in context. | M | 2 |
| CO05-F101 | Server search | For unencrypted Matrix rooms not fully cached; marked "from server". | C | 3 |
| CO05-F102 | Disappearing messages | Per room timer (off, 1 h, 1 day, 1 week, custom). Matrix has no ratified standard: implemented as a room state event plus per-event expiry (self-destruct, MSC2228 style) honoured by this client and by clients that follow it; the room states plainly that other clients may keep messages. XMPP: message retraction after the timer on a best-effort basis. Local copies are erased at expiry, including the search index and media cache. | M | 2 |
| CO05-F103 | Local retention | Per account or room: keep for ever, keep N days locally, keep nothing after reading (ephemeral session). | S | 2 |
| CO05-F104 | Export | Room or range to HTML (static, no scripts), plain text, JSON (events as stored), with or without media; signed with the user's key through SE05 on request. | M | 2 |
| CO05-F105 | Import | Megolm key files; history from another install of this app (encrypted archive); XMPP and IRC log import (plain text logs) Phase 3. | S | 3 |
| CO05-F106 | Panic erase of an account | Remove tokens, keys, history and cache for one account at once, after one confirmation; optionally sign the device out first. | S | 2 |
| CO05-F107 | Backup rules | `svc.backup` includes the encrypted stores and excludes media cache; restore needs the keyring item. | M | 2 |

### 4.8 Notifications

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F108 | Notification policy | Global default, per account, per space, per room: all messages, mentions and keywords, none. Matrix push rules are read and written so other clients agree. | M | 1 |
| CO05-F109 | Content privacy | Notification shows sender and text, sender only, or "New message"; lock-screen default is "New message". Encrypted-room text is never shown on the lock screen unless allowed. | M | 1 |
| CO05-F110 | Actions | Reply inline, Mark read, Mute 1 h; calls: Answer, Decline. | M | 1 |
| CO05-F111 | Keywords | Custom highlight words per account. | S | 2 |
| CO05-F112 | Quiet hours and focus | Follows system do-not-disturb; own schedule per account; calls from favourites may break through (setting). | S | 2 |
| CO05-F113 | Badges and sounds | Unread and mention counts on the launcher; one short system sound per burst, never one per message. | S | 1 |
| CO05-F114 | No push gateway | Notifications come from the app's own connection (`bg.run`). No third-party push service is used. Stated in settings so the user knows the app must run. | M | 1 |

### 4.9 Moderation and safety

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F115 | Member actions | Kick, ban, unban, mute (power level), change role, with a reason field. XMPP MUC affiliations and roles. IRC `KICK`, `MODE +b`, `+q`, `+v`, `+o`. | M | 2 |
| CO05-F116 | Room settings | Name, topic, avatar, join rules (invite, knock, restricted to a space, public), history visibility, guest access, power levels editor, aliases, server ACLs, upgrade room version. | M | 2 |
| CO05-F117 | Message moderation | Redact one, redact all recent by a user, with a reason. | M | 2 |
| CO05-F118 | Ignore and block | Ignore user (Matrix ignored-users list, XEP-0191 blocking); hides messages and invites; optional sync to the CO03 block flag. | M | 1 |
| CO05-F119 | Report | Report a message or room to the server admins (Matrix report API, XEP-0377). | S | 2 |
| CO05-F120 | Policy lists | Subscribe to Matrix moderation policy rooms (`m.policy.rule.*`) and apply them as local ignores or, for admins, as suggested bans. | C | 3 |
| CO05-F121 | Invite and spam controls | Auto-reject invites from non-contacts, from servers on a block list, or with no shared room; preview media in invites is never loaded. | M | 2 |
| CO05-F122 | Audit of admin actions | A per-room log view of state changes (who changed what). | S | 3 |
| CO05-F123 | Space administration | Add and remove child rooms, suggested rooms, order, permissions inheritance view. | S | 3 |

### 4.10 Bridges, integration, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO05-F124 | Bridge awareness | Detect bridged rooms (`m.bridge` / `uk.half-shot.bridge` state, XMPP gateway disco identities) and bridged users (ghost patterns from the bridge state). The room header says "Bridged to <network>: messages leave end-to-end encryption at the bridge". Per-message network icon. | M | 2 |
| CO05-F125 | Bridge limits | Features a bridge cannot carry (edits, reactions, threads) are marked before sending when the bridge state declares them. | C | 3 |
| CO05-F126 | Contact integration | Names, avatars and trust from CO03 `lookup`; "Add to contacts" writes the identity; start chat from a contact through the `compose` intent. | M | 2 |
| CO05-F127 | Intents and links | Handles `compose` (to an identity), `share` (text, files to a chosen room), URL schemes `matrix:`, `xmpp:`, `irc:`, `ircs:`; emits `open`, `view`, `share`, `pick`, `scan`. | M | 1 |
| CO05-F128 | Meeting links from Calendar | CO02 "Join" opens the room or call here. | S | 3 |
| CO05-F129 | Command palette | Every action, room, person and setting. | M | 1 |
| CO05-F130 | Appearance | Bubbles or flat (IRC-style) layout, compact mode, timestamps always or on hover, 12 or 24 hour, avatar size, name colours hashed per user (contrast-checked). | S | 1 |
| CO05-F131 | Session restore | Open rooms, scroll positions, drafts, tabs. | M | 1 |
| CO05-F132 | Store recovery | Integrity check; rebuild the search index; re-sync a room; recover E2EE state from secret storage and backup after store loss. | M | 2 |
| CO05-F133 | Audit entries | First contact with each server, sign-ins, device verifications, key resets, sandbox opens, device use (camera, microphone, screen) to `svc.audit`. | M | 2 |
| CO05-F134 | Screen reader mode | Messages as one line each with sender, time, state; new messages announced politely in the open room; reactions summarised; a key to read the last N messages. | M | 2 |
| CO05-F135 | Captions for calls | Optional live local captions through `svc.model`. | C | 3 |
| CO05-F136 | Diagnostics | Redacted protocol log per account, off by default, kept in memory unless the user saves it; nothing is sent anywhere. | S | 2 |
| CO05-F137 | Mixnet transport for text | Phase 3: accounts between members of the same AEGIS deployment exchange messages over the mixnet through SE03. Multi-second latency is expected and shown; no calls; attachments limited by the bulk plane ceiling. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Messenger
├─ Main window
│  ├─ Header bar: menu, title (room name), search, device indicators (mic, camera, screen), [net]
│  ├─ Space rail (48 px): accounts, spaces, activity, add
│  ├─ Room list (sidebar): filter field, sections, invites
│  ├─ Conversation: room header (shield, topic, call buttons), banners, timeline, composer
│  ├─ Right panel (F4): room info | members | thread | search results | media gallery
│  └─ Status bar: connection per account, outbox, route
├─ Call window (also docked as a strip in the conversation; picture-in-picture mini window)
├─ Media viewer window
├─ Dialogs: account wizard · verification (SAS emoji, QR, fingerprint) · recovery set-up and
│  restore · key backup · security overview · sessions and devices · create room · join and
│  explore · room settings (General, Security, Roles, Notifications, Advanced) · export ·
│  storage · preferences
├─ Popovers: emoji and reaction picker · mention, emoji and command completion · message
│  actions · link confirmation · notification level · disappearing timer · device picker
├─ Sheets: invite preview · untrusted-device send block · identity changed alert ·
│  attachment send preview · report · moderation action with reason
└─ Shell-drawn: capability prompts (microphone, camera, screen), screen and window picker,
   file portal, notifications with inline reply
```

### 5.2 Main window

Shown at 1280×800.

```
+------------------------------------------------------------------------------------------+
| [=] Messenger — Sol Marsh         [/ Search…            ]          [🎤] (1) [net] _ ▢ ✕   |
+----+---------------------+---------------------------------------------+-----------------+
|(R) | [Filter rooms…    ] | (SM) Sol Marsh  [✔ Verified] 🔒 E2EE   (4)   | ROOM INFO    (9)|
|(W) | INVITES           1 | @sol:example.org · online      [📞] [🎥] [⋯] | Sol Marsh       |
|----|  ? Unknown room     +---------------------------------------------+ @sol:example.org|
| ⌂  | FAVOURITES          | ⏱ Messages disappear after 1 week     (5)   | ✔ Verified by   |
| ◆A | >(SM) Sol Marsh   2 |---------------- Wed 18 Nov -----------------|   you, 3 Mar    |
| ◆B |  (FA) Family      ● | Sol  14:02                                   | 2 devices, both |
|    | PEOPLE              |  Did you get the draft?                     |   verified      |
| ✉  |  (IV) Ines Vale     |  [brief.pdf 212 KiB ▾]                 (6)   |-----------------|
|    |  (TM) Tam ⚠         |                         Yes, reading it now  | Notifications   |
| +  | ROOMS               |                              14:05 ✓✓  You   |  [All v]        |
|(2) |  # pbsd-dev  12 [tor]|  ↪ Sol: Did you get the draft?              | Disappearing    |
|    |  # design      @1   |                    Section 3 needs a fix (7) |  [1 week v]     |
|    | CHANNELS (IRC)      |                       14:06 ✓ You · edited   | Files (4)       |
|    |  #hardenedbsd       |  👍 1  [+]                                   | Media (12)      |
|    |  #freebsd           |------------------- Today -------------------| Links (3)       |
|    | LOW PRIORITY        | ── new ──────────────────────────────────── | Pinned (1)      |
|    |  # announcements    | Sol  09:12                                   |-----------------|
|    |                 (3) |  Call when you can?                          | [Verify again]  |
|    |                     |  Sol is typing…                              | [Export chat…]  |
|    |                     +---------------------------------------------+ [Ignore user]   |
|    |                     | [+] [ Message Sol…                  ] [☺][🎤]|                 |
|    |                     |      Markdown · Enter sends             (8)  |                 |
+----+---------------------+---------------------------------------------+-----------------+
| ● rae@example.org connected · ● work (tor) connected · Outbox 0                    (10)   |
+------------------------------------------------------------------------------------------+
```

1. Device indicators appear in the header only while a device is in use; here the microphone.
2. Space rail: account avatars with a route dot, home (all rooms), spaces, activity (mentions), add.
3. Room list (260 px default, 200 to 400) with sections; unread count, mention `@` count, dot for unread without notification; a warning icon marks a contact whose identity changed; a route badge marks rooms on an anonymous account.
4. Room header: avatar, name, trust shield with a word, encryption state, call buttons, overflow menu.
5. Banners: disappearing timer, bridge notice, unverified-device warning, offline, key-backup prompt.
6. File chip with a menu: Preview, Open in disposable sandbox, Open with, Save.
7. Own message with a reply excerpt, an "edited" marker and delivery ticks (one tick sent, two read).
8. Composer: attach button, text field with live Markdown styling, emoji, voice message. Hint line states the mode.
9. Right panel: here room info. `F4` toggles it; it also hosts members, thread, search and gallery.
10. Status bar: one dot and word per account, outbox count.

Layout options: bubbles (shown) or flat one-line-per-message for IRC-style reading. The rail collapses into the room list under 900 px.

### 5.3 Other views and dialogs

#### 5.3.1 Verification, emoji compare (560×480)

```
+----------------------------------------------------------+
| Verify Sol Marsh                              Step 2 / 3 |
+----------------------------------------------------------+
| Ask Sol what they see. Do both screens show the same     |
| pictures in the same order?                              |
|                                                          |
|    🐶     🔑     🎩     ☂️     🚀     🌙     🎸            |
|    Dog    Key    Hat  Umbrella Rocket Moon  Guitar       |
|                                                          |
| [Show numbers instead]                              (1)  |
| Best checked in person or on a call where you know       |
| their voice.                                             |
|                                                          |
|        [They don't match]          [They match]     (2)  |
+----------------------------------------------------------+
```

1. Decimal fall-back (three four-digit groups). 2. Both buttons have equal weight; "They don't match" cancels with reason `m.mismatched_sas` and marks the attempt. Steps: 1 Start or accept (choose emoji or QR), 2 Compare, 3 Result (what changed: "Sol's devices are now trusted; this is saved to Contacts"). States: waiting for the other side, timed out, cancelled by peer with the reason in plain words.

#### 5.3.2 Security overview (720×560)

```
+---------------------------------------------------------------------------+
| Security — rae@example.org                                                |
+---------------------------------------------------------------------------+
| ✔ This device is verified                                                 |
| ✔ Cross-signing is set up                                    [Reset…]     |
| ✔ Recovery key exists (stored in Passwords and Keys)         [Show] [New] |
| ⚠ Key backup is off: old messages cannot be restored         [Turn on]    |
| ⚠ 1 of your sessions is not verified: "Phone (Element)"      [Verify]     |
+---------------------------------------------------------------------------+
| Sending    [Refuse unverified devices of verified users v]           (1)  |
| Receiving  [x] Warn on messages from unverified devices                   |
| Key requests  [Only from my verified devices v]                           |
+---------------------------------------------------------------------------+
| Sessions                                                                  |
|  ✔ This device       PBSD desktop      now                                |
|  ✔ Laptop            PBSD laptop       2 h ago          [Sign out]        |
|  ⚠ Phone (Element)   unverified        yesterday        [Verify][Sign out]|
+---------------------------------------------------------------------------+
```

1. The send policy in plain words. Every warning row has one fix button.

#### 5.3.3 Call window (640×480) and docked strip

```
+----------------------------------------------------------+
| Sol Marsh · 04:12 · ✔ verified · relayed (IP hidden) (1) |
+----------------------------------------------------------+
|                                                          |
|                  [ remote video ]                        |
|                                             +----------+ |
|                                             | self     | |
|                                             +----------+ |
+----------------------------------------------------------+
| [🎤 Mute] [🎥 Camera] [🖥 Share] [⏸ Hold] [⚙ Devices] [✕ End]|
| Network: good · Opus 32 kb/s · VP8 640×360 15 fps   (2)  |
+----------------------------------------------------------+
```

1. Peer trust and whether media is relayed or direct. 2. Plain quality line; details on click. States: ringing, connecting (ICE), reconnecting, on hold, peer muted, no microphone granted (button opens the prompt again), route forbids calls (call buttons disabled with the reason).

#### 5.3.4 Account wizard (600×480)

```
+----------------------------------------------------------+
| Add an account                                           |
+----------------------------------------------------------+
| Network  (•) Matrix   ( ) XMPP   ( ) IRC                 |
| Address  [@rae:example.org__________]                    |
| Route    (•) Clear  ( ) VPN [v]  ( ) Tor            (1)  |
| Found:   https://matrix.example.org (well-known)         |
| Sign in  (•) Password [____________]                     |
|          ( ) Single sign-on (opens the browser)     (2)  |
| Device name [PBSD desktop______]  [ ] random label       |
|                                   [Cancel]  [Sign in]    |
+----------------------------------------------------------+
```

1. Chosen before any network contact; discovery then runs on that route. 2. SSO opens IN01 in a disposable profile. After sign-in the wizard continues to "Verify this device" (from another device or with the recovery key) or "Set up recovery" for a new account. IRC page asks network, server, port, nick, SASL, channels to join.

#### 5.3.5 Room settings, Security and Roles (720×560)

```
+---------------------------------------------------------------------------+
| # design — settings                                                       |
+--------------+------------------------------------------------------------+
| General      | Encryption        On (cannot be turned off)                |
| > Security   | Who can join      [Members of space "Work" v]              |
| Roles        | Who can read history [Members, since they joined v]        |
| Notifications| Disappearing messages [Off v]  ⓘ other clients may keep    |
| Advanced     | Send to           [Verified devices only v]                |
|              |------------------------------------------------------------|
|              | Roles            Default  Moderator  Admin                 |
|              | Send messages      ●                                       |
|              | Invite             ●                                       |
|              | Remove messages             ●                              |
|              | Kick, ban                   ●                              |
|              | Change settings                        ●              (1)  |
|              | Members with roles: Rae (Admin) · Ines (Moderator) [Edit]  |
+--------------+------------------------------------------------------------+
```

1. Power levels shown as three named roles; "Advanced" exposes the raw numbers.

#### 5.3.6 Thread panel and search results (right panel, 360 px)

```
| THREAD · "Section 3 needs a fix"        [✕] |   | SEARCH "draft"  in: all rooms  [filters v] |
| You  14:06                                  |   | Sol Marsh · Wed 14:02                      |
|  Section 3 needs a fix                      |   |  Did you get the **draft**?                |
| Sol  14:09                                  |   | # design · 12 Nov · Ines                   |
|  Which paragraph?                           |   |  new **draft** of the icons is up          |
| You  14:10                                  |   | # pbsd-dev · 3 Nov · tam                   |
|  The second one.                            |   |  **draft** RFC attached                    |
| [ Reply in thread…                 ] [☺]    |   | 14 results · searched locally         (1)  |
```

1. States where the search ran. Clicking a result opens the room at that message with context.

Sheets without a drawing: the **invite preview** lists inviter, whether they are a contact, rooms in common, when their server was first seen, member count and encryption state, and loads no media; buttons Decline and ignore user, Decline, Accept. The **attachment send preview** lists each file with its size before and after processing and the switches "Remove location and metadata" and "Scale to 2048 px", plus a caption field.

### 5.4 Menus

Items separated by ` · `; shortcut in brackets.

```
File     New chat [Ctrl+N] · New room… [Ctrl+Shift+N] · Join or explore… [Ctrl+J] ·
         Add account… · Send file… [Ctrl+U] · Export chat… · Work offline · Close tab [Ctrl+W] ·
         Quit [Ctrl+Q]
Edit     Undo [Ctrl+Z] · Redo [Ctrl+Shift+Z] · Cut · Copy · Paste [Ctrl+X/C/V] ·
         Paste as plain text [Ctrl+Shift+V] · Edit last message [Up in empty composer] ·
         Find in room [Ctrl+F] · Search everywhere [Ctrl+Shift+F] · Preferences [Ctrl+,]
View     Quick switcher [Ctrl+K] · Layout > Bubbles | Flat · Compact · Room list > By activity |
         A–Z | Manual | By account · Unread first · Space rail · Sidebar [F9] · Right panel [F4] >
         Room info | Members [Ctrl+Shift+M] | Threads [Ctrl+Shift+T] | Media | Activity ·
         Show join and leave events · Timestamps > · Zoom [Ctrl++ / - / 0] · Full screen [F11]
Go       Next/previous room [Alt+Down/Up] · Next/previous unread [Alt+Shift+Down/Up] ·
         Jump to latest [Ctrl+End] · Jump to date… [Ctrl+G] · Jump to first unread [Ctrl+Home] ·
         Activity [Ctrl+Shift+A] · Back/Forward [Alt+Left/Right]
Room     Mark read [Esc] · Mark unread · Favourite · Low priority · Notifications > All |
         Mentions | None · Mute for > 1 h | 8 h | until I turn it back on · Invite people… ·
         Disappearing messages > · Room settings… · Pin to tabs · Open in new window · Leave room
Message  Reply [R] · React [+] · Edit [E] · Reply in thread [T] · Forward… [F] · Copy text ·
         Copy link · Pin · Select [X] · View source · Report… · Delete for everyone [Delete] ·
         Hide for me
Call     Voice call [Ctrl+Shift+C] · Video call [Ctrl+Shift+V in call menu only] ·
         Mute [Ctrl+D] · Camera [Ctrl+E] · Share screen… · Hold · Devices… · End call [Ctrl+.]
Security Verify user… · Verify this device… · Security overview… · Sessions and devices… ·
         Key backup… · Export room keys… · Import room keys… · Identity details
Tools    Moderation > Kick… | Ban… | Remove recent messages… · Ignored users… · Storage… ·
         Protocol log · Check store
Help     Messenger help [F1] · Keyboard shortcuts [Ctrl+/] · About Messenger
```

### 5.5 Keyboard and pointer

Timeline focus (`Shift+Tab` from the composer, or `Ctrl+Up`): `Up/Down` move between messages; `R`, `E`, `+`, `T`, `F`, `X` act on the focused message; `Enter` opens a file, link or thread; `Space` plays or pauses media; `Esc` returns to the composer and marks read. Composer: `Tab` completes names, emoji and commands; `Up` in an empty field edits the last message; `Ctrl+B/I/E` bold, italic, code; `Ctrl+Shift+U` voice message; `Esc` cancels reply or edit. Push-to-talk key is user-defined and works only while the call window exists.

Context menus:

- **Message**: Reply, React, Edit, Reply in thread, Forward, Copy text, Copy link, Quote, Pin, Select, View source, Report, Delete for everyone, Hide for me.
- **Room**: Mark read, Favourite, Low priority, Notifications, Mute, Invite, Copy address, Settings, Open in new window, Leave.
- **User (member or avatar)**: Message, Call, Verify, View contact (CO03), Add to contacts, Mention, Change role, Kick, Ban, Ignore.
- **File or image**: Preview, Open in disposable sandbox, Open with, Save as, Copy, Forward, Show hash.
- **Link**: Copy clean link, Open (asks per host), Open in disposable browser profile.
- **Account avatar**: Presence, Security overview, Sessions, Settings, Go offline, Sign out.

Drag and drop:

| Drag | Onto | Result |
|---|---|---|
| Files, images, text | Conversation or composer | Attachment send preview |
| Room | Section, space, another position | Tag, add to space, reorder |
| Room tab | Window edge / outside | Split view / new window |
| Message | Composer | Reply to it |
| Message | Another room | Forward (confirm) |
| Image or file chip | Outside (Files, another app) | The decrypted file, with provenance attributes |
| Contact (from CO03) | Room list / member list | Start chat / invite |
| User | Room in list | Invite to that room (confirm) |

Pointer: hover shows the message action bar (react, reply, thread, more); double-click a message replies; `Ctrl`+wheel zooms; swipe right on a message (touch) replies.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty (no account) | "Add an account to start chatting." [Add account]. One line explains that no account is created with any vendor. |
| Empty room | "This is the start of your chat with Sol." plus the encryption and verification state. |
| Loading | Cached timeline at once; a spinner row at the top while older history loads; room list skeleton only on first sync, with a determinate bar where the server reports progress. |
| Initial sync | Wizard page with progress and the option to keep using other accounts. |
| Error | Per account in the status bar ("work: sign-in expired · Sign in"); failed messages are marked in place with Retry. |
| Offline | Banner "Offline · messages will send when connected"; queued messages show a clock. |
| Route down | "Route unavailable (Tor). Not connecting any other way." [Open Anonymity]. |
| Locked | Account store key unavailable: rooms listed without previews; [Unlock]. |
| Read-only | Rooms where the user cannot post (power level, voice needed, banned, tombstoned): composer replaced by the reason and, for upgraded rooms, a link to the new room. |
| Undecryptable | Placeholder with reason and Retry (section 4.5). |
| Identity changed | `--danger` banner in the room until the user verifies again or accepts; sending is blocked meanwhile by default. |
| Narrow (<640) | One pane at a time: room list, then conversation; right panel as a sheet; rail folds into the list header; call window becomes full-window. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Space rail | `listView` of `toolButton`s | |
| Room list | `modelListView` over `SqliteModel`, sectioned | section headers (FUSE-WP-28) |
| Timeline | `modelListView` with variable-height rows, bottom-anchored | missing: bottom-anchored virtual list with stable scroll on prepend (`CO05-GAP-1`) |
| Message body | G-10 rich text view (read-only profile) fed typed runs | selection across messages is `CO05-GAP-2` |
| Composer | G-10 rich text (edit) with completion popovers; plain `codeEdit` fall-back in Phase 1 | |
| Image, sticker previews | `image` from helper bitmaps | animated images need frame scheduling in `image` (`CO05-GAP-3`) |
| Audio, video playback, call video | `videoWidget` | stub today (V3); needs G-11 |
| Voice waveform | missing: waveform (listed as missing in the FUSE build plan gap table) | |
| Emoji picker | `gridView` + `lineEdit` + `tabBar` | data from AX04 |
| Member list, sessions, search results | `modelListView`, `tableView` | |
| Room settings, roles | `tabWidget`, `comboBox`, `checkBox`, `tableView` with radio cells | |
| Verification | `dialog`, `label`, `image` (QR), `button` | |
| Tabs, splitters | `tabWidget`, `splitter` | tear-off FUSE-WP-31 |
| Popovers, sheets, toasts | missing in V3; FUSE-WP-30 | |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, room list, timeline assembly, composer, notification policy, moderation UI logic, outbox |
| UI | FUSE UI | app host | All windows |
| Matrix protocol helper | C++23 | one per account, `proc.helper:matrix` | Client-server API, sync, JSON parsing, event validation, state resolution for display, media transfer. Sockets from the network service. |
| XMPP protocol helper | C++23 | one per account, `proc.helper:xmpp` | Stream, XML parsing (no DTD, no entities), stanza routing, XEP logic, MAM, HTTP upload |
| IRC protocol helper | C++23 | one per network, `proc.helper:irc` | IRC line parser, IRCv3 capabilities, SASL, history |
| Crypto helper | C++23 (own) or Rust (imported library); decision in 6.5 | one per account, `proc.helper:e2ee` | Olm, Megolm, cross-signing, SAS, SSSS, key backup, OMEMO double ratchet and X3DH, attachment encryption. Holds session keys; talks to `svc.keyring` for the store key. No network, no files except its own encrypted key store descriptor. |
| Message sanitiser helper | C++23 | pooled, `proc.helper:msg-safe` | Matrix HTML subset and XHTML-IM to typed runs; allow-list. Shares the tokeniser source with CO01's HTML sanitiser. No capabilities. |
| Media helper | C++23 | pooled, `proc.helper:media` | Decode images and stickers, thumbnails, blur-hash, metadata strip, re-encode; voice message encode. No capabilities. Uses the G-11 codecs. |
| Call engine | C++23 or Rust; section 6.5 | one per call, `proc.helper:rtc` | ICE, DTLS-SRTP, RTP/RTCP, jitter buffer, congestion control, audio processing, codec glue. Gets UDP and TCP sockets from the network service and device streams from the portals. |
| Fetch proxy helper | C++23 | `proc.helper:fetch` | Link previews (when enabled), discovery documents; no cookies, fixed user agent |
| History store | ForgeScript over encrypted SQLite | app host | Events, rooms, index |
| Mixnet transport adapter | Rust (ANONYMOUS client code) behind SE03 | SE03's process | Phase 3 only; Messenger sees a message-oriented bus interface |

```
                +------------------------------------------------------+
                | app host: view models · timeline · outbox · policy   |
                | history store (encrypted SQLite)                     |
                +--+------+------+------+-------+-------+--------+-----+
   session bus     |      |      |      |       |       |        |
        +----------+  +---+--+ +-+---+ ++-----+ +------++ +-----+-+ +-+------+
        v             v      | v     | v      | v       | v       | v        |
   +---------+   +--------+  +-------+ +------+ +-------+ +-------+ +--------+
   | matrix  |   | xmpp   |  | irc   | | e2ee | | msg-  | | media | | rtc    |
   | helper  |   | helper |  | helper| |helper| | safe  | | helper| | engine |
   +----+----+   +---+----+  +---+---+ +--+---+ +-------+ +-------+ +---+----+
        |            |           |        |  keyring                    | UDP/TCP,
        +------------+-----------+        v                             | mic, cam,
                     v              svc.keyring                         v screen
        network service (rules, route via SE03, DNS, TLS)        portals + G-11
```

Ciphertext path: a protocol helper hands encrypted payloads to the app, which passes them to the crypto helper; plaintext comes back to the app and goes to the sanitiser before display. The protocol helpers never see plaintext of E2EE rooms or any key.

### 6.2 Data model

Stores under `~/.local/share/org.pbsd.messenger/<account-id>/`: `history.sqlite` (page-encrypted, key in `svc.keyring`), `crypto.sqlite` (owned by the crypto helper, separately keyed), `media/<blake3>` (encrypted at rest with per-file keys kept in `history.sqlite`). Cache: `~/.cache/org.pbsd.messenger/` (thumbnails; encrypted, key in memory only, so it is useless after logout).

| Entity | Fields |
|---|---|
| `account` | `id: uuid`, `accounts_ref: text`, `network: enum(matrix, xmpp, irc)`, `user_id: text`, `device_id: text?`, `display_name`, `avatar_ref`, `route: text`, `endpoints: json` (per route), `presence: enum`, `policy: json`, `sync_token: text` |
| `room` | `id: i64`, `account_id`, `remote_id: text` (room ID, bare JID, channel), `kind: enum(dm, group, room, space, channel, server)`, `name`, `topic`, `avatar_ref`, `canonical_alias`, `encrypted: bool`, `enc_algo: text?`, `bridge: json?`, `tags: json`, `notify: enum`, `unread: u32`, `highlights: u32`, `fully_read: text`, `last_activity: i64`, `draft: json`, `expiry_s: u32?`, `retention_local: json`, `prev_batch: text`, `membership: enum`, `power_levels: json`, `parent_spaces: json`, `order: f64` |
| `member` | `room_id`, `user_id`, `display_name`, `avatar_ref`, `membership: enum`, `role: i32`, `last_active: i64?` |
| `event` | `id: i64`, `room_id`, `remote_id: text` (event ID, stanza ID, generated for IRC), `sender: text`, `ts: i64`, `type: text`, `content: json` (decrypted), `relates_to: text?`, `rel_type: enum(reply, thread, edit, reaction, none)`, `thread_root: text?`, `state: enum(queued, sending, sent, delivered, failed, received)`, `redacted: bool`, `expires_at: i64?`, `sender_device: text?`, `trust_at_receipt: enum`, `session_id: text?`, `txn_id: text?` |
| `event_edit` | `event_id`, `edit_event: text`, `ts`, `content: json` |
| `reaction` | `event_id`, `sender`, `key: text`, `remote_id` |
| `receipt` | `room_id`, `user_id`, `event_remote_id`, `kind: enum(read, private, fully_read)`, `thread: text?`, `ts` |
| `media` | `id`, `event_id`, `mxc_or_url: text`, `mime`, `sniffed_mime`, `size: u64`, `name`, `blob: text?`, `file_key: blob`, `iv: blob`, `sha256: blob`, `thumb_blob: text?`, `blurhash: text?`, `pinned: bool` |
| `device` | `account_id`, `user_id`, `device_id`, `name`, `ed25519: text`, `curve25519: text`, `trust: enum(verified, cross_signed, unverified, blocked)`, `first_seen: i64`, `deleted: bool` |
| `user_identity` | `account_id`, `user_id`, `master_key: text`, `verified: bool`, `changed_at: i64?`, `contact_ref: text?` |
| `omemo_device` | `account_id`, `jid`, `device_id: u32`, `identity_key: blob`, `trust: enum(undecided, blind, verified, distrusted)`, `active: bool` |
| `outbox` | `seq`, `room_id`, `txn_id`, `payload: json`, `attachments: json`, `not_before: i64?`, `attempts`, `error` |
| `call_log` | `id`, `room_id`, `direction`, `kind: enum(voice, video)`, `started: i64`, `duration_s: u32`, `relayed: bool`, `result: enum` |
| `push_rule_cache`, `ignored_user`, `keyword` | per account policy data |
| `fts` | FTS5 virtual table over `event.content` text, inside the encrypted database |

`crypto.sqlite` (crypto helper only): Olm account, Olm sessions, inbound and outbound Megolm sessions, cross-signing private keys (unless kept only in SSSS), backup key, OMEMO identity, signed pre-keys, pre-keys, ratchet sessions. Migration is forward only with a copy; the crypto store migrates inside the helper.

Expiry: a single timer per account wakes at the next `expires_at`; deletion removes the event row, edits, reactions, media blobs, FTS rows and thumbnails in one transaction, then runs an incremental vacuum.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Matrix client-server API (JSON over HTTPS), spec v1.11 or later | yes | yes | own; HTTP and TLS from FORGE-WP-21 | Matrix helper |
| Matrix E2EE: Olm, Megolm, SAS, cross-signing, SSSS, key backup | yes | yes | section 6.5 | crypto helper |
| Matrix HTML subset (`org.matrix.custom.html`) | yes | yes (generated) | own, shared tokeniser with CO01 | sanitiser helper |
| XMPP core and IM (RFC 6120, 6121, 7622), XML streams | yes | yes | own XML parser | XMPP helper |
| XEPs: 0030, 0045, 0048/0402, 0054/0292, 0084, 0115, 0163, 0184, 0191, 0198, 0199, 0280, 0308, 0313, 0333, 0352, 0363, 0368, 0393, 0424, 0444, 0454, 0461, 0077, 0377, 0388 (SASL2), 0386 (Bind2), 0484 (FAST) | yes | yes | own | XMPP helper |
| OMEMO (XEP-0384), X3DH, double ratchet | yes | yes | section 6.5 | crypto helper |
| Jingle (XEP-0166, 0167, 0176, 0320, 0353, 0215) | yes | yes | own signalling | XMPP helper (signalling), call engine (media) |
| IRC (RFC 1459, 2812 as practised), IRCv3 (`cap-notify`, `sasl`, `server-time`, `message-tags`, `batch`, `chathistory`, `echo-message`, `labeled-response`, `away-notify`, `account-tag`, `multi-prefix`, `sts`) | yes | yes | own | IRC helper |
| Matrix VoIP (`m.call.*`), MatrixRTC (Phase 3) | yes | yes | own signalling | Matrix helper, call engine |
| SDP, ICE (RFC 8445), STUN, TURN, DTLS 1.2/1.3, SRTP, RTP, RTCP | yes | yes | section 6.5 | call engine |
| Opus, VP8, VP9, AV1 | yes | yes | codecs from the G-11 pipeline (section 6.5) | call engine, media helper |
| JPEG, PNG, GIF, WebP, AVIF, APNG | yes | JPEG, PNG, WebP | system decoders shared with `svc.thumbnail` | media helper |
| Ogg Opus (voice messages) | yes | yes | G-11 | media helper |
| Megolm key export file | yes | yes | own | crypto helper |
| Export HTML, text, JSON | — | yes | own | app host |
| AEGIS client transport (Sphinx packets, paced sending) | via SE03 | via SE03 | ANONYMOUS repo crates | SE03 process |

### 6.4 Integration

- **Intents handled**: `compose` (chat with an identity), `share` (text and files into a room chosen in a picker), `open`/`view` for the URL schemes. **Emitted**: `open`, `view`, `share`, `pick` (CO03 contact, file), `scan` (ME04, QR verification), `compose` (CO01 for an e-mail identity).
- **URL schemes**: `matrix:`, `xmpp:`, `irc:`, `ircs:`; `https://matrix.to/#/…` links are parsed locally when passed by another app.
- **Services used**: `svc.accounts`, `svc.keyring`, `svc.notify`, `svc.settings`, `svc.intent`, `svc.portal.file`, `svc.portal.mic`, `svc.portal.camera`, `svc.portal.screen`, `svc.media`, `svc.power`, `svc.search` (room names only, optional), `svc.thumbnail`, `svc.backup`, `svc.audit`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y`, `svc.model` (optional).
- **Services offered** (Proposed): `org.pbsd.messenger.share` (target of the share sheet with a room picker), `org.pbsd.messenger.call` (start a call with an identity; always shows the call window first).
- **Other apps**: CO03 (`lookup`, `picker`, `trust`), SE03 (routes; mixnet adapter), SE04 (recovery key safekeeping, client certificates, OpenPGP keys for XEP-0373), SE05 (signing exports), IN01 (SSO page, disposable link opening), ME04 (QR scan), ME03 (owner of G-11), AX04 (emoji data), CO02 (join links), IN04 (location).
- **Backing repos**:
  - Verified: `videoWidget` is a frame-source stub (PLATFORM.md V3 from `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp`).
  - Verified: FORGE has Ed25519, XChaCha20-Poly1305, BLAKE2b and Argon2id through Monocypher and lacks SHA-256, AES, HMAC-SHA-256, X.509 and TLS (stated in `Apps/00-foundation/build-plans/FORGE_BUILD_PLAN.md`, FORGE-WP-22; AES and HMAC are not listed there as present).
  - Verified from `ANONYMOUS:README.md`: AEGIS is a metadata-hiding mixnet for a permissioned consortium; Sphinx packets with constant-rate sending; an `aegis-client` crate exists (`ANONYMOUS:crates/aegis-client/src/`: `session.rs`, `send.rs`, `transport.rs`, `tcp_transport.rs`, `padding.rs`, `path.rs`); the README states it is "not interactive; multi-second latency is inherent", that strong guarantees hold for internal client-to-client traffic and clearnet exit is weaker, and that TEE attestation is deferred. No Tor implementation was seen in that repo; the `tor` route name comes from PLATFORM.md section 6 only.
  - Proposed: everything else in this spec.

### 6.5 What must be built or imported (stated plainly)

Nothing below exists in FUSE, FORGE or PBSD today. Licences are from memory and are **Unverified**; each must be checked before vendoring. Every imported library must build offline on HardenedBSD and run without writable-executable memory (assembly paths are fine; run-time code generators are not).

**End-to-end encryption.**

| Need | Option A: own C++23 | Option B: import |
|---|---|---|
| Primitives missing from FORGE: AES-256 (CBC, CTR, GCM), SHA-256, HMAC-SHA-256, HKDF, Curve25519 exposed for ECDH | Add to FORGE (FORGE-WP-22.1 already plans SHA-256; AES and HMAC must be added) | A vetted primitive library chosen by FORGE-WP-22 |
| Olm and Megolm, SAS, cross-signing, SSSS, backup | Write from the published specifications; test against the public test vectors and against a reference implementation | vodozemac (Rust, Apache-2.0) with matrix-sdk-crypto (Rust, Apache-2.0) as the state machine. PLATFORM.md section 2 allows Rust helpers behind IPC. The old C library libolm is deprecated by its authors and is not an option. |
| OMEMO: X3DH, double ratchet | Write from the Signal specifications (public documents) on the same primitives | libsignal (Rust, AGPL-3.0) or a smaller ratchet crate; AGPL is compatible with this project's licence |

Recommendation: Option B for Matrix crypto (the state machine is large and security-critical, and a maintained audited implementation exists), with the Rust toolchain pinned and vendored as the ANONYMOUS work already requires; own C++23 for OMEMO on FORGE primitives only if no acceptable crate is found. This is a decision for the owner; see section 13.

**Real-time media (the WebRTC-equivalent stack).** Calls need all of these parts; none exist:

| Part | What it does | Build or import |
|---|---|---|
| ICE agent, STUN, TURN client | Finds a path between peers, relays when needed | Own C++23 (bounded protocols, good T2 candidates) or libnice (LGPL-2.1, needs GLib: rejected) or libjuice (MPL-2.0, small, C) |
| DTLS 1.2/1.3 with `use_srtp` | Key agreement for media | From the TLS library FORGE-WP-21 selects, if it has DTLS; otherwise a second library |
| SRTP and SRTCP | Media encryption | libsrtp (BSD-3-Clause) or own on FORGE AES |
| RTP, RTCP, NACK, PLI, FEC, RTX | Packetisation and loss recovery | Own |
| Jitter buffer, lip sync | Smooth playout | Own |
| Congestion control | Bandwidth estimate (transport-wide feedback, GCC or similar) | Own, from the published drafts |
| Audio processing | Echo cancellation, noise suppression, gain control | Hardest part to write well. Import the audio-processing module extracted from the WebRTC project (BSD-3-Clause) or speexdsp (BSD-3-Clause, weaker echo canceller) |
| Audio codec | Opus | libopus (BSD-3-Clause) through G-11 |
| Video codecs | VP8 baseline; VP9 or AV1 optional | libvpx (BSD-3-Clause); dav1d (BSD-2-Clause) to decode AV1; an AV1 encoder is optional. H.264 is not planned (patent licensing). All through G-11. |
| SDP offer and answer | Session description | Own |
| Data channels (SCTP over DTLS) | Not needed for calls | Not built in Phase 2 |
| Capture and render | Microphone, camera, screen, speakers | Device portals and the G-11 pipeline |

Alternatives to assembling the parts: (1) the full libwebrtc code base (BSD-3-Clause): rejected as a whole because of its size, its build system and its bundled dependencies, which do not fit an offline, memory-safe-subset build; (2) a Rust sans-I/O WebRTC library such as str0m (MIT or Apache-2.0): a good fit for the "Rust helper behind IPC" rule, covers ICE, DTLS, SRTP, RTP and SDP, and leaves codecs and audio processing to us. Recommendation: evaluate option (2) plus the extracted audio-processing module and G-11 codecs; fall back to own C++23 parts if the Rust library cannot meet the no-W+X and offline rules.

**Group calls** additionally need a selective forwarding unit on the server side (not ours to ship) and per-frame encryption in the client. Phase 3.

**Anonymous transport.** Tor and VPN routing are SE03's and the network service's work; Messenger only selects the route. The AEGIS mixnet is a different thing: it suits text between members of one deployment and cannot carry calls (latency) or reach ordinary homeservers with full strength (the README says clearnet exit is weaker). The Phase 3 feature is therefore scoped to member-to-member text through SE03.

## 7. Security and privacy

### 7.1 Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | Encrypted history, crypto store, media |
| `fs.portal` | Send and save files, export, key import and export |
| `net.user` | Chat servers the user typed; TURN servers named by those servers |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-account route. A `mixnet` route value is needed in Phase 3 and does not exist in PLATFORM.md section 6 (section 13). |
| `proc.helper:matrix`, `:xmpp`, `:irc`, `:e2ee`, `:msg-safe`, `:media`, `:rtc`, `:fetch` | Helpers in 6.1 |
| `dev.mic` | Calls, voice messages; prompt on first use; indicator while on |
| `dev.camera` | Video calls |
| `dev.screen` | Screen share; the shell picker chooses what is shared |
| `svc.accounts`, `svc.keyring` | Endpoints; tokens, store keys, recovery key hand-off |
| `svc.notify` | Messages, calls, verification requests |
| `svc.media`, `svc.power` | Call session controls; sleep inhibit during calls |
| `svc.intent`, `svc.portal.file`, `svc.portal.mic`, `svc.portal.camera`, `svc.portal.screen` | Hand-offs and device portals |
| `svc.thumbnail`, `svc.search`, `svc.settings`, `svc.backup`, `svc.audit`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y` | Standard services |
| `svc.model`, `ai.local` | Optional translate, summarise, transcription, captions; off by default |
| `bg.run` | Stay connected for notifications and calls |
| `bg.autostart` | Optional; the user turns it on to receive messages after login |

Not requested: `dev.location` (location is typed or picked in IN04), `proc.exec`, `fs.path`, `dev.usb`.

### 7.2 Threat model

Assets: message content, keys and sessions, the contact graph, presence and activity times, the user's IP address, voice and video, files.

| Adversary | Attack | Mitigation |
|---|---|---|
| Homeserver, XMPP server, IRC network (honest but curious, or compromised) | Read content | E2EE on by default for private Matrix rooms and for XMPP chats where the peer supports OMEMO; the UI always states when a room is not encrypted; IRC is marked as never end-to-end encrypted |
| Malicious server | Add a device or swap an identity key to read future messages | Cross-signing; refuse unverified devices of verified users by default; identity-change alert blocks sending; trust recorded in CO03 |
| Malicious server | Withhold, reorder, replay, or lie about room state and members | Megolm ratchet index checks and replay detection; member list of encrypted rooms shown with device trust; history visibility shown; state not treated as authenticated |
| Malicious server | Key backup tampering | Backup key verified against secret storage and cross-signing before use |
| Remote user | Malformed events, HTML, media to exploit a parser | Protocol, sanitiser and media helpers have no capabilities beyond passed descriptors; T2 targets; fuzzing; size and depth limits (event 64 KiB as the spec bounds, nesting 32) |
| Remote user | Spam invites, abusive media, phishing links | Invite controls; no media loaded from invites; link confirmation and real targets; report and ignore |
| Remote user | Malicious file | Dangerous type gate; disposable sandbox; content sniffing; provenance attributes |
| Remote user in a call | Learn the IP address | Relay-only by default for unverified contacts; calls off on anonymous routes |
| Bridge operator | Reads bridged rooms | Bridge banner; bridged rooms never shown as end-to-end encrypted |
| Network observer | Link accounts, learn who talks when | Per-account route and circuit identity; no cross-account connection reuse; typing and receipts off on anonymous routes; mixnet option for member traffic in Phase 3 |
| Link preview | URL leak to a server or site | Off by default; never server-side in encrypted rooms |
| Other local app | Read history or keys | Stores are under `fs.app` and encrypted with keys in `svc.keyring`; the crypto store is opened only by the crypto helper |
| Compromised protocol helper | Read plaintext | It never receives plaintext or keys for E2EE content |
| Compromised sanitiser or media helper | Exfiltrate plaintext it was given | No network, no files, killed after each batch or on limits |
| Device theft, unlocked session | Read everything | Out of scope beyond session lock, per-account lock and panic erase |
| Recipient | Keeps messages past the disappearing timer | Stated honestly: timers bind only cooperating clients |

What is stored: section 6.2. What leaves the machine: protocol traffic to the user's servers and, in calls, media to the peer or the relay. Nothing else; no push gateway, no vendor service.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `view.layout` | enum bubbles/flat | bubbles | Timeline layout |
| `compose.enter_sends` | bool | true | Enter sends, Shift+Enter breaks |
| `privacy.send_typing` | bool per account | true (false on anonymous routes) | Typing notices |
| `privacy.read_receipts` | enum public/private/off | public (private on anonymous routes) | Read receipts |
| `privacy.link_previews` | enum off/local/server_unencrypted | off | Link previews |
| `privacy.strip_metadata` | bool | true | Strip metadata on upload |
| `privacy.invites_from` | enum anyone/contacts/shared_room | anyone | Invite filter |
| `media.auto_download_mib` | int | 5 | Auto-download limit for images |
| `media.unverified_sandbox` | bool | true | Disposable sandbox by default for unverified senders |
| `e2ee.send_policy` | enum all/refuse_unverified_of_verified/verified_only | refuse_unverified_of_verified | Send policy |
| `e2ee.key_requests` | enum own_verified/never | own_verified | Answer key requests |
| `e2ee.backup` | bool | ask at set-up | Server key backup |
| `history.local_retention_days` | int? | none | Local retention |
| `calls.relay_only` | enum always/unverified/never | unverified | Relay-only media |
| `calls.push_to_talk` | key? | none | Push-to-talk key |
| `notify.default` | enum all/mentions/none | DMs all, rooms mentions | Notification level |
| `notify.content` | enum full/sender/none | sender | Notification detail |
| `notify.lock_screen` | enum none/sender | none | Lock-screen detail |
| `irc.allow_plaintext.<network>` | bool | false | Permit a non-TLS IRC connection |
| `search.system_index` | bool | false | Room names to `svc.search` |
| other keys | — | — | `view.room_order` (activity), `view.show_membership_events` (folded), `privacy.presence` (share; hide on anonymous routes), `e2ee.omemo_blind_trust` (true), `history.default_expiry_s` (none), `view.compact`, `view.group_by_account`, `view.timestamps` (hover), `compose.markdown`, `compose.spellcheck`, `media.auto_download_metered` (false), `media.cache_days` (90), `calls.video_codec` (auto), `calls.noise_suppression`, `calls.echo_cancel`, `notify.keywords`, `irc.strip_colours`; each mirrors a feature in section 4 |
| `ai.enabled` | bool | false | Local model features |

## 9. Accessibility and languages

- Each message's a11y name: sender, time, body, then states ("edited", "encrypted, from a verified device", "3 reactions: thumbs up 2, heart 1", "has a thread with 4 replies").
- New messages in the open room are announced politely; mentions assertively; a setting limits announcements to mentions.
- Verification emoji are announced by name in the UI language; the decimal form is offered first for screen-reader users.
- Calls: all controls are labelled buttons with state; incoming call is an assertive alert with Answer and Decline reachable by one key each; optional local captions.
- Sender colours are checked for 4.5:1 contrast against the background and are never the only way to tell senders apart (names are always present in flat layout and at the top of each group in bubbles).
- Animated images and stickers do not autoplay with reduced motion; they show a play button.
- Bidirectional text: each message takes its direction from its content; bubbles mirror by locale; mixed-direction mentions are isolated with Unicode isolates.
- Display names are sanitised: control and bidi-override characters removed, length capped at 100 characters, look-alike of a known contact flagged (FORGE-WP-23.4 confusables).
- Emoji shortcodes and picker search work in the UI language and English.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame with cached room list | under 400 ms |
| Switch room (cached) | under 50 ms to a full timeline frame |
| Timeline scroll with mixed media | 60 fps; row layout under 2 ms |
| Send: key press to local echo | under 16 ms |
| Decrypt 1 000 Megolm events (back-pagination) | under 200 ms |
| Incremental sync apply for 500 events | under 100 ms, off the UI thread (helper plus batched model update) |
| Search 5 000 000 stored events | first results under 300 ms |
| Call: mouth-to-ear latency added by the client | under 150 ms; echo canceller converges under 2 s |
| Call CPU at 640×360 VP8 on a 4-core laptop | under 40% of one core each for encode and decode |
| Memory | app host under 300 MiB with 2 000 rooms; each protocol helper under 150 MiB; call engine under 200 MiB |
| Idle | zero frames; network wake-ups only for protocol keep-alives (Matrix long poll, XMPP ping 5 min, IRC ping) |
| Largest supported | 5 000 rooms per account, rooms of 100 000 members (lazy-loaded), 2 GiB single file upload where the server allows |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit (ForgeScript)**: timeline assembly (edits, redactions, reactions, threads, out-of-order arrival), unread and highlight counts, notification policy evaluation against Matrix push rules, outbox ordering and retry, expiry sweeps with a fake clock.
- **Crypto**: the published Olm and Megolm test vectors; interoperability sessions against a second implementation (recorded transcripts as fixtures); SAS and QR flows as table-driven state tests; OMEMO vectors and recorded sessions with another client; wrong-MAC, replayed-index and unknown-session cases.
- **Protocol conformance**: Matrix helper against a local Synapse or Conduit in a jail, offline; the spec's event validation rules as fixtures; XMPP helper against Prosody and ejabberd in a jail; IRC helper against an Ergo server and scripted transcripts.
- **Sanitiser**: property test that output contains only allowed run types and link schemes; known Matrix client XSS samples must produce inert text.
- **Fuzz**: Matrix JSON events, sync responses, XMPP XML streams, IRC lines and tags, SDP, STUN and TURN messages, RTP and RTCP packets, DTLS records (if own code), Megolm export files, HTML subset.
- **Calls**: loopback call in CI with synthetic audio and video; network impairment (loss 0–20%, jitter, reorder, bandwidth steps) with objective quality scores recorded; ICE with relay-only; interoperability calls against another Matrix client and another XMPP client, run by hand until automated.
- **Privacy**: packet-capture tests proving no traffic outside the selected route, no link fetch with previews off, no direct peer packets in relay-only mode, no connection reuse across accounts.
- **Golden renders**: main window in both layouts, verification steps, security overview, call window, all themes, 200%, RTL.
- **Scripted UI**: the 15 top tasks through the a11y tree.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Message sanitiser filter | Output has only allow-listed run types, attributes and schemes | T1 | PRISM proof on the filter over the typed tree | Formal model of the tree; tokeniser stays T2 |
| Matrix, XMPP, IRC parsers | No runtime errors on any input; bounded depth and size | T2 | PRISM bounded checks, fuzzing | — |
| STUN, TURN, RTP, RTCP, SDP parsers (if own) | No runtime errors | T2 | PRISM, fuzzing | If imported Rust: T3 (safe language, fuzzing), PRISM does not apply |
| Crypto helper (if own C++23) | Memory safety, no undefined behaviour; ratchet never reuses a message key | T2, with T1 for the ratchet index monotonicity | PRISM; vectors | Constant-time behaviour is not provable by PRISM; timing harness only |
| Crypto helper (if imported Rust) | Memory safety by language | T3 | Upstream audits (to be read and cited), fuzzing, vectors | Not C++23; outside PRISM |
| Device trust state machine | A device is `verified` only after a completed SAS or QR flow or a valid cross-signing chain; identity change always clears user verification | T1 | Small state model proved; table tests | Model to be written |
| Send policy gate | No Megolm session is shared with a device the policy excludes | T1 | Proof over the recipient-set function | Must live in provable code (C++23 side) |
| Expiry sweeper | After a sweep no row, index entry or blob of an expired event remains | T3 | Property tests with store inspection | SQLite is T4 |
| Route selection | The app never requests a socket on a route other than the account's | T2 in the app-side wrapper; enforcement is the network service's T1 target (FORGE-WP-21) | Single call site, proof of argument flow | — |
| Call engine media path | No out-of-bounds access in jitter buffer and packetiser | T2 (own) or T3 (imported) | PRISM or fuzzing | Imported codecs are T4 |
| Codecs, audio processing, SQLite | Third-party C or C++ | T4 | Sandboxing, fuzzing upstream | Third-party code |

## 12. Build plan

### Phase 1 — usable core (Matrix text with E2EE)

| ID | Work item | Depends on |
|---|---|---|
| CO05-W01 | Decide the crypto and call-stack sources (section 6.5); licence and offline-build check | Owner decision; FORGE-WP-22 |
| CO05-W02 | Encrypted history store, schema, migrations, FTS | G-07, G-09, FORGE-WP-23.1, FORGE-WP-22 |
| CO05-W03 | Matrix protocol helper: login, sync, send, media, back-pagination | FORGE-WP-21, FORGE-WP-27, FORGE-WP-28 |
| CO05-W04 | Crypto helper: Olm, Megolm, device tracking, attachment encryption | CO05-W01 |
| CO05-W05 | Cross-signing, SAS and QR verification, secret storage, key backup | CO05-W04 |
| CO05-W06 | Main window: rail, room list, timeline, composer (plain Markdown) | G-01, G-02, FUSE-WP-28, `CO05-GAP-1` |
| CO05-W07 | Message sanitiser helper and message view | G-10 (read-only profile), CO01-W06 (shared tokeniser) |
| CO05-W08 | Replies, reactions, edits, deletions, mentions, read state, drafts | CO05-W06 |
| CO05-W09 | Media helper: previews, thumbnails, metadata strip; send and save files; dangerous type gate | `svc.portal.file`, system image decoders |
| CO05-W10 | Notifications, policy, inline reply, background connection | `svc.notify`, `bg.run` |
| CO05-W11 | Account wizard, multi-account, offline outbox, session restore | `svc.accounts`, `svc.keyring` |
| CO05-W12 | Verification UX, encryption state display, undecryptable-message handling | CO05-W05 |
| CO05-W13 | Fixtures, local test servers, fuzzers, goldens | FORGE-WP-35 |

### Phase 2 — full feature

| ID | Work item | Depends on |
|---|---|---|
| CO05-W14 | XMPP helper and core XEPs; MUC; MAM; upload | FORGE-WP-21 |
| CO05-W15 | OMEMO in the crypto helper; trust UI | CO05-W04 |
| CO05-W16 | IRC helper, IRCv3, SASL, slash commands, flat layout | FORGE-WP-21 |
| CO05-W17 | Per-account route, route hygiene, fail-closed, alternate endpoints | SE03, SE02, FORGE-WP-21.4 |
| CO05-W18 | Trust records with CO03; contact lookup; identity-change alert | CO03-W14, CO03-W15 |
| CO05-W19 | Spaces, threads, room creation and settings, explore and join, invites controls | CO05-W06 |
| CO05-W20 | Call engine: ICE, DTLS-SRTP, RTP, jitter buffer, congestion control, audio processing | CO05-W01, G-11, FORGE-WP-21 (UDP capability), `svc.portal.mic` |
| CO05-W21 | Voice calls on Matrix and Jingle; call UI; relay-only mode; call history | CO05-W20 |
| CO05-W22 | Video calls | CO05-W21, `svc.portal.camera`, G-11 video codecs |
| CO05-W23 | Rich composer, emoji and sticker packs, voice messages, spell check | G-10 (edit), G-17, G-11 |
| CO05-W24 | Search UI, media gallery, media viewer, storage management | CO05-W02, G-11 |
| CO05-W25 | Disappearing messages, local retention, export, panic erase | CO05-W02 |
| CO05-W26 | Moderation tools, report, ignore, bridge awareness | CO05-W19 |
| CO05-W27 | Security overview, sessions and devices, send policy | CO05-W05 |
| CO05-W28 | Disposable sandbox open; SSO hand-off | SH04 (disposable domain), IN01 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CO05-W29 | Screen share | `svc.portal.screen`, CO05-W22 |
| CO05-W30 | Group calls (MatrixRTC, frame encryption) | CO05-W22; a server-side forwarding unit run by the user's server |
| CO05-W31 | Mixnet text transport through SE03 | SE03 (AEGIS adapter), a `mixnet` route in the platform |
| CO05-W32 | Local model features: translate, summarise, transcription, captions | G-15, `svc.model` |
| CO05-W33 | Polls, location, scheduled send, policy lists, space administration | CO05-W19 |
| CO05-W34 | QR login, dehydrated devices, key sharing on invite, XEP-0373, ATM | CO05-W05, CO05-W15 |
| CO05-W35 | Log import, server search, admin audit view, bridge limits | CO05-W24 |

## 13. Open questions and risks

1. **Crypto source decision.** Importing the Rust Matrix crypto stack is the fastest safe path but adds a pinned Rust toolchain to the build (already implied by ANONYMOUS and noted under G-04). Writing Olm and Megolm in C++23 keeps one toolchain and PRISM coverage but is a long, high-risk job. Owner to decide before any Messenger work starts.
2. **The real-time stack is the largest unknown in the comms category.** It depends on G-11, on a DTLS-capable TLS choice, on UDP sockets through the network service (FORGE-WP-21.3 says "UDP only with an explicit capability"; no such capability is named in PLATFORM.md section 6), and on an audio-processing module that is realistically imported C++. Calls may slip to after the P1 milestone; text chat must not wait for them.
3. **New platform items this app needs**: a UDP capability name; a `mixnet` value for `net.route`; a disposable-sandbox launch flag; a trusted browser hand-off for SSO before G-14 exists; app-provided service capabilities (for CO03's trust interface).
4. **Disappearing messages on Matrix** have no ratified standard. The feature is honest only if the UI says so; behaviour may need to change when the specification settles.
5. **Sliding sync** specification is still moving; `/sync` v2 is the baseline.
6. **No push gateway** means no notifications when the app is not running. This is by design and must be explained to users.
7. **OMEMO version split** (0.3 against newer) fragments reach; supporting both doubles test work.
8. **Server-side link previews and bridges** leak content by nature; the mitigations are warnings, not prevention.
9. **Anonymous routes and Matrix**: homeservers see account identity and room membership whatever the route; the route hides only the client's network location. The UI must not suggest more.
10. **AEGIS fit**: it is built for a permissioned consortium and member-to-member traffic. Whether a general user will ever have a deployment to join is not known. The Phase 3 feature may be dropped.
11. **Selection across messages and bottom-anchored virtual lists** are FUSE gaps specific to chat timelines (`CO05-GAP-1`, `CO05-GAP-2`).
12. **Unverified**: all third-party licences and library capabilities in section 6.5; SE03's route mechanism; whether FORGE will ship AES and HMAC.
