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
| CO05-F138 | Local network chat | Not planned in this version (would need discovery and listening sockets). Recorded so the decision is explicit. | C | 3 |
