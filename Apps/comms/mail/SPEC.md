<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Mail

By Odin Loch

| | |
|---|---|
| ID | `CO01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.mail` |
| Category | comms |
| Priority | P1 |
| Backing repos | FUSE (widgets, `SqliteModel`), FORGE (ForgeScript, network service plan FORGE-WP-21, crypto plan FORGE-WP-22). No mail code exists in any repo read. |
| Status | Planned. Nothing built. |

## 1. Purpose

Mail is the email client for people who run their own accounts on any standards-based server. It keeps a full offline copy of every account, renders hostile messages safely, and makes encryption and sender authentication visible. It replaces KMail from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- IMAP4rev1/rev2 with IDLE, CONDSTORE, QRESYNC; JMAP Core and Mail; SMTP submission; POP3 as import only.
- OAuth2 where a provider demands it, through a trusted browser hand-off. Password and client-certificate login otherwise.
- Unified inbox, threads, folders and labels, local full-text search, Sieve and local rules.
- Compose in rich text (G-10), plain text and Markdown; signatures, templates, attachments.
- OpenPGP and S/MIME with keys held by SE04; Autocrypt and WKD discovery.
- Safe HTML rendering through a restricted subset renderer (decision in section 6.5).
- Sender authentication display (SPF, DKIM, DMARC, ARC), phishing indicators, tracker stripping.
- Snooze, send later, undo send, unsubscribe helper, mailing-list tools.
- Import and export: mbox, Maildir, EML, PST (read only).
- Per-account network route (clear, VPN, Tor) through SE03. Offline queue.

Out of scope:

| Item | Owner |
|---|---|
| Calendar storage, views, free/busy | CO02 Calendar (Mail hands iTIP parts over) |
| Address book storage, merge, CardDAV | CO03 Contacts (Mail uses its picker service) |
| Task storage | CO04 Tasks ("make task from message" is an intent) |
| Key generation, key storage, certificates, token PINs | SE04 Passwords and Keys |
| Routes, circuits, VPN profiles | SE03 Anonymity |
| Firewall rules, DNS policy | SE02 Firewall |
| Full web rendering of a message | IN01 Web Browser (disposable profile, on request) |
| Viewing PDF, office files, images at full size | OF03, OF04, OF05, ME01 through `svc.intent` |
| Feeds and newsletters as RSS | IN02 Feeds |
| NNTP, Exchange EWS/MAPI, ActiveSync | Not planned. Exchange servers are reached by IMAP/SMTP or JMAP only. |
| Running a mail server | Not planned |

## 3. Users and top tasks

Personas:

- **Rae, daily user.** Two personal accounts and one work account. Wants one inbox and fast triage.
- **Sol, journalist.** Uses OpenPGP, routes one account over Tor, opens attachments from strangers.
- **Ines, administrator.** Lives in mailing lists, writes Sieve, wants keyboard-only use and raw headers.
- **Tam, migrating user.** Arrives with a PST file and 15 years of mbox archives.

| # | Task | Target steps |
|---|---|---|
| T1 | Add an account by typing an address and a password | 3 (address, password, Accept) |
| T2 | Read the next unread message | 1 key (`N`) |
| T3 | Reply and send | 3 (`R`, type, `Ctrl+Enter`) |
| T4 | Archive or delete the open message | 1 key (`E` or `Delete`) |
| T5 | Find a message from a person about a topic | 2 (`Ctrl+K` or `/`, type `from:rae invoice`) |
| T6 | Send an encrypted and signed message to a contact with a known key | 0 extra steps (on by policy) |
| T7 | Open a suspicious attachment safely | 2 (click, "Open in disposable sandbox") |
| T8 | Load remote images for one message | 1 click on the banner |
| T9 | Unsubscribe from a list | 2 (banner button, confirm) |
| T10 | Snooze a message until tomorrow morning | 2 (`H`, pick) |
| T11 | Undo a send | 1 (toast Undo within the window) |
| T12 | Accept a meeting invitation | 1 click (hand-off to CO02) |
| T13 | Create a filter from the open message | 3 (menu, adjust, Save) |
| T14 | Import a PST or mbox archive | 3 (File > Import, pick, Start) |
| T15 | See why a message is flagged as possible spoofing | 1 click on the trust badge |

## 4. Features

Priority: M must, S should, C could. Phase: 1 usable core, 2 full feature, 3 advanced.

### 4.1 Accounts and connection

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F001 | Account wizard | Address, then autoconfiguration, then credentials, then identity. Stores the endpoint in `svc.accounts`, the secret in `svc.keyring`. | M | 1 |
| CO01-F002 | Autoconfiguration | Order: bundled provider table (offline), DNS SRV (RFC 6186, RFC 8314), `autoconfig.<domain>` XML, `.well-known/jmap`. Each network probe is listed before it runs; the user can skip to manual. | M | 1 |
| CO01-F003 | Manual server setup | Host, port, security (implicit TLS, STARTTLS required; no plaintext option except for `localhost`), auth method, per protocol. | M | 1 |
| CO01-F004 | IMAP client | IMAP4rev1 (RFC 3501) and rev2 (RFC 9051). Extensions used when advertised: IDLE, CONDSTORE, QRESYNC, UIDPLUS, MOVE, SPECIAL-USE, LIST-EXTENDED, NAMESPACE, ESEARCH, LITERAL+, COMPRESS=DEFLATE, ID (off by default), ENABLE, UTF8=ACCEPT, OBJECTID, STATUS=SIZE. | M | 1 |
| CO01-F005 | IMAP IDLE | One IDLE connection per watched folder, up to `sync.idle_folders` (default 3: Inbox plus two); re-issued every 25 minutes; falls back to polling. | M | 1 |
| CO01-F006 | CONDSTORE and QRESYNC | Flag changes by `MODSEQ`; expunges by `VANISHED`. Without them, a UID and flag diff per folder. | M | 1 |
| CO01-F007 | JMAP client | RFC 8620 and RFC 8621: `Email/query`, `Email/changes`, `Mailbox/*`, `Thread/*`, `EmailSubmission/*`, `Identity/*`, `VacationResponse/*`, blob upload and download; push by EventSource. | S | 2 |
| CO01-F008 | SMTP submission | Port 465 implicit TLS or 587 STARTTLS (RFC 6409, RFC 8314). Extensions: PIPELINING, 8BITMIME, SMTPUTF8, SIZE, DSN, CHUNKING, REQUIRETLS. | M | 1 |
| CO01-F009 | POP3 import | POP3 over TLS with UIDL. Downloads into a local folder. Option to leave or delete on the server after N days. No POP3 account type beyond this. | S | 2 |
| CO01-F010 | Authentication methods | SASL PLAIN and LOGIN over TLS only, SCRAM-SHA-256(-PLUS), OAUTHBEARER, XOAUTH2, EXTERNAL (client certificate from SE04). CRAM-MD5 and NTLM are refused. | M | 1 |
| CO01-F011 | OAuth2 hand-off | Authorisation code flow with PKCE (RFC 7636) and a loopback redirect (RFC 8252). The consent page opens in IN01 in a disposable profile through `svc.intent`. Mail never sees the password. Refresh token in `svc.keyring`. Client IDs are supplied by the user or by a signed provider table. | M | 2 |
| CO01-F012 | Device authorisation flow | RFC 8628 for providers that allow it, so no browser is needed on this machine. | C | 3 |
| CO01-F013 | TLS policy | TLS 1.2 minimum, 1.3 preferred, system trust store (FORGE-WP-21.5), optional per-account certificate pin, DANE TLSA shown when the resolver validates it. A certificate error stops the connection and shows the chain; no "ignore for ever". | M | 1 |
| CO01-F014 | Per-account route | `route` is `clear`, `vpn:<profile>` or `tor`. Set per account; enforced by the network service, not by Mail. The account row shows the route badge. | M | 2 |
| CO01-F015 | Route isolation | On a `tor` account: no autoconfiguration over clear, no DNS on the clear route, a separate circuit identity per account, EHLO name `localhost.localdomain`, no ID command, UTC `Date` header, no `User-Agent`. | M | 2 |
| CO01-F016 | Multiple identities | Per account: name, address, reply-to, organisation, signature, default key, sent folder. Plus-addressing and catch-all identities generated on reply. | M | 1 |
| CO01-F017 | Aliases from server | JMAP `Identity/get`; for IMAP the user adds them. | S | 2 |
| CO01-F018 | Account health panel | Last sync, quota (RFC 9208), server capabilities, TLS version and cipher, certificate expiry, error log. | S | 2 |
| CO01-F019 | Connection limits | At most `net.max_connections` per account (default 4), back-off with jitter from 5 s to 15 min, honours `[THROTTLED]` and `[UNAVAILABLE]`. | M | 1 |
| CO01-F020 | Offline mode | A header switch and automatic detection. All reads work. Writes queue in the operation journal. | M | 1 |
| CO01-F021 | Local-only account | An account with no server: holds imports and archives. | M | 1 |
| CO01-F022 | Account export and import | Settings without secrets as a signed JSON file; secrets move only through SE04 export. | S | 3 |
| CO01-F023 | Remove account | Removes the endpoint, the cache and the queue after one confirmation that names the unsent items. Server data is untouched. | M | 1 |

### 4.2 Synchronisation and offline store

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F024 | Full offline copy | Every subscribed folder is downloaded in full (headers, bodies, attachments) by default. Policy per folder: full, bodies without attachments over N MiB, headers only, last N days. | M | 1 |
| CO01-F025 | Content-addressed message store | Raw RFC 5322 bytes stored once, named by BLAKE3 hash, zstd-compressed, in `~/.local/share/org.pbsd.mail/blobs/`. Duplicates across folders and accounts share one blob. | M | 1 |
| CO01-F026 | Operation journal | Every user action (flag, move, delete, append, send) is a durable journal row replayed against the server in order. Conflicts resolve server-wins for existence, merge for flags. | M | 1 |
| CO01-F027 | Sync scheduler | Priority: open folder, Inbox, IDLE folders, the rest. Foreground sync never waits behind a bulk download. | M | 1 |
| CO01-F028 | Background sync | With `bg.run`: keeps IDLE open with no window. Respects `svc.power` (pauses bulk on battery saver) and metered networks. | S | 2 |
| CO01-F029 | Folder subscription manager | Tree with check boxes, sizes and message counts from STATUS. | M | 1 |
| CO01-F030 | Special-use mapping | Inbox, Drafts, Sent, Archive, Junk, Trash from SPECIAL-USE or JMAP roles; manual override. | M | 1 |
| CO01-F031 | Server-side search fallback | When a folder is headers-only, body queries go to IMAP SEARCH or JMAP `Email/query` and results are marked "from server". | S | 2 |
| CO01-F032 | Integrity check | Verify blob hashes and index consistency; repair by re-fetching. Runs on demand and after an unclean stop. | S | 2 |
| CO01-F033 | Store encryption | The store sits in the user's encrypted dataset. Option: per-account store key in `svc.keyring` so one account can be locked while the session is open. | S | 3 |
| CO01-F034 | Quota display | Bar in the account row; warning at 90%. | S | 2 |
| CO01-F035 | Sync log | Per-account log of commands (no secrets, no bodies), viewable and exportable. | S | 2 |

### 4.3 Reading

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F036 | Three-pane layout | Folder sidebar, message list, reading pane. Reading pane right, below or off. | M | 1 |
| CO01-F037 | Unified inbox | Virtual folders across accounts: All Inboxes, Unread, Flagged, Drafts, Sent, Snoozed, Scheduled, Outbox. Account colour stripe on each row. | M | 1 |
| CO01-F038 | Conversation threading | By `References` and `In-Reply-To` (JWZ algorithm), fall back to normalised subject within 30 days. Thread spans folders (Sent included). JMAP `threadId` or IMAP `THREADID` used when present. | M | 1 |
| CO01-F039 | Conversation view | Messages stacked, read ones collapsed, quoted text folded, per-message actions. | M | 1 |
| CO01-F040 | Message list columns | Flags, attachment, encryption and signature state, from/to, subject, snippet, date, size, account, labels. Column chooser, sort, group by date, sender or thread. | M | 1 |
| CO01-F041 | Density and preview lines | Compact (one line), normal (two), comfortable (three with snippet). | S | 1 |
| CO01-F042 | Read state rules | Mark read: at once, after N seconds, or only by hand. | M | 1 |
| CO01-F043 | Flags and keywords | `\Seen`, `\Flagged`, `\Answered`, `$Forwarded`, `$Junk`, `$NotJunk`, `$MDNSent`, plus user keywords as coloured labels. | M | 1 |
| CO01-F044 | Plain-text view | `format=flowed` (RFC 3676) reflow, quote levels as coloured bars, links detected but never auto-opened, signature dimmed after `-- `. | M | 1 |
| CO01-F045 | Safe HTML view | Restricted renderer (section 6.5): no script, no forms submission, no plugins, no remote fetch, no CSS that hides or positions over the chrome. | M | 1 |
| CO01-F046 | Prefer plain text | Per account, per sender or global: show `text/plain` when present. A switch in the reading pane toggles parts. | M | 1 |
| CO01-F047 | Remote content control | Blocked by default. Banner: "Load images once", "Always for this sender", "Always for this domain". Loads go through the fetch proxy (section 4.7). | M | 1 |
| CO01-F048 | Tracker stripping | Remove 1×1 and hidden images, known tracker hosts (offline list), and tracking query parameters on links. The banner states the count. | M | 2 |
| CO01-F049 | Link safety | Hover and focus show the real target. Mismatch between link text and target, punycode and mixed-script hosts are flagged. Click asks once per host, then opens through `svc.intent`. | M | 1 |
| CO01-F050 | Inline images | `cid:` parts decoded in the image helper and shown inline. `data:` images under 2 MiB allowed. | M | 1 |
| CO01-F051 | Dark-mode adaptation | Recolour message styles for the dark theme while keeping contrast; a toggle shows original colours. | S | 2 |
| CO01-F052 | Headers view | Summary, full headers, raw source (`Ctrl+U`) with MIME tree. | M | 1 |
| CO01-F053 | MIME structure panel | Tree of parts with type, size, encoding, disposition; save any part. | S | 2 |
| CO01-F054 | Zoom and reader width | Per message zoom; max line width for plain text. | S | 1 |
| CO01-F055 | Translate and summarise | Through `svc.model` with `ai.local`, on request only, shown in a side panel marked "machine output". Off by default. | C | 3 |
| CO01-F056 | Read receipts | MDN (RFC 8098) requests are never answered automatically. Options: ask, never. | M | 2 |
| CO01-F057 | Print | Message or thread to `svc.portal.print`, headers selectable, remote content never fetched for print. | S | 2 |
| CO01-F058 | Open in new window or tab | Message, thread or compose in tabs under the header bar; tear off to a window. | S | 1 |
| CO01-F059 | Attachment strip | Chips with name, type from content sniff, size, and a warning when extension and content disagree. | M | 1 |
| CO01-F060 | Attachment preview | Thumbnail by `svc.thumbnail`; quick look in the preview helper sandbox for images, PDF first page, text. | M | 2 |
| CO01-F061 | Open in disposable sandbox | Hands a read-only handle to the target app started in a throw-away capability domain with `net.none` and no access to user files. Default for unknown senders. | M | 2 |
| CO01-F062 | Save attachments | One, all, or all as a folder, through `svc.portal.file`. Saved files carry provenance (sender, message ID, hash) as extended attributes. | M | 1 |
| CO01-F063 | Dangerous type gate | Executables, scripts, bundles, shortcuts, macro documents: never opened directly; only "Save" and "Open in disposable sandbox". | M | 1 |
| CO01-F064 | Detach and delete attachment | Replace an attachment by a stub note in the stored message (new message appended, old removed). | C | 3 |

### 4.4 Organising

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F065 | Folder operations | Create, rename, move, delete, subscribe, empty, mark all read; drag to reorder favourites. | M | 1 |
| CO01-F066 | Labels | IMAP keywords and JMAP keywords shown as labels; Gmail `X-GM-LABELS` mapped when the extension is present. Colour and shortcut `1`–`9` per label. | S | 2 |
| CO01-F067 | Archive | One key. Target: one Archive folder, or per year, or per year and month. | M | 1 |
| CO01-F068 | Move and copy | Picker with type-ahead (`V` move, `Shift+V` copy); drag to a folder; across accounts by fetch and append. | M | 1 |
| CO01-F069 | Delete and undo | To Trash with a toast Undo. `Shift+Delete` expunges after confirmation. | M | 1 |
| CO01-F070 | Junk handling | Mark junk moves to Junk and sets `$Junk`. Local classifier (naive Bayes over tokens, per account, trained by the user's marks). No network lookups. | S | 2 |
| CO01-F071 | Snooze | Hide until a time: later today, tomorrow, weekend, next week, pick. Stored as keyword `$snoozed` plus a local wake time; moved to a Snoozed folder so other clients agree. | S | 2 |
| CO01-F072 | Pin | Pinned messages stay on top of a folder (`$pinned` keyword). | C | 3 |
| CO01-F073 | Saved searches | Any query saved as a virtual folder in the sidebar. | S | 2 |
| CO01-F074 | Follow-up reminder | "Remind me if no reply in N days" on a sent message; local check against the thread. | S | 3 |
| CO01-F075 | Bulk selection actions | Select all in folder or search result (not only the loaded page); actions run as one journal batch with progress. | M | 1 |
| CO01-F076 | Sender view | All mail from one address or domain, with its key and authentication history. | S | 3 |
| CO01-F077 | Retention rules | Per folder: delete or archive older than N days; preview before it runs; off by default. | C | 3 |

### 4.5 Search

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F078 | Local full-text index | SQLite FTS5 over subject, addresses, body text, attachment names, and text extracted from attachments (PDF, office, plain) by the extraction helper. | M | 1 |
| CO01-F079 | Query language | `from:`, `to:`, `cc:`, `subject:`, `body:`, `has:attachment`, `filename:`, `is:unread/flagged/encrypted/signed/answered`, `in:`, `account:`, `label:`, `before:`, `after:`, `older_than:7d`, `larger:5M`, `list:`, quotes, `OR`, `-`. | M | 1 |
| CO01-F080 | Search field with tokens | Typed operators become chips; suggestions from contacts and folders. | S | 2 |
| CO01-F081 | Quick filter bar | Unread, flagged, has attachment, from contacts, encrypted; applies to the open folder. | M | 1 |
| CO01-F082 | Find in message | `Ctrl+F` in the reading pane and in compose. | M | 1 |
| CO01-F083 | Encrypted mail indexing | Off by default. When on, decrypted text goes to a separate index encrypted with a key in `svc.keyring`. | S | 3 |
| CO01-F084 | System search contribution | Subject, sender and date go to `svc.search` only when `search.system_index` is on (default off). Bodies never. | S | 2 |
| CO01-F085 | Search across archives | Imported and local folders are in the same index. | M | 2 |

### 4.6 Compose and send

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F086 | Compose window | New, reply, reply all, reply to list, forward inline, forward as attachment, redirect (resend), edit as new. | M | 1 |
| CO01-F087 | Plain-text editor | Hard wrap off; `format=flowed` on send; quote prefix handling; top or bottom posting per identity. | M | 1 |
| CO01-F088 | Rich editor | G-10 widget: bold, italic, underline, strike, headings, lists, quote, links, inline images, tables, code, colour, alignment. Output is `multipart/alternative` with a generated plain part. Emitted HTML uses a fixed safe subset with inline styles. | M | 2 |
| CO01-F089 | Markdown compose | Write Markdown, live preview in `markdownView`, send as plain plus rendered HTML. | S | 2 |
| CO01-F090 | Recipients | To, Cc, Bcc, Reply-To, From. Completion from the CO03 picker service and from local recent recipients. Chips show key state (lock icon) and a warning for external domains. | M | 1 |
| CO01-F091 | Address checks | Syntax (RFC 5322, SMTPUTF8), look-alike of a known contact, reply-all to a large list, Bcc to self. Warnings are inline, not modal. | S | 2 |
| CO01-F092 | Attachments in compose | Add by portal, drag, paste. Size bar against the server SIZE limit. Images can be scaled and stripped of metadata (calls the SE05 metadata cleaner helper by intent). | M | 1 |
| CO01-F093 | Forgotten-attachment check | Body mentions an attachment (localised word list) but none is present: inline prompt before send. | S | 2 |
| CO01-F094 | Signatures | Per identity, plain and rich, placement above or below the quote, `-- ` separator. | M | 1 |
| CO01-F095 | Templates and quick replies | Templates with variables (`{first_name}`, `{date}`, `{subject}`); inserted from a menu or `;name` expansion. | S | 2 |
| CO01-F096 | Drafts | Autosave every 10 s locally and to the server Drafts folder every 60 s (encrypted drafts are encrypted to self). Crash restore reopens all compose windows. | M | 1 |
| CO01-F097 | Undo send | Hold in Outbox for `send.undo_seconds` (default 10, 0 to 120). Toast offers Undo. | M | 1 |
| CO01-F098 | Send later | Pick a time. Held locally in Scheduled; sent when due if the app runs (`bg.run`). Uses JMAP `EmailSubmission` `sendAt` or SMTP FUTURERELEASE when offered so the server can hold it. | S | 2 |
| CO01-F099 | Offline outbox | Messages queue with their attachments and are sent in order on reconnect. Failures stay with the server reply shown. | M | 1 |
| CO01-F100 | Sent copy | Append to Sent by IMAP APPEND, skip when the server does it (Gmail, JMAP). BURL not used. | M | 1 |
| CO01-F101 | Header hygiene | `Message-ID` on the identity domain with a random left part; no `User-Agent`, no `X-Mailer`, no local hostname or IP in headers; `Date` in UTC when `privacy.utc_date` is on. | M | 1 |
| CO01-F102 | Spell check | Through the G-17 service; language per message; ignores quoted text. | S | 2 |
| CO01-F103 | Mail merge | Template plus a recipient list from CO03 group or CSV; one message per recipient; dry-run preview. | C | 3 |
| CO01-F104 | Delivery status | DSN requests optional; bounces matched to the sent message and shown on it. | S | 3 |
| CO01-F105 | Vacation responder | JMAP `VacationResponse` or a generated Sieve `vacation` script; dates, subject, text. | S | 3 |
| CO01-F106 | Compose assist | Local model rewrite, shorten, tone, through `svc.model`; on request; diff shown before apply. | C | 3 |

### 4.7 Encryption and signing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F107 | OpenPGP | PGP/MIME (RFC 3156) sign, encrypt, verify, decrypt. Inline PGP read only. All private-key operations happen in the SE04 engine; Mail holds key handles, never key bytes (FORGE-WP-22.2, 22.4). | M | 2 |
| CO01-F108 | S/MIME | CMS sign, encrypt, verify, decrypt (RFC 8551); AES-256-GCM AuthEnveloped preferred; certificates and chains from SE04; revocation by stapled or cached CRL, OCSP only if the user enables network checks. | S | 3 |
| CO01-F109 | Protected headers | Subject and other headers encrypted (draft "Header Protection for Cryptographically Protected E-mail"); outer subject `...`. | S | 3 |
| CO01-F110 | Autocrypt Level 1 | Parse and send `Autocrypt:` headers, peer state per address, recommendation engine (disable, discourage, available, encrypt), setup message import. | S | 2 |
| CO01-F111 | Key discovery | Order: CO03 contact keys, SE04 keyring, Autocrypt state, WKD (direct and advanced), keys.openpgp.org (off by default). Network lookups use the account's route and are shown before they run. | S | 2 |
| CO01-F112 | Fetch proxy | All non-mail HTTP (remote images, WKD, autoconfig, unsubscribe, OAuth token) goes through one helper with no cookies, no referrer, a fixed generic user agent, the account's route, and a first-contact log entry. | M | 1 |
| CO01-F113 | Trust display | Per message: signed by a verified key, signed by an unverified key, bad signature, encrypted, mixed parts. Icon plus word; click opens details with fingerprint and the CO03 trust state. | M | 2 |
| CO01-F114 | Encryption policy | Per account and per contact: always, when possible (opportunistic), ask, never. Compose shows which recipients block encryption. | M | 2 |
| CO01-F115 | Partial-encryption defence | Decrypted parts are rendered in isolation; unencrypted parts around them are marked; no HTML may span the boundary (defence against exfiltration by quoting, "EFAIL" class). | M | 2 |
| CO01-F116 | Attach public key and key requests | Attach own key; import keys from attachments into SE04 after a fingerprint review. | S | 2 |
| CO01-F117 | Decrypt for storage | Option to store a decrypted copy locally in the encrypted store, so search works without the token present. Off by default. | C | 3 |
| CO01-F118 | Hardware tokens | Signing and decryption on a token through SE04; Mail shows "touch your key" state. | S | 3 |
| CO01-F119 | Symmetric password mail | Not supported for send. Received OpenPGP symmetric messages can be decrypted with a typed passphrase. | C | 3 |

### 4.8 Authentication, phishing and privacy indicators

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F120 | Authentication-Results display | Parse RFC 8601 headers added by the account's own server (trusted `authserv-id` list per account). Show SPF, DKIM, DMARC and ARC results with icon and word. | M | 2 |
| CO01-F121 | Local DKIM verification | Optional: verify DKIM signatures locally with a DNS lookup over the account's route (off by default; leaks sender domains to the resolver). | C | 3 |
| CO01-F122 | Spoofing indicators | Display name contains a different address; From domain differs from DKIM `d=`; Reply-To on another domain; first-time sender; look-alike of a contact (confusables table, FORGE-WP-23.4); sender claims to be you. | M | 2 |
| CO01-F123 | Known-sender badge | In contacts, replied to before, or key verified. | S | 2 |
| CO01-F124 | External banner | Optional banner on mail from outside the account's domains. | C | 3 |
| CO01-F125 | Phishing explanation | The trust badge opens a sheet listing each check, its result and the raw header it came from. | M | 2 |
| CO01-F126 | Unsubscribe helper | `List-Unsubscribe` and `List-Unsubscribe-Post` (RFC 8058): one-click POST through the fetch proxy, or a `mailto:` draft. Never follows a body link automatically. | S | 2 |
| CO01-F127 | Privacy report | Per message: remote resources blocked, trackers removed, links rewritten, with hosts listed. | S | 3 |

### 4.9 Filters and rules

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F128 | Local rules | Conditions on headers, addresses, size, age, list ID, authentication result, attachment type; actions move, copy, label, flag, mark read, delete, forward, notify, stop. Run on arrival, on send, or by hand. | M | 2 |
| CO01-F129 | Sieve editor | RFC 5228 with `fileinto`, `envelope`, `imap4flags`, `vacation`, `variables`, `body`, `date`, `regex` when offered. Form editor and text editor (`codeEdit` with Sieve grammar); syntax checked locally. | S | 2 |
| CO01-F130 | ManageSieve | RFC 5804: list, get, put, activate, check scripts; JMAP Sieve (RFC 9661) when offered. | S | 2 |
| CO01-F131 | Rule from message | Pre-filled from the open message. | S | 2 |
| CO01-F132 | Rule test | Run a rule against a folder as a dry run and list what would change. | S | 3 |
| CO01-F133 | Rule scripts | ForgeScript rules in the app sandbox with a narrowed capability set (no `net`, no `fs`); signed by the user's key before they run. | C | 3 |

### 4.10 Lists, invitations, contacts, hand-offs

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F134 | Mailing-list tools | `List-Id`, `List-Post`, `List-Archive`, `List-Help`, `List-Owner` shown as actions; reply to list; mute thread (`$muted`); per-list folder rule in one click. | S | 2 |
| CO01-F135 | Patch mail view | Detect `git format-patch` mail: monospace, diff colouring, "save as mbox", hand-off to DV03. | C | 3 |
| CO01-F136 | Invitation hand-off | `text/calendar` parts (iMIP, RFC 6047) are summarised by CO02's iCalendar helper and shown as a card: Accept, Tentative, Decline, Open in Calendar. Replies are composed by CO02 and sent by Mail. | M | 2 |
| CO01-F137 | Contact integration | Sender card with photo, key and trust state from CO03; "Add to contacts"; optional interaction history record sent to CO03. | M | 2 |
| CO01-F138 | Make task or note | Intents to CO04 and OF02 with a `mid:` link back to the message. | S | 2 |
| CO01-F139 | `mailto:` and `mid:` | Handles `mailto:` (RFC 6068) with a field allow-list (to, cc, bcc, subject, body; `attach` ignored) and `mid:` links from other apps. | M | 1 |
| CO01-F140 | Share target | Other apps share files to Mail by the `share` and `compose` intents. | M | 1 |
| CO01-F141 | vCard and key attachments | `.vcf` to CO03, `.asc`/`.p7c` to SE04, with a review step. | S | 2 |

### 4.11 Import, export, backup

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F142 | Import mbox | mboxo, mboxrd, mboxcl, mboxcl2; streamed; files over 100 GiB. | M | 2 |
| CO01-F143 | Import Maildir | Maildir and Maildir++ with flags from file names. | M | 2 |
| CO01-F144 | Import EML | Files and folders of `.eml`; drag onto a folder. | M | 1 |
| CO01-F145 | Import PST and OST | Read only, in the import helper (own reader of the documented MS-PST format; ANSI and Unicode; compressible and high encryption). Folders, messages, attachments; calendar and contact items are handed to CO02 and CO03. | S | 3 |
| CO01-F146 | Import from other clients | Thunderbird profile (mbox plus `msgFilterRules.dat`), KMail Maildir, Apple Mail `.emlx`. | C | 3 |
| CO01-F147 | Export | Message as EML; selection or folder as mbox (mboxrd) or Maildir; thread as PDF through print. | M | 2 |
| CO01-F148 | Backup rules | `svc.backup` includes the store and index definitions, excludes cache and rebuildable index data. | M | 2 |
| CO01-F149 | Server-to-server copy | Copy a folder tree from one account to another with progress and resume. | C | 3 |

### 4.12 Notifications, settings, recovery, accessibility

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO01-F150 | New-mail notifications | `svc.notify` with sender and subject (content hidden on the lock screen by default); actions Archive, Delete, Mark read, Reply. Per folder and per sender rules; VIP list. | M | 1 |
| CO01-F151 | Badge count | Unread in chosen folders. | S | 1 |
| CO01-F152 | Quiet hours | Follows system do-not-disturb; per-account schedule. | S | 2 |
| CO01-F153 | Command palette | Every action, folder, account and saved search. | M | 1 |
| CO01-F154 | Keyboard sets | Default single-key set, and optional Gmail-like and mutt-like sets; all rebindable. | S | 2 |
| CO01-F155 | Session restore | Open folders, tabs, drafts and scroll positions return after a restart or crash. | M | 1 |
| CO01-F156 | Store recovery | Journal replay after power loss; index rebuild from blobs; full re-sync as last resort without losing local-only folders. | M | 1 |
| CO01-F157 | Audit entries | First contact with each host, OAuth grants, key imports, remote-content loads and sandbox opens are written to `svc.audit`. | M | 2 |
| CO01-F158 | Screen reader reading mode | Linear reading order of a thread with headings per message; quoted text skippable. | M | 2 |
| CO01-F159 | High-contrast message view | Forces theme colours on message content. | S | 2 |
| CO01-F160 | Diagnostics bundle | Redacted logs and capability lists saved to a file the user chooses; nothing is sent. | S | 3 |

## 5. GUI

### 5.1 Window map

```
Mail
├─ Main window (three-pane)
│  ├─ Header bar (menu, title, search field, offline switch, [net] badge) and tab strip
│  ├─ Sidebar: unified folders, accounts and folder trees, labels, saved searches
│  ├─ Message list: toolbar, quick filter bar, list
│  ├─ Reading pane: banners, header block, trust badge, body, attachment strip
│  ├─ Inspector (F4): sender card, thread participants, privacy report, MIME tree
│  └─ Status bar: sync state, outbox, quota, route
├─ Compose window (also as a tab): recipients, subject, security bar, toolbar, editor, attachments
│  └─ Sheets: send later, encryption problems, forgotten attachment, template picker
├─ Message window (detached message or thread) · Raw source window (headers, MIME tree, bytes)
├─ Dialogs: account wizard (4 pages) · account settings (General, Servers, Identities, Sync,
│  Security, Route, Folders) · rules editor and rule test results · Sieve editor ·
│  folder subscriptions · import wizard · export · key and certificate details (read-only;
│  edit opens SE04) · preferences
├─ Sheets and popovers: trust and authentication sheet · remote content sheet · link
│  confirmation · move-to-folder picker · snooze · unsubscribe confirmation · invitation card
└─ Shell-drawn: capability prompts, file portal, print dialog, notifications
```

### 5.2 Main window

Shown at 1280×800, wide layout.

```
+------------------------------------------------------------------------------------------+
| [=] Mail — Inbox (All)        [/ Search mail…                 ] (1)  [Offline o] [net] _ ▢ ✕|
+------------------------------------------------------------------------------------------+
| [Mailbox] [Re: Budget ✕] [New message ✕]                                           (2)   |
+--------------------+------------------------------+--------------------------------------+
| [Compose]      (3) | [Archive][Delete][Move v][⚑] | ! Remote images blocked (4 trackers) |
|                    | [Unread][⚑][📎][Contacts](5) | [Load once] [Always for sender] (8)  |
| ALL ACCOUNTS       +------------------------------+--------------------------------------+
| > Inbox        12  | ● Rae Okafor          09:14  | Budget for Q1                    (9) |
|   Unread       31  |   Budget for Q1          📎🔒 | Rae Okafor <rae@example.org>         |
|   Flagged       4  |   Hi, attached is the…      | to me, Sol            Tue 09:14      |
|   Drafts        2  | ----------------------------| [✔ Signed · verified key] (10)       |
|   Snoozed       1  |   Lists · pbsd-dev (3) 08:50 | [SPF pass · DKIM pass · DMARC pass]  |
|   Outbox        0  |   [PATCH 2/5] vm: fix…       +--------------------------------------+
|                    | ----------------------------| Hi,                                  |
| rae@example.org(4) | ⚠ "IT Support"       Mon     |                                      |
|   Inbox         9  |   Password expiry notice     | Attached is the draft. Can you check |
|   Archive          |   Your password will…   (6)  | section 3 before Friday?             |
|   Lists         5  | ----------------------------|                                      |
|   Junk             |   Sol Marsh           Mon    | > On Mon, Sol wrote:   [… 12 lines]  |
| work (tor) [tor]   |   Re: Interview notes    🔒   |                                (11)  |
|   Inbox         3  |                              +--------------------------------------+
|                    |                         (7)  | [q1-budget.ods 84 KiB v] [Save all]  |
| LABELS             |                              |                                (12)  |
|   ● urgent         |                              | [Reply] [Reply all] [Forward] [⋯]    |
+--------------------+------------------------------+--------------------------------------+
| Synced 09:15 · Outbox 0 · rae@example.org 41% of 10 GiB · route: clear         (13)       |
+------------------------------------------------------------------------------------------+
```

1. Search field with operator chips; `/` focuses it. Also the command palette on `Ctrl+K`.
2. Tab strip. The first tab is the mailbox. Messages and drafts open as tabs; a tab can be dragged out to a window.
3. Primary action, filled accent. `C` or `Ctrl+N`.
4. Account section header with route badge (`[tor]`, `[vpn]`) in `--net` colour. Collapsible.
5. Quick filter bar (toggle chips).
6. Row with a spoofing warning icon; the icon has a word in the tooltip and in the a11y name.
7. Message list, virtualised, grouped by thread; 320 px wide by default, splitter on both sides.
8. Banners stack above the header block: remote content, encryption state, invitation, unsubscribe, snooze, "from server" search result.
9. Header block: subject, sender chip (opens sender card), recipients, date. `Ctrl+U` opens the raw source.
10. Trust badges. Click opens the trust sheet (5.3.3).
11. Body. Plain text in a `markdownView`-like read-only text view, HTML in the mail document view (section 6.5). Folded quote shows a line count.
12. Attachment strip. Each chip has a menu: Preview, Open in disposable sandbox, Open with, Save.
13. Status bar: sync state, outbox count (click opens Outbox), quota, the selected account's route.

Reading pane position: right (default above 1100 px), below (640–1100 px), off (list only; Enter opens a tab).

### 5.3 Other views and dialogs

#### 5.3.1 Compose window (900×700)

```
+------------------------------------------------------------------------------------------+
| [=] New message                                                           [net] _ ▢ ✕    |
+------------------------------------------------------------------------------------------+
| [Send ▾] [Attach] [Template v] [Sign ✔] [Encrypt 🔒 ✔]  Format: (•) Rich ( ) Plain ( ) MD |
+------------------------------------------------------------------------------------------+
| From    [Rae Okafor <rae@example.org> v]                                    (1)          |
| To      [Sol Marsh 🔒 ✕] [ines@example.net ⚠ no key ✕] [____________]       (2) [Cc Bcc] |
| Subject [Budget for Q1_____________________________________________]                    |
+------------------------------------------------------------------------------------------+
| 🔒 Cannot encrypt: no key for ines@example.net. [Find key] [Send unencrypted] [Remove]  (3)|
+------------------------------------------------------------------------------------------+
| [B][I][U][S] [H v] [• list][1. list] [" quote] [link] [image] [table] [</>] [Aa]   (4)   |
+------------------------------------------------------------------------------------------+
| Hi Sol,                                                                                  |
|                                                                                          |
| Please check section 3.                                                             (5)  |
|                                                                                          |
| --                                                                                       |
| Rae                                                                                      |
+------------------------------------------------------------------------------------------+
| [q1-budget.ods 84 KiB ✕] [+]                         84 KiB of 25 MiB  ▓░░░░░░░░   (6)   |
+------------------------------------------------------------------------------------------+
| Draft saved 09:21 · English (AU) · route: clear                                          |
+------------------------------------------------------------------------------------------+
```

1. Identity picker; changing it changes signature, key and sent folder.
2. Recipient chips with key state. Completion popover lists contacts, then recent addresses.
3. Security bar: one line stating what will happen and what blocks it.
4. Format toolbar (hidden in Plain mode).
5. Editor: G-10 rich text, or `codeEdit` in plain and Markdown modes.
6. Attachment strip with a size bar against the server limit.

`Send ▾` menu: Send (`Ctrl+Enter`), Send later…, Send and archive thread, Save draft. States: offline (button reads "Queue"), sending, error (banner with server reply), read-only (sent copy opened as new).

#### 5.3.2 Account wizard (640×520)

```
+--------------------------------------------------------------+
| Add a mail account                                 Step 2 / 4|
+--------------------------------------------------------------+
| Address   [rae@example.org___________]                       |
|                                                              |
| Found settings from: DNS SRV records                    (1)  |
|   Incoming  IMAP  imap.example.org : 993  TLS                |
|   Outgoing  SMTP  smtp.example.org : 465  TLS                |
|   Sign-in   Password (SCRAM-SHA-256)                         |
|   [Edit manually]                                            |
|                                                              |
| Network route  (•) Clear  ( ) VPN [profile v]  ( ) Tor  (2)  |
|                                                              |
| Lookups made: _imaps._tcp.example.org (DNS)             (3)  |
|                                                              |
|                                   [Back]  [Cancel]  [Next]   |
+--------------------------------------------------------------+
```

1. Source of the settings is always stated. 2. Route is chosen before any further contact. 3. Every lookup made is listed. Pages: Address, Servers and route, Sign in (password, OAuth hand-off or certificate), Identity and sync policy. States: probing, not found (manual form), certificate error (chain shown, stop), wrong password, OAuth waiting for the browser.

#### 5.3.3 Trust and authentication sheet (520×460)

```
+----------------------------------------------------------+
| Is this message really from rae@example.org?             |
+----------------------------------------------------------+
| ✔ Signature   Good, key 4F2A … 9C1D, verified by you      |
| ✔ DKIM        pass, d=example.org                        |
| ✔ SPF         pass, mailfrom example.org                 |
| ✔ DMARC       pass, policy reject                        |
| – ARC         none                                       |
| ✔ Sender      In your contacts, 41 earlier messages      |
| ✔ Links       3 links, all match their text              |
+----------------------------------------------------------+
| Results come from mx.example.org (your server).     (1)  |
| [Show raw Authentication-Results]  [Open key in SE04]    |
|                                               [Close]    |
+----------------------------------------------------------+
```

1. States which server asserted the results. If the header came from an untrusted `authserv-id`, rows read "not checked". Failure rows use `--danger` with a cross and a one-line reason.

#### 5.3.4 Rules editor (760×560)

```
+---------------------------------------------------------------------------+
| Rules — rae@example.org                       [Local rules | Sieve]  (1)  |
+----------------------+----------------------------------------------------+
| [+] [–] [↑] [↓]      | Name [Lists to folder_____________]  [x] Enabled   |
| > Lists to folder    | When  (•) all  ( ) any  of:                        |
|   VIP notify         |  [List-Id v] [contains v] [pbsd-dev______] [–]     |
|   Receipts           |  [+ condition]                                     |
|                      | Do:                                                |
|                      |  [Move to v] [Lists/pbsd-dev v] [–]                |
|                      |  [Mark read v]                  [–]                |
|                      |  [+ action]     [x] Stop processing more rules     |
|                      | Run: [x] on arrival  [ ] on send  [ ] by hand only |
+----------------------+----------------------------------------------------+
| [Test on folder…] (2)                              [Cancel]  [Save]       |
+---------------------------------------------------------------------------+
```

1. Switch between local rules and the server Sieve script. 2. Dry run lists matches without changing anything.

#### 5.3.5 Invitation card (inline in the reading pane)

```
+--------------------------------------------------------------------+
| 📅 Invitation: Budget review                                        |
| Thu 19 Nov 2026, 14:00–15:00 (Australia/Sydney) · Room 4           |
| Organiser: sol@example.org · 4 attendees                           |
| Your calendar: free   (from CO02)                                  |
| [Accept] [Tentative] [Decline]  [Propose time…]  [Open in Calendar]|
+--------------------------------------------------------------------+
```

Import wizard (640×480, no drawing): source picker with the sniffed format and size, target folder, options (keep structure, skip duplicates by hash, hand calendar items to CO02 and contacts to CO03), a determinate progress bar with per-item error list; the job runs in the import helper and survives closing the dialog.

Invitation card states: update to a known event, cancellation, reply from an attendee, CO02 not installed (shows the raw summary and "Save .ics").

### 5.4 Menus

Items are separated by ` · `; the shortcut follows each item in brackets.

```
File      New message [Ctrl+N] · New window [Ctrl+Shift+N] · New folder… [Ctrl+Shift+F] ·
          Open message file… [Ctrl+O] · Save as… [Ctrl+S] · Save attachments… [Ctrl+Shift+S] ·
          Import… · Export selection… · Add account… · Work offline [Ctrl+Shift+O] ·
          Print… [Ctrl+P] · Close tab [Ctrl+W] · Quit [Ctrl+Q]
Edit      Undo [Ctrl+Z] · Redo [Ctrl+Shift+Z] · Cut · Copy · Paste [Ctrl+X/C/V] ·
          Paste as quotation [Ctrl+Shift+V] · Select all [Ctrl+A] · Find in message [Ctrl+F] ·
          Search mail [/] · Preferences [Ctrl+,]
View      Reading pane > Right | Below | Off · Density > Compact | Normal | Comfortable ·
          Threads [Ctrl+T] · Sort by > Date | From | Subject | Size | Unread · Columns… ·
          Sidebar [F9] · Inspector [F4] · Message body > Original HTML | Plain text | Simplified ·
          Load remote content [Ctrl+Shift+I] · Show original colours · Headers > Summary | All ·
          Message source [Ctrl+U] · Zoom in/out/reset [Ctrl++ / Ctrl+- / Ctrl+0] · Full screen [F11]
Go        Next/previous message [J / K] · Next unread [N] · Next/previous thread [] / [] ·
          Go to folder… [G] · Inbox [G then I] · Back/Forward [Alt+Left / Alt+Right]
Message   Reply [R] · Reply all [Shift+R, A] · Reply to list [L] · Forward [F] ·
          Forward as attachment [Shift+F] · Redirect… · Edit as new · Archive [E] ·
          Delete [Delete] · Delete for ever [Shift+Delete] · Move to… [V] · Copy to… [Shift+V] ·
          Mark > Read [M] | Unread [U] | Flagged [S] | Junk [!] | Not junk · Label > [1…9] ·
          Snooze… [H] · Mute thread [Shift+M] · Create rule from message… · Make task ·
          Make note · Unsubscribe
Security  Trust details [Ctrl+I] · Verify signature again · Decrypt and show ·
          Import key from message… · Attach my public key · Open Passwords and Keys
Tools     Rules… · Sieve scripts… · Folder subscriptions… · Account settings… · Sync now [F5] ·
          Sync all [Shift+F5] · Check store integrity · Activity and sync log
Help      Mail help [F1] · Keyboard shortcuts [Ctrl+/] · About Mail
```

Compose window adds: Message (Send `Ctrl+Enter`, Send later `Ctrl+Shift+Enter`, Save draft `Ctrl+S`, Discard), Insert (Attachment `Ctrl+Shift+A`, Image, Link `Ctrl+L`, Table, Template, Signature, Public key), Format (text styles, `Ctrl+B/I/U`, Quote level `Ctrl+]`/`Ctrl+[`, Rich/Plain/Markdown), Security (Sign `Ctrl+Shift+G`, Encrypt `Ctrl+Shift+E`, Protect subject).

### 5.5 Keyboard and pointer

Single-key shortcuts act only when focus is in the list or reading pane, never in a text field.

`J`/`K` or arrows move in the list; `Enter` or `O` opens; `Space`/`Shift+Space` pages the message and at the end goes to the next unread; `X` toggles row selection; `Z` undoes; `Tab`/`F6` cycles panes; `Ctrl+1…9` switches tab; `Ctrl+Shift+K` inserts a link in compose; `Esc` closes a popover, leaves search or closes a message tab.

Context menus:

- **Message row**: Reply, Reply all, Forward, Archive, Delete, Move to, Copy to, Mark, Label, Snooze, Mute thread, Open in new tab, Open in new window, Create rule, View source, Copy message link (`mid:`).
- **Folder**: New subfolder, Rename (`F2`), Mark all read, Empty (Trash, Junk), Sync policy, Subscribe, Add to favourites, Properties.
- **Sender chip**: New message to, Copy address, Add to contacts, Show sender view, Always load images, Encryption policy, Block sender (local rule).
- **Attachment chip**: Preview, Open in disposable sandbox, Open with…, Save as…, Copy, Show hash.
- **Link**: Copy target, Open (asks), Open in disposable browser profile.

Drag and drop:

| Drag | Onto | Result |
|---|---|---|
| Messages | Folder or label | Move (Ctrl: copy; onto label: add label) |
| Messages | Compose editor | Attach as `message/rfc822` |
| Messages | Files or desktop | `.eml` files (offer `message/rfc822` and `text/plain`) |
| Files | Compose window | Attach; images onto the editor insert inline |
| Files `.eml`, `.mbox` | Folder | Import |
| Attachment chip | Files or another app | The file, with provenance attributes |
| Address chip | Between To, Cc, Bcc | Move the recipient |
| Folder | Folder tree | Move or reorder |
| Tab | Window edge / outside | Split view / new window |

### 5.6 States

| State | Behaviour |
|---|---|
| Empty (no account) | "Add a mail account to start." with [Add account] and [Import an archive]. |
| Empty folder | "Nothing in Inbox." No illustration. |
| Loading | List shows cached rows at once; a thin progress line under the toolbar; bodies not yet fetched show "Downloading…" with size. |
| Error | Per account banner in the sidebar row and status bar; "Details" opens the sync log. Certificate errors show the chain and never offer a silent bypass. |
| Offline | Header switch on; status "Offline · 3 queued"; Send reads Queue; server search disabled with a reason tooltip. |
| Route down | Account marked "Route unavailable (Tor)"; no fallback; [Open Anonymity] button. |
| Locked | Account store key not available: folder tree shows a lock; [Unlock] asks the keyring. |
| Read-only | Imported archives opened in place and shared read-only folders (ACL without write): write actions disabled with a tooltip. |
| Narrow (<640) | One pane at a time: folders, then list, then message; `Alt+Left` goes back; compose is full width; the inspector is a sheet. |
| Medium (640–1100) | Sidebar collapses to icons; reading pane below or as a pushed page. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Sidebar | `treeView` in a `scrollArea` | counts as trailing badges |
| Message list | `modelListView` over `SqliteModel` | two-line delegate; needs sticky group headers (FUSE-WP-28) |
| Reading pane body, plain | read-only text view on the G-10 widget | selection, links |
| Reading pane body, HTML | G-10 rich text view fed by the mail document tree | section 6.5 |
| Banners | `panel` + `label` + `button` | |
| Attachment strip | `gridView` of chips | |
| Tabs | `tabWidget` / `tabBar` | tear-off needs FUSE-WP-31 |
| Compose editor | G-10 rich text; `codeEdit` for plain and Markdown; `markdownView` for preview | |
| Recipient field | `lineEdit` plus chips | missing: tag input (`CO01-GAP-1`, see FUSE build plan "tag input") |
| Search field | `lineEdit` | missing: search field with tokens (`CO01-GAP-2`) |
| Rules editor | `listView`, `comboBox`, `lineEdit`, `checkBox` | |
| Sieve editor | `codeEdit` | Sieve grammar file |
| MIME tree, folder subscriptions | `treeView` | |
| Account settings | `listView` + `propertyGrid`-style forms | |
| Other | `splitter`, `progressBar`, `dialog`, `menuBar`, `popupMenu`, `tooltip`, `calendar`, `dateTimeEdit`; toasts, popovers and sheets are missing in V3 and planned in FUSE-WP-30 | |

Gaps: G-10 (rich text), G-17 (spell check), `CO01-GAP-1` tag input, `CO01-GAP-2` token search field, `CO01-GAP-3` mail document view (a read-only profile of G-10 with table layout and image boxes; no new widget if G-10 covers it).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, commands, threading, rule engine, journal, scheduler |
| UI | FUSE UI (`.fuseui`, CSS) | app host | All windows |
| Store module | ForgeScript over SQLite (FORGE-WP-23.1) | app host | Index database, blob store bookkeeping |
| Mail protocol helper | C++23 | one per account, `proc.helper:mail-proto` | IMAP, JMAP, SMTP, POP3, ManageSieve state machines and parsers. Gets connected TLS sockets from the network service; has no file access except blob descriptors passed in. |
| MIME helper | C++23 | pooled, `proc.helper:mime` | RFC 5322 and MIME parse and build, charset decoding, header decoding (RFC 2047, 2231), threading keys, text extraction for the index. No capabilities. |
| HTML sanitiser helper | C++23 | one per rendered message, `proc.helper:html-safe` | HTML and CSS tokenise, tree build, allow-list filter, output the mail document tree. No capabilities. Killed after 2 s or 256 MiB. |
| Image and preview helper | C++23 | pooled, `proc.helper:preview` | Decode inline images to bitmaps in shared memory; first-page previews. No capabilities. Uses the system image decoders shared with `svc.thumbnail`. |
| Fetch proxy helper | C++23 | one, `proc.helper:fetch` | Remote images, WKD, autoconfig, unsubscribe POST, OAuth token calls. Sockets from the network service on the account's route. |
| Import helper | C++23 | per job, `proc.helper:import` | mbox, Maildir, PST, emlx readers. Reads only the handle the portal gave. |
| Crypto engine | C++23 (SE04's helper) | SE04 process | OpenPGP and CMS operations on key handles |
| Network service | C++23 (platform, FORGE-WP-21) | system | Rule check, route, DNS, connect, TLS in the per-app protocol helper |

```
                       +-----------------------------+
                       |  app host (ForgeScript+UI)  |
                       |  store · journal · rules    |
                       +--+----+----+----+----+---+--+
          session bus     |    |    |    |    |   |
   +----------------------+    |    |    |    |   +------------------+
   v                           v    v    v    v                      v
+----------+   +-----------+ +----+ +--------+ +-------+   +-------------------+
| mail-    |   | mime      | |html| |preview | |import |   | svc.keyring, SE04 |
| proto    |   | helper    | |safe| |helper  | |helper |   | engine, CO03      |
| (per     |   | (no caps) | |    | |        | |       |   | picker, CO02 ical |
| account) |   +-----------+ +----+ +--------+ +-------+   +-------------------+
+----+-----+        fetch proxy helper ----+
     | sockets passed as descriptors       |
     v                                     v
+------------------------------------------------+
| network service: rules, route (SE03), DNS, TLS |
+------------------------------------------------+
```

### 6.2 Data model

Stores under `~/.local/share/org.pbsd.mail/`: `index.sqlite` (schema version in table `meta`), `fts.sqlite` (rebuildable), `fts-enc.sqlite` (encrypted-mail index, optional), `blobs/ab/cd/<blake3>.zst`, `journal.sqlite`. Cache under `~/.cache/org.pbsd.mail/` (decoded images, thumbnails, remote images keyed by URL hash).

| Entity | Fields |
|---|---|
| `account` | `id: uuid`, `accounts_ref: text` (id in `svc.accounts`), `kind: enum(imap, jmap, local)`, `name: text`, `colour: text`, `route: text`, `sync_policy: json`, `trusted_authserv: json`, `enc_policy: enum`, `state: json` |
| `identity` | `id: uuid`, `account_id`, `name`, `address`, `reply_to`, `signature_plain`, `signature_rich`, `key_ref: text`, `smtp_ref: text`, `sent_folder_id` |
| `folder` | `id: uuid`, `account_id`, `parent_id`, `path: text`, `delimiter: text`, `role: enum`, `uidvalidity: u32`, `uidnext: u32`, `highestmodseq: u64`, `jmap_id: text`, `subscribed: bool`, `sync_mode: enum`, `total: u32`, `unread: u32` |
| `message` | `id: i64`, `blob: blob32` (BLAKE3), `message_id: text`, `thread_id: i64`, `date: i64` (UTC s), `received: i64`, `subject: text`, `from_json`, `to_json`, `cc_json`, `size: u32`, `has_attachments: bool`, `snippet: text`, `list_id: text`, `auth_json`, `crypto_state: enum`, `body_state: enum(none, headers, body, full)` |
| `folder_message` | `folder_id`, `message_id`, `uid: u32`, `modseq: u64`, `flags: u32`, `keywords: json`, `jmap_email_id: text` |
| `thread` | `id: i64`, `root_message_id: text`, `subject_norm: text`, `last_date: i64`, `count: u32`, `unread: u32`, `muted: bool` |
| `part` | `message_id`, `path: text` ("1.2"), `type: text`, `name: text`, `size: u32`, `cid: text`, `disposition: enum`, `sniffed_type: text`, `sha256: blob32` |
| `journal_op` | `seq: i64`, `account_id`, `kind: enum(flag, move, copy, delete, append, send, create_folder, …)`, `args: json`, `state: enum(pending, sent, done, failed)`, `attempts: u16`, `error: text`, `created: i64` |
| `outbox_item` | `id`, `identity_id`, `blob`, `envelope_json`, `not_before: i64`, `undo_until: i64`, `state`, `server_reply: text` |
| `rule` | `id`, `account_id`, `name`, `enabled`, `order: u16`, `match: enum(all, any)`, `conditions: json`, `actions: json`, `run_on: u8` |
| `snooze` | `message_id`, `wake_at: i64`, `from_folder_id` |
| `peer_key_state` | `address: text`, `autocrypt_json`, `last_seen: i64`, `policy: enum` (key material itself lives in SE04 and CO03) |
| `remote_allow` | `scope: enum(sender, domain)`, `value: text`, `account_id` |
| `recent_recipient` | `address`, `name`, `count: u32`, `last_used: i64` |
| `template` | `id`, `name`, `subject`, `body_plain`, `body_rich`, `variables: json` |

Migration: forward only; the previous database is copied to `index.sqlite.v<N>.bak` before a step. FTS is rebuilt, not migrated. Blobs never change.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| IMAP4rev1/rev2 and extensions | yes | yes | own C++23 | mail protocol helper |
| JMAP Core, Mail, Submission (JSON over HTTPS) | yes | yes | own; HTTP from FORGE-WP-21 | mail protocol helper |
| SMTP submission | — | yes | own | mail protocol helper |
| POP3 | yes | — | own | mail protocol helper |
| ManageSieve, Sieve scripts | yes | yes | own | protocol helper; Sieve syntax check in ForgeScript |
| TLS 1.2/1.3, X.509 | yes | yes | platform choice (FORGE-WP-21, open Q6 there) | per-app protocol helper |
| SASL (PLAIN, SCRAM, OAUTHBEARER, EXTERNAL) | yes | yes | own | mail protocol helper |
| OAuth2 (RFC 6749, 7636, 8252, 8628) | yes | yes | own | fetch proxy helper |
| RFC 5322, MIME, RFC 2047/2231, `format=flowed` | yes | yes | own | MIME helper |
| Legacy charsets (ISO-8859-x, Windows-125x, ISO-2022-JP, GBK, Big5, KOI8) | yes | UTF-8 only | FORGE `forge_text` encodings where present (Unverified coverage), else own tables | MIME helper |
| HTML, CSS (mail subset) | yes | yes (generated subset) | own tokeniser and tree builder following the WHATWG parsing algorithm | HTML sanitiser helper |
| OpenPGP (RFC 9580, RFC 4880), PGP/MIME | yes | yes | SE04 engine | SE04 engine process |
| CMS, S/MIME | yes | yes | SE04 engine | SE04 engine process |
| Autocrypt headers | yes | yes | own | MIME helper (parse), app (state) |
| iCalendar in `text/calendar` | summary only | — | CO02 iCalendar helper | CO02 helper |
| mbox, Maildir, EML, emlx, TNEF (read only, in the MIME helper) | yes | yes (mbox, Maildir, EML) | own | import helper |
| PST, OST | yes | no | own reader from the published MS-PST specification | import helper |
| JPEG, PNG, GIF, WebP, SVG subset in mail | yes | — | system decoders | preview helper |
| zstd (blob compression) | yes | yes | zstd, BSD-3-Clause (vendored), or the codec FORGE-WP-23.5 selects | app host (own data only) |

Third-party code proposed: zstd only, unless FORGE-WP-23.5 picks another codec. Licence stated from memory; confirm before vendoring.

### 6.4 Integration

- **Intents handled**: `compose` (with `mailto:` fields, attachments as handles), `share` (files, text, URLs), `open`/`view` for `message/rfc822`, `application/mbox`.
- **Intents emitted**: `open`, `view`, `print`, `share`, `pick` (contact), `compose` to CO05 (start chat with sender), `new` to CO04 and OF02.
- **URL schemes**: `mailto:`, `mid:`, `cid:` (internal only).
- **MIME**: view `message/rfc822`, `application/mbox`, `application/vnd.ms-outlook` (PST, import); export `message/rfc822`, `application/mbox`.
- **Services used**: `svc.accounts`, `svc.keyring`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.thumbnail`, `svc.settings`, `svc.backup`, `svc.audit`, `svc.power`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y`, `svc.model` (optional).
- **Service offered**: `org.pbsd.mail.send` (Proposed): another app asks to send a prepared message; Mail always shows the compose window, never sends silently. Used by CO02 for iMIP replies.
- **CO02**: iTIP summary and reply building (`org.pbsd.calendar.itip`). **CO03**: picker and key lookup (`org.pbsd.contacts.picker`, `.keys`), interaction history. **CO04**, **OF02**: `new` intent with a `mid:` link. **SE04**: key handles, certificate chains, OAuth token storage. **SE03**: route state and circuit identity. **IN01**: OAuth consent page and "open link in disposable profile".
- **Backing repos**: Verified: FUSE has `SqliteModel`, `codeEdit`, `markdownView`, `treeView`, `modelListView` (`FUSE:docs/ui/README.md`). Verified: FORGE today has Ed25519, XChaCha20-Poly1305, BLAKE2b, Argon2id and lacks SHA-256, X.509, TLS, OpenPGP and S/MIME (stated in `Apps/00-foundation/build-plans/FORGE_BUILD_PLAN.md`, FORGE-WP-22). Everything else in this section is Proposed.

### 6.5 Decision: HTML mail rendering

Choice: **a restricted HTML subset renderer on top of G-10, not the web content engine G-14.**

| Reason | Detail |
|---|---|
| Attack surface | Mail HTML is the most hostile content the desktop sees unprompted. A full engine brings a DOM, a style engine, layout and possibly script. Mail needs none of the dynamic parts. |
| Schedule | G-14 has no code and is the largest gap. Mail must not wait for it (ROADMAP.md section 3 allows "or the restricted HTML renderer"). |
| Sufficiency | Mail HTML is tables, inline styles, images and links. The subset below covers it. |
| Privacy by construction | The renderer has no fetch path. Remote images exist only if the app asks the fetch proxy and passes bitmaps in. |

Pipeline: MIME helper extracts the HTML part, then the sanitiser helper parses it and emits a **mail document tree** (a typed record list: blocks, inline runs, tables, images, links with real targets), then the app feeds that tree to the G-10 view. The G-10 view never sees HTML.

Allowed: `p div span br hr h1–h6 ul ol li dl dt dd blockquote pre code b strong i em u s sub sup small a img table thead tbody tfoot tr td th caption font center`. CSS (inline, `<style>` blocks, simple selectors: type, class, id, descendant): `color background-color font-family font-size font-weight font-style text-decoration text-align line-height margin padding border width max-width height (images and cells) vertical-align white-space display:none|block|inline|table*`. Media queries limited to `max-width` and `prefers-color-scheme`.

Removed: `script iframe object embed form input button select textarea video audio svg math link meta base frame`, all `on*` attributes, `position`, `float` beyond left/right on images, `opacity` under 0.3, `z-index`, `transform`, `@import`, `@font-face`, `url()` in CSS, `expression`. Text made invisible (same colour as background, font size under 6 px, `display:none` containing links) is flagged to the phishing checks. Link schemes allowed: `https`, `http`, `mailto`, `mid`, `cid`; others shown as text.

Escape hatch (Phase 3, needs G-14): "Open in browser sandbox" renders the message in IN01 in a disposable profile with network blocked unless the user allows it.

## 7. Security and privacy

### 7.1 Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | Store, index, journal, cache |
| `fs.portal` | Save and attach files, import and export archives |
| `net.user` | Mail servers the user typed; each first contact is logged |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-account route; the service enforces the one set on the account |
| `proc.helper:mail-proto`, `:mime`, `:html-safe`, `:preview`, `:fetch`, `:import` | Helpers in section 6.1 |
| `svc.accounts` | Endpoint records |
| `svc.keyring` | Passwords, OAuth refresh tokens, store keys, key handles |
| `svc.notify` | New mail, send failures, snooze wake |
| `svc.portal.file`, `svc.portal.print` | Attachments, import, export, printing |
| `svc.intent` | Open attachments and links, hand-offs to CO02, CO03, CO04, IN01, SE04 |
| `svc.search` | Optional header contribution to system search |
| `svc.thumbnail` | Attachment thumbnails |
| `svc.settings`, `svc.backup`, `svc.audit`, `svc.power`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y` | Standard services |
| `svc.model`, `ai.local` | Optional summarise, translate, compose assist; off by default |
| `bg.run` | IDLE, send later, snooze wake without a window |
| `bg.autostart` | Optional; off by default. Not requested: `proc.exec`, `fs.path`, any `dev.*`. |

### 7.2 Threat model

Assets: message contents, credentials and tokens, private keys (held by SE04), the social graph, the user's network location and reading behaviour.

| Adversary | Attack | Mitigation |
|---|---|---|
| Remote sender | Malformed MIME, HTML, images, PST to gain code execution | All parsing in zero-capability helpers; T2 targets and fuzzing; helpers killed on time and memory limits |
| Remote sender | Tracking pixels, read receipts, link tracking | No remote fetch by default; fetch proxy; tracker stripping; MDN never automatic |
| Remote sender | Phishing, display-name spoofing, look-alike domains, hidden text | Authentication display, spoofing indicators, real link targets, confusables check |
| Remote sender | Exfiltration of decrypted text through HTML or quoted parts | Isolated rendering of decrypted parts; renderer has no fetch path; reply quotes only the decrypted part's text |
| Remote sender | Malicious attachment | Dangerous type gate; disposable sandbox; content sniffing; provenance attributes |
| Network attacker | Downgrade, STARTTLS stripping, bad certificate | TLS required; no plaintext fallback; pinning optional; certificate errors stop |
| Malicious or compromised server | Forged `Authentication-Results`, injected messages, flag tampering | Only trusted `authserv-id` headers shown as results; end-to-end signatures shown separately; local DKIM check optional |
| Network observer | Linking accounts by IP, timing or headers | Per-account route with circuit isolation; header hygiene; fail-closed route |
| OAuth provider page | Token theft through the embedded view | No embedded view; consent in IN01 disposable profile; PKCE; loopback redirect bound to a random port and state |
| Other local app | Reading mail store | Store is under `fs.app`; other apps have no handle; `svc.search` gets headers only when enabled |
| Compromised helper | Pivot to files or network | Capability mode; only passed descriptors; protocol helper cannot open new sockets |
| Local attacker with the unlocked session | Read all mail | Out of scope beyond per-account store keys (section 4.2) and session lock |

What is stored: section 6.2. What leaves the machine: protocol traffic to the user's servers; remote-content, WKD and unsubscribe requests only after the user allowed them; nothing else.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `view.reading_pane` | enum right/below/off | right | Reading pane position |
| `view.prefer_plain` | bool | false | Show the plain part when present |
| `privacy.remote_content` | enum blocked/contacts/always | blocked | Remote content policy |
| `privacy.strip_trackers` | bool | true | Remove trackers and tracking parameters |
| `privacy.utc_date` | bool | false (true on Tor accounts) | UTC `Date` header |
| `privacy.mdn` | enum never/ask | never | Read receipts |
| `send.undo_seconds` | int 0–120 | 10 | Undo-send window |
| `send.default_format` | enum rich/plain/markdown | plain | Compose format |
| `crypto.default_policy` | enum always/opportunistic/ask/never | opportunistic | Encryption policy |
| `crypto.protect_subject` | bool | true | Protected headers |
| `crypto.wkd_lookup` | bool | false | WKD lookups |
| `crypto.index_decrypted` | bool | false | Index decrypted text |
| `sync.idle_folders` | int 1–10 | 3 | IDLE connections per account |
| `sync.default_mode` | enum full/no_large/headers/days | full | Folder sync policy |
| `notify.enabled` | bool | true | New-mail notifications |
| `search.system_index` | bool | false | Contribute headers to `svc.search` |
| `attach.unknown_sender_sandbox` | bool | true | Disposable sandbox by default for unknown senders |
| other view, send and sync keys | — | — | `view.mark_read_after_s` (2), `privacy.confirm_links` (first per host), `sync.metered` (headers only), `net.max_connections` (4), `notify.show_content` (sender), `view.density`, `view.threads`, `view.dark_adapt_html`, `send.reply_format`, `send.quote_position`, `send.check_attachment_words`, `crypto.sign_default`, `crypto.autocrypt`, `sync.poll_minutes` (15), `sync.large_attachment_mib` (25), `junk.local_classifier`, `notify.folders`, `keys.scheme`, `archive.scheme` (year); each mirrors a feature in section 4 |
| `ai.enabled` | bool | false | Local model features |

## 9. Accessibility and languages

- Message list rows expose: unread state, sender, subject, date, attachment, encryption, warning, thread count, in that order, as one a11y name plus separate states.
- Thread reading mode: each message is a region with a heading; quoted blocks are collapsible groups with "quoted text, 12 lines".
- Trust badges and banners are announced as live regions (polite); spoofing warnings are assertive once.
- HTML mail text obeys minimum font size 11 px and the high-contrast override.
- Right-to-left messages render with the paragraph direction from the Unicode algorithm; the list mirrors by locale, not by message.
- Internationalised addresses (SMTPUTF8, IDNA) display in Unicode with a punycode tooltip; mixed-script hosts are flagged.
- Forgotten-attachment words, reply prefixes (`Re:`, `AW:`, `SV:`, `RE :`) and subject normalisation use per-locale tables.
- Single-key shortcuts can be turned off as a whole for switch and voice users.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame with cached list | under 400 ms |
| Open a folder of 1 000 000 messages | first rows under 100 ms; scroll at 60 fps |
| Open a cached 100 KiB HTML message | under 80 ms including the sanitiser round trip |
| Search across 2 000 000 messages | first 50 results under 300 ms |
| Idle | zero frames; network wake-ups only from IDLE keep-alive (one per 25 min per connection) |
| Memory | app host under 250 MiB with a 2 000 000 message store; each helper under 256 MiB hard limit |
| Largest supported | 5 000 000 messages per account, 100 GiB mbox import, 150 MiB single message |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit (ForgeScript)**: threading, query parser, rule engine, journal replay and conflict cases, Autocrypt recommendation table, snooze and send-later clocks with a fake time source.
- **Protocol**: scripted fake IMAP, JMAP, SMTP, POP3 and ManageSieve servers with recorded transcripts; disconnects at every byte offset; QRESYNC and non-QRESYNC diffs; differential runs against Dovecot, Cyrus and Stalwart in a jail on the build machine, offline.
- **MIME corpus**: differential comparison of parsed structure against a reference parser on a public test corpus plus the author's own archive; round-trip build and parse.
- **HTML sanitiser**: allow-list property tests (output never contains a removed element, attribute, scheme or CSS property); the html5lib tokeniser and tree-construction fixtures vendored for offline runs; known mail-client exploit samples.
- **Crypto**: interoperability vectors with GnuPG and Sequoia outputs (files, not binaries at test time); EFAIL-style gadgets must not render or fetch.
- **Fuzz**: IMAP response parser, JMAP JSON, MIME, RFC 2047, HTML, CSS, PST, mbox, TNEF, Autocrypt header, Authentication-Results.
- **Golden renders**: main window, compose, trust sheet, each banner, in dark, light and high contrast at 100% and 200%.
- **Scripted UI through the a11y tree**: the 15 top tasks.
- **Privacy**: a network trace test asserting zero packets on opening any corpus message with default settings, and zero clear-route packets for a Tor account.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| HTML sanitiser filter | Output tree contains only allow-listed node kinds, attributes and schemes | T1 | PRISM proof over the filter function on the typed tree | Needs a formal model of the tree type; parser itself stays T2 |
| HTML and CSS tokeniser, tree builder | No runtime errors on any input | T2 | PRISM bounded checks at entry points, fuzzing | Size of the parsing algorithm |
| MIME helper | No runtime errors; bounded recursion (depth 64) | T2 | PRISM, fuzzing | — |
| IMAP, SMTP, POP3, JMAP parsers | No runtime errors | T2 | PRISM, fuzzing, transcript tests | — |
| PST, mbox, TNEF readers | No runtime errors; no out-of-file offsets followed | T2 | PRISM, fuzzing | — |
| Journal replay | Replaying a journal twice gives the same server state (idempotence) | T3, T1 as a stretch | Model-based tests against a fake server | Needs a server model |
| Fetch proxy | Never sends cookies, referrer or non-allowed headers | T2 plus unit tests | Request builder is the only path; proof of header allow-list | — |
| Rule engine, threading, query parser | Memory safe by language | T3 | ForgeScript checks, unit tests | Interpreter not verified |
| TLS, OpenPGP, CMS | Owned elsewhere | — | FORGE-WP-21, SE04 | — |
| SQLite | Vendored C | T4 | Defensive configuration | Third-party C |

## 12. Build plan

### Phase 1 — usable core

| ID | Work item | Depends on |
|---|---|---|
| CO01-W01 | Store: schema, blob store, journal, migrations | G-07, G-09 (`svc.settings`), FORGE-WP-23.1 |
| CO01-W02 | MIME helper: parse, build, charsets, snippet and text extraction | G-07, FORGE-WP-27, FORGE-WP-28 |
| CO01-W03 | Mail protocol helper: IMAP core, IDLE, CONDSTORE, QRESYNC; SMTP | FORGE-WP-21, G-08 |
| CO01-W04 | Account wizard, `svc.accounts` and `svc.keyring` use, offline provider table | G-09, CO01-W03 |
| CO01-W05 | Main window: sidebar, list on `SqliteModel`, reading pane, plain-text view | G-01, G-02, FUSE-WP-28 |
| CO01-W06 | HTML sanitiser helper and mail document view | G-10 (read-only profile), CO01-W02 |
| CO01-W07 | Threading, unified folders, flags, move, archive, delete, undo | CO01-W01, CO01-W05 |
| CO01-W08 | FTS index, query language, quick filters | CO01-W01, CO01-W02 |
| CO01-W09 | Compose in plain text, drafts, outbox, undo send, sent copy | CO01-W03, CO01-W05 |
| CO01-W10 | Attachments: strip, save, dangerous type gate, inline images (preview helper) | CO01-W02, `svc.portal.file` |
| CO01-W11 | Fetch proxy helper; remote content banner and allow lists | FORGE-WP-21, CO01-W06 |
| CO01-W12 | Notifications, badge, `mailto:`, `compose` and `share` intents, session restore | G-09 |
| CO01-W13 | Fake servers, corpus, fuzz harnesses, golden renders | FORGE-WP-35 |

### Phase 2 — full feature

| ID | Work item | Depends on |
|---|---|---|
| CO01-W14 | JMAP client | CO01-W03 |
| CO01-W15 | OAuth2 hand-off | IN01 (disposable profile), `svc.intent`, CO01-W11 |
| CO01-W16 | Per-account route, isolation, fail-closed | SE03, SE02, FORGE-WP-21.4 |
| CO01-W17 | OpenPGP, Autocrypt, key discovery, trust display, partial-encryption defence | SE04 engine (FORGE-WP-22.4), CO03 keys |
| CO01-W18 | Authentication display, spoofing indicators, trust sheet | CO01-W02, FORGE-WP-23.4 |
| CO01-W19 | Rich and Markdown compose, signatures, templates, spell check | G-10 (edit), G-17 |
| CO01-W20 | Local rules, Sieve editor, ManageSieve | CO01-W03 |
| CO01-W21 | Snooze, send later, background sync | `bg.run`, `svc.power` |
| CO01-W22 | Tracker stripping, unsubscribe helper, mailing-list tools, junk classifier | CO01-W06, CO01-W11 |
| CO01-W23 | Invitation card and iMIP send service | CO02 (`itip` interface) |
| CO01-W24 | Contact picker, sender card, interaction history | CO03 |
| CO01-W25 | Attachment preview, disposable sandbox open | `svc.thumbnail`, SH04, disposable domain support (see section 13) |
| CO01-W26 | Import mbox, Maildir; export; POP3 import; backup rules | CO01-W02 |
| CO01-W27 | Printing, labels, saved searches, account health, sync log | G-16 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CO01-W28 | S/MIME, protected headers, hardware tokens | SE04 |
| CO01-W29 | PST/OST and other-client import | CO01-W26, CO02, CO03 |
| CO01-W30 | Encrypted-mail index, per-account store keys, decrypt for storage | CO01-W17 |
| CO01-W31 | Local DKIM check, privacy report, sender view, follow-up reminders | CO01-W18 |
| CO01-W32 | Local model features | G-15, `svc.model` |
| CO01-W33 | Rule scripts, rule test, mail merge, retention, vacation responder | CO01-W20 |
| CO01-W34 | "Open in browser sandbox" for complex HTML | G-14, IN01 |
| CO01-W35 | Device authorisation flow, server-to-server copy, diagnostics bundle | CO01-W15 |

## 13. Open questions and risks

1. **Disposable sandbox launch** is not defined in PLATFORM.md. Mail needs an intent flag "open in a throw-away domain with `net.none`". Owner is assumed to be SH04 with VM01 for the heavy case. New gap candidate.
2. **OAuth2 client identifiers.** Large providers require a registered client ID and may refuse unknown clients. No vendor account is allowed, so the user must supply an ID or use an app password. Risk: some providers become unusable.
3. **A trusted browser hand-off portal** does not exist in PLATFORM.md section 8. Using IN01 means OAuth waits for G-14. Until then only password, SCRAM and certificate login work.
4. **TLS library** is undecided (FORGE-WP-21 Q6). All protocol work depends on it.
5. **G-10 table layout.** Mail HTML depends on nested tables. If G-10 tables are too limited, the mail document view needs its own table layout pass (`CO01-GAP-3`).
6. **PST reader** is large own code for a hostile format. It may slip from Phase 3.
7. **Sieve and server rules differ by server.** The form editor must degrade to the text editor when it cannot represent a script.
8. **Scheduled wake-ups.** Send later and snooze need the app to run (`bg.run`) or a system timer service that starts it. No such service is listed in PLATFORM.md section 8.
9. **Unverified**: FORGE legacy charset coverage (ISO-2022-JP, GB18030); local junk classifier quality. Nothing about SE03's route mechanism was read; the `tor` and `vpn` route names come from PLATFORM.md section 6 only.
