<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Transfers

By Odin Loch

| | |
|---|---|
| ID | `IN03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.transfers` |
| Category | internet |
| Priority | P2 |
| Backing repos | none. Uses the planned network service (FORGE-WP-21), FORGE's BLAKE3 and Ed25519 (Verified in the tree per `FORGE_BUILD_PLAN.md` ground truth, `FORGE:NOTICE`), and routes from SE03. |
| Status | Planned. Nothing built. |

## 1. Purpose

Transfers moves files between this machine and everything else: web downloads, BitTorrent, remote servers (SFTP, FTP, FTPS, WebDAV, S3-compatible), rsync, and other devices on the local network. It is for anyone who downloads large files, manages a server, seeds torrents or sends a file to a phone or a second computer. It replaces KGet, qBittorrent, FileZilla, and KDE Connect's file sending.

## 2. Scope

In scope:

- Download manager: HTTP and HTTPS with resume, segments, mirrors, Metalink, checksum and signature checks.
- Quarantine and provenance tagging of everything that arrives.
- BitTorrent v1, v2 and hybrid, DHT, magnet links, encryption, sequential mode, seeding rules, strict binding to one network route.
- Two-pane remote file client for SFTP, FTP, FTPS, WebDAV and S3-compatible stores.
- rsync client and one-way folder synchronisation jobs.
- Encrypted device-to-device send on the local network, verified by a code.
- One queue with scheduling, priorities and bandwidth limits.
- Download hand-off from IN01 and other apps; site bookmarks; credentials in `svc.keyring`.

Out of scope:

| Item | Owner |
|---|---|
| Mounting remote locations in the file manager | SY02 Files (it may reuse this app's protocol helpers; see 6.4) |
| Releasing files from quarantine, malware scan, signature key management | SE05 Vault, SE04 Passwords and Keys |
| Backups and ZFS send/receive | AD03 Backup |
| Running VPN or Tor | SE03 Anonymity |
| Firewall rules and port forwarding on the router | SE02 Firewall (Transfers only shows port state) |
| Playing media while downloading | ME03 Video, ME02 Music (Transfers offers sequential download and an "open" hand-off) |
| Interactive remote shells | SY03 Terminal |
| Git transport | DV03 Version Control |

## 3. Users and top tasks

Personas:

- **Downloader**: fetches disk images and large archives; wants resume, checksum checks and no babysitting.
- **Torrent user**: wants torrents to use the VPN only, ever; sets ratio limits; watches a folder.
- **Site owner**: uploads to a server over SFTP or to a bucket; compares folders; edits a remote file.
- **Household user**: sends photos from one of their machines to another with no cloud.
- **Administrator**: runs scheduled rsync jobs at night with a speed limit.

| # | Task | Target steps |
|---|---|---|
| T1 | Download a URL | `Ctrl+N`, paste, Enter (3) |
| T2 | Verify a download against a published SHA-256 | select, `Ctrl+Shift+V`, paste hash (3) |
| T3 | Resume everything after a reboot | none (automatic) |
| T4 | Add a magnet link on the VPN route | click link in browser, confirm sheet (2) |
| T5 | Confirm a torrent cannot leak outside the VPN | look at the route chip on the row (0) |
| T6 | Connect to a saved SFTP site | sidebar, double-click site (1) |
| T7 | Upload a folder to a server | drag from left pane to right pane (1) |
| T8 | Send a file to my other device | right click file in Files, Share, pick device, compare code (4) |
| T9 | Limit speed now | status bar speed chip, pick preset (2) |
| T10 | Schedule large downloads for 01:00–06:00 | Preferences > Schedule, tick, set times (3) |
| T11 | Compare a local and a remote folder | `Ctrl+Shift+C` in the two-pane view (1) |
| T12 | Run a saved rsync job | Jobs, select, Run (2) |
| T13 | Find where a file came from | select, `F4`, Provenance (2) |

## 4. Features

Priority: M must, S should, C could. Phase: 1 usable core, 2 full feature, 3 advanced.

### 4.1 Download manager (HTTP and HTTPS)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F001 | Add by URL | Dialog accepts one or many URLs (one per line), a Metalink file or a text file of URLs. Shows name, size and type from a `HEAD` (or ranged `GET`) before starting. | M | 1 |
| IN03-F002 | Resume | Range requests with `If-Range` on ETag or Last-Modified. If the validator changed, the partial file is kept as `.old` and the user is asked. | M | 1 |
| IN03-F003 | Segmented download | Up to 8 segments per file (default 4), at most 4 connections per host. Segments are re-split when one finishes. Turned off automatically when the server refuses ranges. | M | 2 |
| IN03-F004 | Mirrors | Several URLs for one file; segments spread across mirrors; a slow or failing mirror is dropped; per-mirror statistics. | S | 2 |
| IN03-F005 | Metalink | Metalink 4 (RFC 5854) and Metalink in HTTP headers (RFC 6249): mirrors, sizes, hashes, piece hashes, signatures. Piece hashes let one bad segment be re-fetched alone. | S | 2 |
| IN03-F006 | Partial files | Data is written to `<name>.part` with a sidecar `<name>.part.state` (segment map, validators). Space is reserved up front where the file system allows. Renamed only after verification rules pass. | M | 1 |
| IN03-F007 | Retry policy | Exponential back-off from 5 s to 10 min, up to 20 tries (setting); 4xx other than 408 and 429 stop at once; `Retry-After` is honoured. | M | 1 |
| IN03-F008 | Name and conflict handling | Name from `Content-Disposition` (sanitised: no path separators, no control characters, no trailing dots, length cap 200), else the URL path. On conflict: rename with a number, overwrite, or skip; asked once per batch. | M | 1 |
| IN03-F009 | Destination | Default folder from a remembered portal grant; per-category folders by rule (type, host, size); "Ask each time". | M | 1 |
| IN03-F010 | Categories and rules | Rules on host, extension, MIME type, size: set folder, route, priority, segments, post-action. | S | 2 |
| IN03-F011 | Request details | Optional headers, referrer and cookies supplied by the calling app for that one request. Never stored beyond the transfer's life unless the user ticks "keep for resume". | M | 1 |
| IN03-F012 | Authentication | HTTP Basic, Digest and bearer; secrets in `svc.keyring` per host and realm. Basic is refused over plain HTTP unless confirmed. | S | 2 |
| IN03-F013 | HTTPS policy | HTTPS first. A redirect from HTTPS to HTTP is refused unless a strong checksum (Metalink or user-supplied) protects the file. | M | 1 |
| IN03-F014 | Redirect limits | At most 10 redirects; each hop listed in the provenance record. | M | 1 |
| IN03-F015 | Compression | `Accept-Encoding` identity for file downloads so sizes and ranges stay exact. | M | 1 |
| IN03-F016 | Batch patterns | URL patterns with ranges: `file[001-120].jpg`, `{a,b,c}`. Preview of the expansion before adding; cap 10 000. | C | 3 |
| IN03-F017 | Page link grab | Given a page URL passed from IN01, list the links the browser extracted (the browser sends the list; Transfers does not parse pages). Filter by type and pick. | C | 3 |
| IN03-F018 | Clipboard watch | Off by default. When on, a copied URL that ends in a file extension raises a toast "Download this?"; uses `svc.clipboard` and never reads items flagged sensitive. | C | 3 |
| IN03-F019 | Open while downloading | For media types, "Open" passes a read handle to ME03 or ME02 when the download is sequential and enough data exists. | C | 3 |
| IN03-F020 | Post-actions | After completion: verify, open folder, hand to UT01 Archives to extract (in its sandbox), run nothing else. No user scripts. | S | 2 |
| IN03-F021 | Link refresh | When a signed URL expires (403 after a pause), "Refresh address" lets the user paste a new URL and keeps the data already fetched if size and hash allow. | S | 2 |
| IN03-F022 | Time stamp | Option to set the file time from `Last-Modified`. Off by default (provenance holds it anyway). | C | 2 |

### 4.2 Verification, provenance and quarantine

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F023 | Hash on the fly | SHA-256 and BLAKE3 are computed while writing; no second read pass for unsegmented files. | M | 1 |
| IN03-F024 | Checksum verification | Paste or load an expected hash: SHA-256, SHA-512, SHA-3, BLAKE2b, BLAKE3. MD5 and SHA-1 are accepted for comparison only and labelled "weak: proves nothing against an attacker". | M | 1 |
| IN03-F025 | Checksum files | Detects and offers sibling files `SHA256SUMS`, `*.sha256`, `*.sha512`, `CHECKSUM` from the same directory URL; parses BSD and GNU formats. | S | 2 |
| IN03-F026 | Signature verification | Detached OpenPGP (`.asc`, `.sig`), minisign, signify and SSH signatures, checked by SE05 against keys held in SE04. Result states signer, key fingerprint and trust level. | M | 2 |
| IN03-F027 | Signed checksum chain | Verifies `SHA256SUMS.asc` then the file against `SHA256SUMS` in one action and shows both links of the chain. | S | 2 |
| IN03-F028 | Verification rules | Per rule or per transfer: "do not release until verified". A failed check moves the file to a `failed` state, keeps it quarantined and offers delete or re-download. | M | 2 |
| IN03-F029 | Provenance record | Extended attributes on every received file and a row in the store: protocol, source URL or peer or device, redirect chain, route, time, TLS certificate fingerprint or SSH host key or torrent info-hash, hashes, requesting app, verification results. | M | 1 |
| IN03-F030 | Quarantine tag | Every file received from outside is tagged quarantined. Files and other apps show the badge; opening goes through SE05's rules. Files from the user's own paired devices are tagged "from own device" and follow a setting. | M | 1 |
| IN03-F031 | Provenance view | Inspector tab: the record as a readable list with copy buttons; "Open source page"; "Download again from the same source". | M | 1 |
| IN03-F032 | Type check | Sniffed type against extension and declared type; mismatch is shown; executables and disk images get a "dangerous type" row marker. | M | 2 |
| IN03-F033 | Provenance for uploads | Uploaded files get a log entry (destination, time, hash) in the store, not on the file. | S | 2 |
| IN03-F034 | Audit | Each new remote endpoint, each quarantine release request and each verification failure is written to `svc.audit`. | M | 1 |

### 4.3 BitTorrent

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F035 | Torrent files and magnets | Open `.torrent` (v1, v2, hybrid) and `magnet:` URIs (btih and btmh). Metadata exchange for magnets. | M | 2 |
| IN03-F036 | Add sheet | Shows name, size, file tree with tick boxes and priorities, trackers, private flag, route, destination, start paused, sequential. Nothing is contacted until confirmed. | M | 2 |
| IN03-F037 | Route binding | A torrent session binds to exactly one route (default: the first VPN profile; clear only after an explicit warning). The torrent helper's sockets come from the network service on that route. If the route drops, all sockets close and the row says "Route down: paused". | M | 2 |
| IN03-F038 | Leak self-test | Per session: compares the address a tracker or DHT peer reports for us with the route's exit address from SE03. A mismatch stops the session and raises an alert. | M | 2 |
| IN03-F039 | No fallbacks | No UPnP or NAT-PMP on the clear interface when bound to a VPN; port mapping only through the route's own port-forward facility if SE03 offers one. No IPv6 unless the route carries it. | M | 2 |
| IN03-F040 | DHT | Mainline DHT (BEP 5), bootstrap nodes configurable, DHT only inside the bound route; off for private torrents. | M | 2 |
| IN03-F041 | Peer discovery | Trackers over HTTP(S) and UDP (BEP 15), peer exchange (BEP 11). Local service discovery is off by default because it announces on the local network. | M | 2 |
| IN03-F042 | Protocol encryption | Message stream encryption: prefer, require or off. Stated in help: it hides the protocol from simple filters, it is not confidentiality. | M | 2 |
| IN03-F043 | Transports | TCP and uTP (BEP 29) with its delay-based congestion control. | S | 2 |
| IN03-F044 | Piece verification | SHA-1 pieces (v1) and SHA-256 Merkle trees (v2). A peer that sends bad pieces twice is banned for the session. | M | 2 |
| IN03-F045 | File selection and priority | Per file: skip, low, normal, high. Skipped files leave no stray data except shared boundary pieces kept in a `.parts` store. | M | 2 |
| IN03-F046 | Sequential and first/last | Sequential piece order per torrent; "first and last pieces first" for media preview. | S | 2 |
| IN03-F047 | Seeding rules | Stop at ratio (default 1.0), or at seeding time, or never; global and per torrent; action at limit: pause or remove (keep files). | M | 2 |
| IN03-F048 | Super-seeding | Initial-seed mode for torrents the user created. | C | 3 |
| IN03-F049 | Limits | Global and per-torrent download and upload rates, connection count, upload slots; active-torrent caps (downloading, seeding). | M | 2 |
| IN03-F050 | Web seeds | BEP 19 HTTP seeds, fetched through the same route and verified by pieces. | S | 3 |
| IN03-F051 | Tracker management | List with status, peers, next announce; add, remove, edit tiers; force re-announce. | S | 2 |
| IN03-F052 | Peer list | Address (hidden behind a "show" toggle in screenshots mode), client, flags, progress, rates; ban peer. No geolocation lookups. | S | 2 |
| IN03-F053 | Create torrent | From a file or folder picked in the portal: v1, v2 or hybrid, piece size, trackers, web seeds, private flag, comment, source tag. Hashing in a helper with progress. | S | 3 |
| IN03-F054 | Watch folder | A portal folder watched for new `.torrent` files; added with the rule set for that folder. | S | 3 |
| IN03-F055 | Move and recheck | Move data to another folder; force recheck; relocate when files were moved outside the app. | M | 2 |
| IN03-F056 | Block lists | Optional peer block list file (CIDR or P2P format) loaded from disk; no automatic fetching. | C | 3 |
| IN03-F057 | Disk behaviour | Sparse or pre-allocated; write cache with a size cap; ZFS-friendly write sizes; pieces verified before they are visible in the final file names. | S | 2 |
| IN03-F058 | Anonymous mode | Generic peer id and client name, no local address in extension handshake, no listening port on the Tor route. Tor routes are discouraged for BitTorrent and the sheet says why. | S | 3 |
| IN03-F059 | RSS for torrents | Not here. A feed rule in IN02 may hand enclosure links to Transfers. | C | 3 |
| IN03-F060 | Quarantine for torrents | Torrent content is tagged like any download; the provenance record holds the info-hash and tracker list, not peer addresses. | M | 2 |

### 4.4 Remote file client

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F061 | Protocols | SFTP (SSH 2), FTP, FTPS (explicit; implicit as an option), WebDAV over HTTPS, S3-compatible object stores (signature version 4, path or virtual-host style). | M | 2 |
| IN03-F062 | Two-pane view | Local pane (a portal-granted folder) and remote pane, or remote and remote. Each pane: path bar, `tableView` (name, size, modified, permissions, owner), filter, tabs. | M | 2 |
| IN03-F063 | Connect dialog and quick connect | Protocol, host, port, user, authentication, route, start folder. Quick-connect field accepts `sftp://user@host/path`. | M | 2 |
| IN03-F064 | SSH host keys | First contact shows the key type and SHA-256 fingerprint and asks. Stored per host in the app's known-hosts store. A changed key blocks with a full explanation and no one-click bypass. | M | 2 |
| IN03-F065 | SSH authentication | Public key through SE04's agent interface (the private key never enters Transfers), hardware-token keys through SE04, password or keyboard-interactive typed into a shell-drawn secret prompt. | M | 2 |
| IN03-F066 | SSH algorithms | Modern set only by default (Ed25519, ECDSA, RSA-SHA2 host keys; curve25519 and post-quantum hybrid key exchange where the library has it, to verify; AEAD ciphers). Legacy set is a per-site opt-in with a warning. | M | 2 |
| IN03-F067 | Jump hosts | One or more intermediate SSH hosts per site (`ProxyJump` equivalent). | C | 3 |
| IN03-F068 | FTP specifics | Passive mode default, active on request; `MLSD` listings with fallbacks for common `LIST` formats; UTF-8 on; plain FTP shows a red "not encrypted" bar and needs confirmation each session. | S | 2 |
| IN03-F069 | FTPS specifics | TLS 1.2 minimum; data channel protection `PROT P` required; certificate checked against the system trust store or a pinned fingerprint. | S | 2 |
| IN03-F070 | WebDAV specifics | `PROPFIND` depth 1 listings, locks honoured, chunked upload where the server supports it, quota display. | S | 2 |
| IN03-F071 | S3 specifics | Buckets as top-level folders; prefixes as folders; multipart upload with resume (part size 16 MiB default); storage class and server-side encryption headers as options; presigned link creation (time-limited) with a clear warning. | S | 3 |
| IN03-F072 | Transfers from panes | Copy, move, drag between panes, queue for later. Folder transfers recurse with a scan phase that shows counts and bytes. All transfers appear in the common queue. | M | 2 |
| IN03-F073 | Resume and overwrite rules | Per conflict: overwrite, overwrite if newer, overwrite if size differs, resume, rename, skip; "apply to all". Resume uses `REST`, SFTP offsets, HTTP ranges, multipart state. | M | 2 |
| IN03-F074 | Integrity after transfer | Where the server offers a hash (SFTP `check-file` extension, S3 checksums and ETag rules, WebDAV checksum properties, FTP `HASH`/`XSHA256`), compare it; otherwise compare size and state that content was not verified. | S | 3 |
| IN03-F075 | File operations | New folder, rename, delete (with confirmation: remote deletes cannot be undone), permissions (octal and tick boxes, recursive option), symlink display, properties. | M | 2 |
| IN03-F076 | Folder comparison | `Ctrl+Shift+C`: marks only-left, only-right, newer, different size; optional checksum compare; "select differing". | S | 3 |
| IN03-F077 | Synchronised browsing | Navigating in one pane follows in the other when the relative path exists. | C | 3 |
| IN03-F078 | Edit remote file | Downloads to the app cache, opens in OF01 by intent with a write handle, uploads on save after showing a diff size note; cache file removed after the session. | S | 3 |
| IN03-F079 | Remote search and filter | Name filter in the current folder; recursive find by name pattern with a depth limit. | S | 3 |
| IN03-F080 | Hidden files and sort | Toggle hidden; sort by any column; folders first; natural number order. | M | 2 |
| IN03-F081 | Connection handling | Up to 4 parallel connections per site (setting); keep-alive; automatic reconnect with back-off; idle disconnect after 10 min. | M | 2 |
| IN03-F082 | Transfer type | Binary always. ASCII mode for FTP is not offered. | M | 2 |
| IN03-F083 | Raw log | Per-connection protocol log (commands and replies, secrets masked) in a bottom panel; copy and save. | S | 2 |
| IN03-F084 | Site-to-site copy | Remote to remote through this machine (streamed, no temporary file). Server-side copy when both ends are the same S3 or WebDAV service. | C | 3 |

### 4.5 rsync and folder jobs

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F085 | rsync client | Push or pull over SSH or to an rsync daemon (`rsync://`), delta transfer. Implementation base in 6.3. | S | 3 |
| IN03-F086 | Job definition | Source, destination, direction, delete extraneous (off by default), exclude patterns, preserve times and permissions, compare by time and size or by checksum, bandwidth cap, route. | S | 3 |
| IN03-F087 | Dry run | Every job can run as a preview that lists what would be copied, changed and deleted, with totals. Jobs with "delete" force a preview on first run. | M | 3 |
| IN03-F088 | Saved jobs and schedule | Named jobs; run now, daily or weekly at a time, or when a named network is connected; missed runs are caught up once. | S | 3 |
| IN03-F089 | Mirror job without rsync | One-way mirror to SFTP, WebDAV or S3 using listings and hashes when the server has no rsync. | C | 3 |
| IN03-F090 | Job report | Per run: files, bytes, duration, errors; kept 90 days; notification on failure. | S | 3 |
| IN03-F091 | Snapshot before delete | Option: ask `svc.backup` for a snapshot of the local destination before a job that deletes. | C | 3 |

### 4.6 Local device send

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F092 | Discovery | Devices running the same protocol on the local link are found by multicast DNS service discovery, only while the Send or Receive panel is open or "discoverable" is on (off by default). | M | 2 |
| IN03-F093 | Pairing by code | First contact: both screens show a 6-digit code derived from a password-authenticated key exchange; the user confirms they match (or types the code shown on the other device). No code, no transfer. | M | 2 |
| IN03-F094 | QR pairing | Alternative: one device shows a QR code carrying its key fingerprint and address; the other scans it with ME04 Camera (intent `scan`). | S | 3 |
| IN03-F095 | Paired devices | Long-term device keys (Ed25519) stored in `svc.keyring`; device list with name, fingerprint, last seen; rename; revoke. | M | 2 |
| IN03-F096 | Encrypted channel | Mutually authenticated session with forward secrecy and an AEAD cipher (XChaCha20-Poly1305); the protocol is specified in this app's `schema/` and versioned. | M | 2 |
| IN03-F097 | Send | Files, folders and text (clipboard item) to one device. Drag onto the device tile, or use the share sheet from any app. | M | 2 |
| IN03-F098 | Receive consent | An incoming offer shows sender, file names, count and size; Accept or Decline. Auto-accept is possible only per paired device and only into a chosen folder. | M | 2 |
| IN03-F099 | Integrity | Each file carries a BLAKE3 hash checked on arrival; a transcript hash confirms the whole batch on both ends. | M | 2 |
| IN03-F100 | Resume | Interrupted sends resume from the last verified chunk within 24 h. | S | 3 |
| IN03-F101 | No relay | Transfers are direct on the local network. No rendezvous or relay server exists. Across networks, the user can use a VPN route that joins the devices. | M | 2 |
| IN03-F102 | Receive tagging | Received files are tagged with the sending device's fingerprint; quarantine applies unless the device is marked "my own device". | M | 2 |
| IN03-F103 | Visibility control | Discoverable: off, paired devices only, everyone on this network for 10 min. The device name shown is a setting and defaults to a generic one. | M | 2 |
| IN03-F104 | Pairing facility for other apps | IPC interface `org.pbsd.transfers.pairing` v1 lets IN01 (sync) and AX05 (clipboard) reuse paired device keys with the user's consent per app. | S | 3 |
| IN03-F105 | Interoperability | The protocol is documented so a mobile client can be written. No compatibility with proprietary nearby-sharing systems. | C | 3 |

### 4.7 Queue, scheduling and bandwidth

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F106 | One queue | Every transfer of every kind is a row: name, kind, direction, size, progress, speed, remaining time, route, state, trust badge. | M | 1 |
| IN03-F107 | States | Queued, connecting, running, paused, waiting (schedule, route, space, network), verifying, done, failed, quarantined-held. Each state has an icon and a word. | M | 1 |
| IN03-F108 | Controls | Start, pause, resume, stop, remove (keep or delete data), retry, move up or down, move to top; all work on multi-selection. | M | 1 |
| IN03-F109 | Concurrency | Max active downloads (default 4), uploads (2), torrents (5 downloading, 5 seeding); per-host limit. | M | 1 |
| IN03-F110 | Priorities | High, normal, low; share bandwidth 4:2:1 when a limit is active. | S | 2 |
| IN03-F111 | Global speed limits | Download and upload caps; presets in the status bar chip (unlimited, 75%, 50%, 25% of the measured peak, custom). | M | 1 |
| IN03-F112 | Alternative limits by schedule | A weekly grid (hour cells) of normal, limited and stopped. | S | 2 |
| IN03-F113 | Per-transfer and per-kind limits | Caps on one transfer, one site, one torrent, or one kind. | S | 2 |
| IN03-F114 | Network conditions | Pause on metered links, on battery below a threshold, or when a named network is not connected; resume alone. | S | 2 |
| IN03-F115 | Disk space guard | Refuses to start or pauses when free space after reservation would fall under a threshold (default 2 GiB or 5%). | M | 1 |
| IN03-F116 | Completion actions | Notify; when the queue is empty: nothing, quit the app, or request sleep through `svc.power` (never power off). | S | 2 |
| IN03-F117 | Background running | With `bg.run`, transfers continue with no window; a shell indicator shows activity; `svc.power` sleep inhibit only while the user set it. | M | 1 |
| IN03-F118 | History | Finished and removed transfers kept in a searchable history (default 90 days) with provenance. | S | 2 |
| IN03-F119 | Statistics | Totals per day, kind and route; current speed graph (`chart`), 10 min window. | C | 2 |
| IN03-F120 | Search and filter | Filter by kind, state, route, category, text. | M | 1 |

### 4.8 Routes and network safety

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F121 | Route per transfer | Clear, a VPN profile or Tor, from SE03. Defaults per kind (torrents: VPN) and per category rule. | M | 2 |
| IN03-F122 | Route chip | Every row shows its route in words and colour `--net`; a mismatch with the default for its kind shows a `--warn` marker. | M | 2 |
| IN03-F123 | Fail closed | A transfer never changes route by itself. Route down means "waiting: route". | M | 2 |
| IN03-F124 | One helper per route | Protocol helpers are started per route and hold sockets for that route only. | M | 2 |
| IN03-F125 | DNS | Names resolve on the transfer's route through the network service. | M | 2 |
| IN03-F126 | Listening ports | Shown per route with state (open, closed, unknown). Random port per session unless fixed. | S | 2 |
| IN03-F127 | Endpoint list | `[net]` badge lists hosts and peer counts per route, with bytes. | M | 1 |
| IN03-F128 | Plain-text protocol warnings | FTP and HTTP rows carry a "not encrypted" label; passwords over them need confirmation. | M | 1 |

### 4.9 Integration with the browser and other apps

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F129 | Download intent | IPC interface `org.pbsd.transfers.submit` v1: URL, suggested name, headers, cookies for the request, referrer, container name, route, destination handle, expected hash. Returns a `transfer_id` and progress signals. | M | 1 |
| IN03-F130 | Browser progress mirror | IN01's download bubble subscribes to state for the transfers it submitted. | M | 2 |
| IN03-F131 | Scheme and type handling | Handles `magnet:`, `sftp:`, `ftp:`, `ftps:`, `dav:`, `davs:`, `s3:`, `rsync:`, and opens `.torrent`, `.metalink`, `.meta4`. | M | 2 |
| IN03-F132 | Share target | Appears in the share sheet as "Send to device" and "Upload to site…". | S | 2 |
| IN03-F133 | Hand-off to other apps | Open with the default app (through quarantine rules), Show in Files, Extract with UT01, Verify with SE05. | M | 1 |
| IN03-F134 | Feeds enclosures | IN02 may submit large enclosures; state is mirrored back. | C | 3 |
| IN03-F135 | Command line | `transfers add <url>`, `--route`, `--dir`, `--sha256`; `transfers list`; `transfers pause|resume <id>`; talks to the running instance over the session bus. | S | 2 |

### 4.10 Sites, bookmarks and credentials

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F136 | Site manager | Saved sites in folders: name, protocol, host, port, user, authentication kind, key reference, route, local and remote start folders, limits, notes, colour. | M | 2 |
| IN03-F137 | Credentials in the keyring | Passwords, tokens and S3 secret keys live in `svc.keyring` under this app; the site store holds only references. | M | 2 |
| IN03-F138 | Path bookmarks | Per site: pairs of local and remote folders. | S | 3 |
| IN03-F139 | Import sites | From FileZilla `sitemanager.xml` and OpenSSH `config` and `known_hosts` files picked in the portal. Passwords in imported files are moved to the keyring and the user is told to delete the source. | S | 3 |
| IN03-F140 | Export sites | JSON without secrets. | S | 3 |
| IN03-F141 | Known hosts manager | List of SSH host keys and TLS pins with first-seen date; remove; compare with a pasted fingerprint. | M | 2 |
| IN03-F142 | Per-site capability | Saving a site records it; the first connection asks the shell for `net.host:<host>:<port>` so later runs do not prompt. Quick connects use `net.user`. | M | 2 |

### 4.11 Settings, accessibility and recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN03-F143 | Preferences | Pages: General, Downloads, Verification, Torrents, Remote sites, Device send, Schedule, Routes, Shortcuts. | M | 1 |
| IN03-F144 | Crash-safe state | Transfer state is journalled; after a crash or power cut every transfer resumes from verified data; torrents do a fast resume check. | M | 1 |
| IN03-F145 | Helper restart | A crashed protocol helper is restarted and its transfers retried up to 3 times; then they fail with the helper's last error. | M | 1 |
| IN03-F146 | Progress announcements | Start, completion and failure are announced; progress percentages on request (`Ctrl+Shift+I` reads the selected row). | M | 1 |
| IN03-F147 | Notifications | Per kind: on completion, on failure, on verification result, on incoming device offer. Grouped. | M | 1 |
| IN03-F148 | Export and backup | Queue, history, sites (no secrets) and jobs export as JSON; included in `svc.backup`. | S | 2 |

## 5. GUI

### 5.1 Window map

```
Transfers window
├─ Header bar: menu, title, search, [net] badge
├─ Sidebar (F9): All · Downloading · Uploading · Seeding · Done · Failed · Held (quarantine)
│               · kinds (Web, Torrents, Sites, Jobs, Devices) · categories · Sites tree · Devices
├─ Content (one of)
│   ├─ Queue view (default): toolbar, transfer table, detail tabs
│   ├─ Two-pane remote view: tabs per connection, local pane, remote pane, log panel
│   ├─ Jobs view: job list, job editor, run reports
│   └─ Devices view: device tiles, send drop zone, incoming offers
├─ Inspector (F4): General · Provenance · Verification · Files · Trackers · Peers · Mirrors · Log
└─ Status bar: speeds, limit chip, route states, free space, schedule state
Dialogs and sheets
├─ Add download · Add torrent · Create torrent · Verify · Connect to site · Site manager
├─ Host key prompt · Conflict · Folder comparison · Job editor · Pair device · Incoming offer
├─ Schedule grid · Known hosts · Preferences
└─ Shell-drawn: file chooser, secret prompt, capability prompts, share sheet
```

### 5.2 Main window

Shown at 1280×800, queue view, inspector closed.

```
+----------------------------------------------------------------------------------------+
|[=] Transfers                              [search ⌘K]              [net] [▁] [▢] [✕]  | (1)
+------------------+---------------------------------------------------------------------+
| All           12 | [+ Add v] [▶][⏸][■] [↑][↓] [Verify] [Open folder]     [filter…  ] | (2)
| Downloading    3 +---------------------------------------------------------------------+
| Uploading      1 | Name               Kind  Size    Progress      Speed   ETA   Route  | (4)
| Seeding        2 | > image.iso  ◐held  Web   3.4 G  ▓▓▓▓░░ 62%   8.1 M/s 2m40  Clear  |
| Done           5 |   distro.torrent    Torr  1.9 G  ▓▓▓▓▓░ 88%   3.0 M/s 1m10  VPN    |
| Failed         1 |   site-backup/      SFTP  412 M  ▓▓░░░░ 31%   1.2 M/s 4m02  Clear  |
| Held           2 |   photos (14)       Dev   96 M   ▓▓▓▓▓▓ done  —       —     LAN    |
| ---------------- |   paper.pdf  ✓ver   Web   840 K  ▓▓▓▓▓▓ done  —       —     Tor    |
| Web  Torrents    |   old.zip    ! fail Web   120 M  checksum mismatch            Clear |
| Sites  Jobs      |                                                                     |
| Devices          +---------------------------------------------------------------------+
| ---------------- | [General][Provenance][Verification][Files][Mirrors][Log]            | (5)
| ▾ Sites          | Source  https://mirror.example.org/pub/image.iso                    |
|   web server     | Via     2 mirrors · 4 segments · TLS 1.3 · cert 9F:2C:…             |
|   bucket-eu      | Hash    SHA-256 pending (62%) · expected 3b1f…e09a (from Metalink)  |
| ▾ Devices        | State   Quarantined until verified                                  |
|   laptop ●       | Segments ▓▓▓▓▓░░▓▓▓░░░▓▓▓▓░░▓▓▓▓▓▓░                                 |
+------------------+---------------------------------------------------------------------+
| ↓ 12.3 M/s ↑ 1.1 M/s [Limit: none v] · VPN up · Tor up · Free 212 G · Schedule: normal | (6)
+----------------------------------------------------------------------------------------+
  (3) sidebar
```

States: empty, loading (restoring the queue), error (row-level), offline (all rows "waiting: network"), route down (rows "waiting: route"), read-only (not applicable), narrow.

1. Header bar. `[net]` lists endpoints by route with byte counts.
2. Toolbar: Add (`toolButton` with menu: URL, torrent or magnet, connect to site, send to device, new job), start, pause, stop, move up and down, Verify, Open folder, filter `lineEdit`.
3. Sidebar 220 px: `treeView` with counts; Sites and Devices subtrees; a green dot plus the word "online" in the tooltip for reachable devices.
4. Transfer table: `tableView`, virtualised, sortable, column chooser. Progress is a `progressBar` in the cell. Trust badge after the name: held (quarantined), verified, failed. Route cell uses `--net`.
5. Detail tabs (`tabWidget`, 220 px high, collapsible): same content as the inspector for the selected row. The segment map is a custom-drawn strip (`viewportWidget` or the planned canvas).
6. Status bar: speeds (tabular figures), limit `comboBox` chip, route states from SE03, free space of the default destination, schedule state.

Narrow (<640): sidebar overlays; table shows name, progress and state only; details open as a sheet.

### 5.3 Other views and dialogs

#### 5.3.1 Add download (dialog, 600×460)

```
+----------------------------------------------------------------------+
| Add downloads                                                        |
| Addresses (one per line)                                             |
| [ https://mirror.example.org/pub/image.iso                         ] |
| [ https://mirror.example.org/pub/SHA256SUMS                        ] |
+----------------------------------------------------------------------+
| ✓ image.iso      3.4 GiB  application/x-iso9660-image  ranges: yes   |
| ✓ SHA256SUMS     1.1 KiB  text/plain  → will be used to verify       |
+----------------------------------------------------------------------+
| Save to  [ ~/Downloads               ] [Choose…]   Route [ Clear v ] |
| Segments [ 4 ]   Priority [ Normal v ]   [x] Hold until verified     |
| Expected hash [ sha256:…                                  ] optional |
| ▸ Request details (headers, referrer, login)                         |
|                              [Cancel] [Add paused] [Download 2 files]|
+----------------------------------------------------------------------+
```

`dialog`, `codeEdit` in plain mode for the address box, `listView` for probe results, `lineEdit`, `spinBox`, `comboBox`, `checkBox`. States: probing, probe failed (row shows the error; Add still allowed), HTTP-only warning row, duplicate of an existing transfer.

#### 5.3.2 Add torrent (sheet, 720×560)

```
+----------------------------------------------------------------------------------+
| Add torrent: distro-15.0-amd64                                                   |
| Info-hash v1 6a3f…  v2 91bc…   · 1.9 GiB · 3 files · 4 trackers · public         |
+-------------------------------------------+--------------------------------------+
| [x] ▾ distro-15.0-amd64            1.9 G  | Route   [ VPN home-vpn v ]  ● up     |
| [x]     disc1.iso          High    1.8 G  |   Exit address shown by Anonymity    |
| [x]     SHA256SUMS         Normal  1 K    |   [!] Clear route exposes your       |
| [ ]     README.txt         Skip    4 K    |       address to every peer          |
|                                           | Save to [ ~/Downloads/torrents ] […] |
|                                           | [ ] Start paused  [ ] Sequential     |
|                                           | Seeding [ Until ratio 1.0 v ]        |
|                                           | Encryption [ Prefer v ]  DHT [on ]   |
+-------------------------------------------+--------------------------------------+
|                                                    [Cancel] [Add torrent]        |
+----------------------------------------------------------------------------------+
```

`treeView` with check boxes and a priority `comboBox` column; `comboBox`, `checkBox`, `toggleSwitch`. For a magnet link the tree shows "Fetching metadata…" after the user confirms the route, never before.

#### 5.3.3 Two-pane remote view (window 1280×800)

```
+----------------------------------------------------------------------------------------+
| [web server ×] [bucket-eu ×] [+]                        sftp · ED25519 SHA256:k3…  ▣  |
+-------------------------------------------+--------------------------------------------+
| Local  [ ~/Sites/blog            ] [↑][⟳] | Remote [ /var/www/blog          ] [↑][⟳]  |
| Name            Size   Modified           | Name            Size   Modified    Perm    |
| ▸ assets/       —      10 Nov             | ▸ assets/       —      02 Nov      755     |
| > index.html    12 K   12 Nov   newer     |   index.html    11 K   01 Nov      644     |
|   feed.xml      48 K   12 Nov   only here |   robots.txt    1 K    14 Jan      644     |
|   style.css     6 K    01 Nov   same      |   style.css     6 K    01 Nov      644     |
| 3 files, 1 folder · 66 K                  | 3 files, 1 folder · 18 K · quota 61%       |
+-------------------------------------------+--------------------------------------------+
| [Upload →] [← Download] [Compare] [Sync browse] [New folder] [Delete]                  |
+----------------------------------------------------------------------------------------+
| Log  > SSH2_FXP_OPENDIR /var/www/blog  < OK  …                              [▾ hide]   |
+----------------------------------------------------------------------------------------+
```

`tabBar` per connection with the host key fingerprint and a lock; two `tableView`s in a `splitter`; path `lineEdit`s (a breadcrumb widget when it exists); log in a read-only `codeEdit`. Comparison marks are words in a column, with colour as a second cue. States: connecting, authentication failed, host key changed (blocking panel), disconnected (panes dimmed, "Reconnect"), plain FTP (red bar across the remote pane).

#### 5.3.4 Host key prompt (dialog, 520×320)

```
+------------------------------------------------------------+
| Trust this server's key?                                   |
| Host        files.example.org:22  (198.51.100.7)           |
| Key         ssh-ed25519                                    |
| Fingerprint SHA256:k3Jd…9QxA                               |
| Route       Clear                                          |
| Compare this fingerprint with one you got from the server's|
| owner by another channel.                                  |
| Paste to compare [ SHA256:…                    ]  ✓ match  |
|                [Cancel] [Trust once] [Trust and remember]  |
+------------------------------------------------------------+
```

Changed-key variant: title "The server's key has changed", shows old and new fingerprints and first-seen date, buttons "Disconnect" (default) and "Open known hosts…". No accept button in that dialog.

#### 5.3.5 Devices view (content area, 1060×700)

```
+--------------------------------------------------------------------------------------+
| This device: "desktop" · discoverable to paired devices            [Visibility v]    |
+--------------------------------------------------------------------------------------+
| Paired                                   | On this network (not paired)              |
| +-------------+  +-------------+         | +-------------+                           |
| |  laptop  ●  |  |  phone   ○  |         | | device-7f3a |   [Pair…]                 |
| |  own device |  |  last: Mon  |         | +-------------+                           |
| +-------------+  +-------------+         |                                           |
+--------------------------------------------------------------------------------------+
|        Drop files on a device, or  [Choose files…]  [Send clipboard text]            |
+--------------------------------------------------------------------------------------+
| Incoming: "laptop" wants to send 14 files (96 MiB)  holiday-01.jpg, … [Decline][Accept]|
+--------------------------------------------------------------------------------------+
```

`gridView` of device tiles, drop zone `panel`, offers as `panel` rows.

#### 5.3.6 Pair device (dialog, 460×340)

```
+--------------------------------------------------+
| Pair with "device-7f3a"                          |
| Check that both devices show the same code:      |
|                                                  |
|                  4 1 7 · 9 0 2                   |
|                                                  |
| Key fingerprint  B3:91:0C:…:7E                   |
| [x] This is my own device (skip quarantine)      |
|            [They differ — cancel] [They match]   |
+--------------------------------------------------+
```

Code in 24 pt monospace. "They match" is not the default button; neither is; the user must choose.

#### 5.3.7 Verify (dialog, 560×380)

```
+--------------------------------------------------------------+
| Verify image.iso                                             |
| Computed  SHA-256  3b1f9c…e09a                    [Copy]     |
|           BLAKE3   77a0d2…41bc                    [Copy]     |
| Expected  [ 3b1f9c…e09a                         ]  ✓ match   |
|           [Load checksum file…]                              |
| Signature [ SHA256SUMS.asc ] [Choose…]                       |
|   ✓ Good signature by "Release Key <rel@example.org>"        |
|     key 4A1B … 9F20 · trust: marked by you on 2026-03-02     |
| Result: file matches a checksum list signed by a trusted key |
|                     [Keep held] [Release from quarantine…]   |
+--------------------------------------------------------------+
```

Release opens SE05's flow; Transfers cannot release by itself.

#### 5.3.8 Schedule grid (preferences page)

```
        00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23
  Mon   █  █  █  █  █  █  ▒  ▒  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ▒  ▒  ▒  ▒  █  █
  Tue   █  █  █  █  █  █  ▒  ▒  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ▒  ▒  ▒  ▒  █  █
  …     █ normal   ▒ limited (↓ 2 M/s ↑ 256 K/s)   · stopped        [Paint: normal v]
```

Custom grid on `gridView` cells; keyboard: arrows move, `1`/`2`/`3` paint.

### 5.4 Menus

```
File      Add download… Ctrl+N · Add torrent or magnet… Ctrl+T · Connect to site… Ctrl+Shift+N ·
          Quick connect Ctrl+L · Send to device… Ctrl+D · New job… · Create torrent… ·
          Import sites… · Export… · Close tab Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Copy Ctrl+C · Copy source address Ctrl+Shift+C (queue view) · Paste address Ctrl+V ·
          Select all Ctrl+A · Find Ctrl+F · Preferences Ctrl+,
View      Sidebar F9 · Inspector F4 · Details panel Ctrl+B · Log panel Ctrl+Shift+L · Columns… ·
          Queue Ctrl+1 · Sites Ctrl+2 · Jobs Ctrl+3 · Devices Ctrl+4 · Refresh F5 · Hidden files Ctrl+. · Full screen F11
Transfer  Start Ctrl+Enter · Pause Space · Stop · Retry Ctrl+R · Move up Ctrl+Up · Move down Ctrl+Down ·
          Move to top Ctrl+Home · Priority ▸ · Limit speed… · Route ▸ · Verify… Ctrl+Shift+V ·
          Force recheck · Force re-announce · Open Enter · Open folder Ctrl+Shift+O ·
          Remove Delete · Remove and delete data Shift+Delete
Site      Upload F6 (two-pane) · Download F7 · New folder Ctrl+Shift+F · Rename F2 · Delete Delete · Permissions… ·
          Compare folders Ctrl+Shift+C · Synchronised browsing · Edit remote file F3 · Bookmark path ·
          Disconnect · Reconnect · Site manager… · Known hosts…
Tools     Pause all Ctrl+Shift+P · Resume all · Speed limits… · Schedule… · History Ctrl+Y · Statistics ·
          Paired devices…
Help      Transfers help F1 · Keyboard shortcuts · About
```

In the two-pane view `Tab` switches pane and `Ctrl+Shift+C` is Compare folders; "Copy source address" keeps that shortcut only in the queue view.

### 5.5 Keyboard and pointer

Context menus: transfer row (Start, Pause, Retry, Priority, Route, Limit, Verify, Open, Open folder, Copy address, Copy hash, Copy magnet link, Provenance, Remove, Remove and delete data); torrent file row (Priority, Skip, Open, Rename); peer row (Ban, Copy address); remote file (Download, Download to…, Queue, Open, Edit, Rename, Delete, Permissions, Copy path, Copy URL, Create presigned link (S3), Properties); local file (Upload, Queue, Rename, Delete to trash, Show in Files); site (Connect, Connect in new tab, Edit, Duplicate, Delete); device (Send files, Send clipboard, Rename, Mark as own device, Revoke pairing).

Drag and drop: URLs, `.torrent` and Metalink files onto the window open the matching Add dialog; files from Files onto a remote pane upload; from a remote pane to Files downloads (a promise-style drop: the file appears when done, quarantined); between panes copies; onto a device tile sends; rows in the queue reorder. Dropping onto a plain-FTP pane asks for confirmation.

Pointer: double-click a done row opens through quarantine rules; middle click on a site opens a new tab; the speed chip scrolls through presets with the wheel.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | "Nothing is transferring. Add a download, open a torrent, connect to a site or send to a device." Four buttons. |
| Loading | Queue restores from the journal; rows show "checking" until resume data is validated. |
| Error | The row turns to "failed" with a one-line cause; "Details" opens the Log tab; Retry is the primary action. |
| Offline | Rows wait with "waiting: network"; local-device send still works on a LAN with no internet. |
| Route down | Rows on that route wait; status bar names the route; "Open Anonymity" link. |
| Disk full | Affected rows pause with "waiting: space"; a banner shows free space and a link to UT04 Disk Usage. |
| Held | Done but quarantined: badge and "Verify" and "Release…" actions. |
| Locked | If the keyring is locked, connections that need secrets wait and a banner offers "Unlock". |
| Narrow | See 5.2. |

### 5.7 Widgets

| Region | FUSE widget | Note |
|---|---|---|
| Sidebar, torrent files, site manager | `treeView` | check boxes and inline `comboBox` cells (FUSE-WP-28) |
| Queue, panes, peers, trackers, history | `tableView` | progress cell renderer; column chooser |
| Detail and inspector tabs | `tabWidget`, `propertyGrid` | |
| Segment and piece maps | `viewportWidget` now; 2D canvas (G-12) later | IN03-GAP-1 small "piece map" widget |
| Speed graph, statistics | `chart` | |
| Device tiles, schedule grid | `gridView` | |
| Log | `codeEdit` (read-only) | |
| Forms and dialogs | `dialog`, `lineEdit`, `spinBox`, `comboBox`, `checkBox`, `toggleSwitch`, `radioButton`, `progressBar`, `fileDialog` (through the portal) | |

Gaps: **IN03-GAP-1** piece-map widget; **IN03-GAP-2** a capability for peer-to-peer networking (many unnamed outbound peers, an inbound listening port and UDP) scoped to one route; PLATFORM section 6 has only `net.host` and `net.user`; **IN03-GAP-3** a capability and service for local-link discovery and listening (multicast DNS), separate from internet access; **IN03-GAP-4** a platform-wide provenance and quarantine tag format (extended attribute names, schema, who may clear them), owned by SE05 and SY02 but needed here first; **IN03-GAP-5** an SSH agent interface in SE04 so keys never enter client apps; **IN03-GAP-6** promise-style drag and drop (file delivered later) in the shell.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Queue, scheduler, rules, limits allocation, site and job stores, view models, IPC interfaces. |
| UI | FUSE UI | app host | Section 5. |
| HTTP transfer helper | C++23 (safe subset) on the platform protocol helper | one per route | HTTP(S) downloads and uploads, segments, mirrors, WebDAV and S3 requests. |
| Torrent helper | C++23 (safe subset), own implementation | one per route in use | Peer wire, DHT, trackers, uTP, piece store, v1 and v2 hashing. Writes only inside the destination handles it was given. |
| SSH helper | C++23 wrapper around an SSH library (6.3) | one per connection | SFTP, SSH transport for rsync. Signing delegated to SE04's agent. |
| FTP helper | C++23 (safe subset), own code | one per connection | FTP and FTPS control and data channels. |
| rsync helper | C++23 wrapper or port (6.3) | one per job run | rsync protocol client. |
| Device-send helper | C++23 (safe subset), own code | one, only while discoverable or sending | Discovery, pairing key exchange, encrypted channel, chunking. |
| Metadata parse helper | C++23 (safe subset) | on demand, no network | Bencode (`.torrent`), Metalink XML, checksum files, FileZilla XML, OpenSSH config. |
| Hash helper | C++23 (safe subset) | pool | Hashing large files off the UI thread; torrent creation. |

```
 IN01, IN02, share sheet ──(org.pbsd.transfers.submit)──┐
                                                        ▼
 app host: queue + scheduler (ForgeScript) + UI (FUSE) + SQLite journal
   ├─ HTTP helper [Clear]   ├─ HTTP helper [VPN]     ── network service ── routes (SE03)
   ├─ torrent helper [VPN]  (peers, DHT, trackers; no other route)
   ├─ SSH helper (per connection) ── SE04 agent (signing)     ├─ FTP helper
   ├─ rsync helper ── SSH helper or rsync daemon              ├─ device-send helper ── local link only
   ├─ metadata parse helper (no net)   ├─ hash helper (no net)
   └─ svc.intent ─► SE05 (verify, release) · UT01 (extract) · SY02 (show)
```

Helpers receive destination directories as handles with write-only-inside rights; they cannot read the rest of the user's files. Bandwidth is allocated by the app host as token budgets sent to helpers every 250 ms.

### 6.2 Data model

Store: `~/.local/share/org.pbsd.transfers/transfers.db` (SQLite, WAL) plus per-transfer state files beside the data. `schema_version` in `meta`.

| Entity | Fields |
|---|---|
| `transfer` | `id text` (ULID), `kind enum{http,torrent,sftp,ftp,ftps,webdav,s3,rsync,device}`, `direction enum{down,up,both}`, `name text`, `state enum`, `state_reason text`, `size u64?`, `done u64`, `priority u8`, `position u32`, `route enum`, `route_profile text?`, `dest_handle text`, `category_id u32?`, `requesting_app text?`, `created i64`, `started i64?`, `finished i64?`, `down_limit u32?`, `up_limit u32?`, `hold_until_verified bool`, `error text?` |
| `http_source` | `transfer_id`, `url text`, `mirror_rank u8`, `etag text?`, `last_modified text?`, `accepts_ranges bool`, `headers json?` (dropped at completion), `bytes u64`, `failures u16` |
| `segment` (in `.part.state`) | `offset u64`, `length u64`, `done u64`, `source_idx u8` |
| `torrent` | `transfer_id`, `info_hash_v1 blob20?`, `info_hash_v2 blob32?`, `private bool`, `piece_length u32`, `pieces_have bitmap` (in resume file), `uploaded u64`, `downloaded u64`, `ratio_limit f32?`, `seed_time_limit_s u32?`, `sequential bool`, `encryption enum{off,prefer,require}`, `dht bool`, `pex bool` |
| `torrent_file` | `transfer_id`, `index u32`, `path text`, `size u64`, `priority u8`, `done u64` |
| `tracker` | `transfer_id`, `tier u8`, `url text`, `status text`, `peers u32`, `next_announce i64` |
| `site` | `id u32`, `folder text`, `name text`, `protocol enum`, `host text`, `port u16`, `user text`, `auth enum{password,key,agent,token,anonymous}`, `keyring_ref text?`, `key_ref text?`, `route enum`, `local_dir_handle text?`, `remote_dir text`, `max_conn u8`, `legacy_algos bool`, `s3_region text?`, `s3_path_style bool`, `colour u8`, `notes text` |
| `known_host` | `host text`, `port u16`, `key_type text`, `fingerprint_sha256 blob32`, `first_seen i64`, `kind enum{ssh,tls_pin}` |
| `job` | `id u32`, `name text`, `kind enum{rsync,mirror}`, `source text`, `dest text`, `site_id u32?`, `direction enum`, `delete_extra bool`, `excludes text[]`, `checksum bool`, `schedule json?`, `route enum`, `limit_kbps u32?`, `last_run i64?`, `last_result text?` |
| `job_run` | `job_id`, `started i64`, `duration_s u32`, `files u32`, `bytes u64`, `deleted u32`, `errors u32`, `dry_run bool`, `report_path text` |
| `device` | `id blob32` (public key), `name text`, `own bool`, `auto_accept_dir_handle text?`, `paired i64`, `last_seen i64`, `revoked bool` |
| `provenance` | `transfer_id`, `file_path text`, `record json` (also written to the file's extended attribute), `sha256 blob32`, `blake3 blob32`, `verified enum{none,checksum,signature,chain,failed}`, `verifier_detail json` |
| `category`, `rule`, `schedule_cell`, `history` | as named in 4.1, 4.7 |

Provenance extended attribute (Proposed, pending IN03-GAP-4): name `user.pbsd.provenance`, value compact JSON, at most 4 KiB; quarantine flag `user.pbsd.quarantine` = `held|released|own-device` with the release record signed by SE05.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| HTTP/1.1, HTTP/2, TLS 1.3 | yes | yes | FORGE-WP-21 protocol helper (Proposed) | HTTP helper |
| Metalink 4 (RFC 5854), Metalink/HTTP (RFC 6249) | yes | — | own code | metadata parse helper |
| Checksum lists (BSD and GNU styles) | yes | yes | own code | metadata parse helper |
| SHA-2, SHA-3, BLAKE2b, BLAKE3, MD5 and SHA-1 (compare only) | yes | yes | FORGE BLAKE3 and Monocypher BLAKE2b (in the FORGE tree); SHA-2/3 from the platform crypto library (FORGE-WP-22, Proposed) | hash helper |
| OpenPGP, minisign, signify, SSH signatures | verify | — | SE05 and SE04; not in this app | SE05's sandbox |
| Bencode, `.torrent` v1/v2, magnet URI | yes | yes | own code | metadata parse helper |
| BitTorrent peer wire, DHT (BEP 5), UDP trackers (BEP 15), PEX (BEP 11), uTP (BEP 29), v2 (BEP 52), web seeds (BEP 19) | yes | yes | own implementation; an established open-source BitTorrent library is used only as a differential test peer | torrent helper |
| SSH 2, SFTP v3 with common extensions | yes | yes | libssh2 (BSD-3-Clause) as the first choice, confined; a safe-subset SSH transport is a later goal shared with SY03, DV03 and VM04. To verify: its post-quantum key exchange support and offline build on HardenedBSD | SSH helper |
| FTP (RFC 959, 2228, 2389, 2428, 3659, 4217) | yes | yes | own code; TLS from the platform library | FTP helper |
| WebDAV (RFC 4918) | yes | yes | own code on the HTTP helper; own XML parser | HTTP helper, XML in metadata parse helper |
| S3-compatible REST API, signature v4 | yes | yes | own code | HTTP helper |
| rsync protocol | yes | yes | openrsync (ISC licence) as the base, confined; to verify: protocol version coverage against current rsync servers | rsync helper |
| Local device send v1 (Proposed, this spec) | yes | yes | own code: multicast DNS service discovery, CPace or SPAKE2 key exchange (choice to make with the platform crypto owners), Ed25519 identities, XChaCha20-Poly1305 frames | device-send helper |
| FileZilla `sitemanager.xml`, OpenSSH `config`, `known_hosts` | yes | — | own code | metadata parse helper |

### 6.4 Integration

- Intents handled: `open` for the schemes and types of F131; `share` (send to device, upload to site). Emitted: `open`, `view`, `share`, `scan` (QR pairing), and requests to SE05 (verify, release) and UT01 (extract).
- MIME: view and import `application/x-bittorrent`, `application/metalink4+xml`, `application/metalink+xml`; export `application/x-bittorrent`, `application/json`.
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.intent`, `svc.keyring`, `svc.clipboard` (opt-in), `svc.power`, `svc.backup`, `svc.audit`, `svc.accounts` (S3 and WebDAV endpoints may be listed there), `svc.undo-trash` (local deletes).
- IPC offered: `org.pbsd.transfers.submit` v1 (F129), `org.pbsd.transfers.pairing` v1 (F104), `org.pbsd.transfers.remote-fs` v1 (list, stat, read, write on a saved site; for SY02 Files to show remote locations without its own protocol code; Proposed, to agree with SY02).
- Other apps: IN01 (downloads), IN02 (enclosures), SY02 (show, remote locations), SE03 (routes), SE04 (keys, agent, trust store), SE05 (verify, quarantine), UT01 (extract), OF01 (edit remote file), ME03 and ME02 (open while downloading), ME04 (QR), AX05 (pairing reuse), UT04 (disk full).
- Backing repos: none. Verified facts relied on: BLAKE3, Ed25519 and XChaCha20-Poly1305 exist in the FORGE tree (cited in `FORGE_BUILD_PLAN.md` from `FORGE:NOTICE` and `FORGE:docs/security.md`; not re-read here). AEGIS's "Mode 2" bulk plane (`ANONYMOUS:README.md`) is a possible future route for large files between consortium members; no integration is planned in this spec.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Store, journals, cache for remote edits. |
| `fs.portal` | Destination folders, upload sources, watch folders, job folders. Grants are remembered per folder. |
| `net.user` | URLs, quick connects and magnet links the user supplies. First contacts logged. Torrent peers are covered only loosely by this capability: IN03-GAP-2. |
| `net.host:<host>:<port>` | Added per saved site at first connection. |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-transfer routes; one helper per route. |
| `proc.helper:http`, `:torrent`, `:ssh`, `:ftp`, `:rsync`, `:device-send`, `:parse`, `:hash` | Section 6.1. |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.intent`, `svc.keyring`, `svc.clipboard`, `svc.power`, `svc.backup`, `svc.audit`, `svc.accounts`, `svc.undo-trash` | Section 6.4. |
| `bg.run` | Transfers continue with no window. Asked at first use. |
| `bg.autostart` | Only if the user enables "resume transfers at login" or scheduled jobs. |

Threat model:

| | |
|---|---|
| Assets | The user's real network address (torrents); server credentials and SSH keys; files being sent and received; the file system outside the chosen folders; the integrity of what was downloaded. |
| Adversaries | Malicious servers, mirrors and peers; a network attacker on the path; a hostile torrent or Metalink file; a hostile device on the local network; malware in downloaded content; a peer trying to learn the address behind a VPN. |
| Attack surface | Protocol parsers in every helper; file names and paths from remote ends (traversal, device names, symlinks); bencode and XML; the listening sockets of the torrent and device-send helpers; the submit interface; SSH and rsync library code. |

Mitigations:

- Every protocol runs in a helper that holds only its sockets (from the network service, bound to one route) and write access inside the destination handle. Remote names are normalised; `..`, absolute paths, symlink escapes and reserved names are refused; writes use handle-relative opens (`openat` under Capsicum), so a traversal cannot leave the folder even if a check is missed.
- Torrent leak defence in depth: route-bound sockets, fail closed, no port mapping off-route, leak self-test (F038), and the firewall's per-app rule from SE02 as an outer wall.
- Server identity: TLS validation, SSH host-key pinning with a hard stop on change, optional pins.
- Private keys stay in SE04; passwords are typed into shell prompts or come from the keyring straight to the helper.
- Everything received is quarantined with provenance; release is SE05's decision, not this app's.
- Device send: explicit pairing with a compared code, mutual authentication, consent per offer, discovery off by default.
- The submit interface accepts only schema-checked requests, shows the requesting app on the row, and cannot choose a destination outside handles the caller owns or the user's default folder.
- Third-party library code (SSH, rsync) is T4 and is the reason those helpers get one process per connection and nothing but one socket and one folder.

Stored: section 6.2. Sent: only the transfers the user asked for. Torrent swarms see the route's exit address, the port and a peer id. Device discovery announces a device name on the local link while enabled. No telemetry, no tracker or mirror lists fetched in the background, no version checks.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `downloads.dir` | portal handle | `~/Downloads` | Default destination. |
| `downloads.segments` | int 1–8 | 4 | F003. |
| `downloads.per_host` | int 1–8 | 4 | Connections per host. |
| `downloads.retries` | int 0–100 | 20 | F007. |
| `downloads.hold_until_verified` | bool | false | Global default of F028. |
| `verify.auto_fetch_checksum_files` | bool | false | F025: fetch sibling checksum files without asking. |
| `queue.max_downloads`, `queue.max_uploads` | int, int | 4, 2 | F109. |
| `queue.min_free_gib` | int 0–1000 | 2 | F115. |
| `limits.down_kbps`, `limits.up_kbps` | int, int | 0, 0 | 0 means unlimited. |
| `limits.alt_down_kbps`, `limits.alt_up_kbps` | int, int | 2048, 256 | Used in "limited" schedule cells. |
| `schedule.enabled` | bool | false | F112. |
| `network.pause_on_metered` | bool | true | F114. |
| `network.pause_below_battery_pct` | int 0–100 | 20 | 0 disables. |
| `route.default_http`, `route.default_torrent`, `route.default_sites` | enum | clear, vpn, clear | F121. |
| `torrent.allow_clear_route` | bool | false | Must be true before the clear route can be picked for a torrent. |
| `torrent.port` | int 0–65535 | 0 | 0 means random per session. |
| `torrent.dht`, `torrent.pex`, `torrent.lsd`, `torrent.utp` | bool ×4 | true, true, false, true | F040–F043. |
| `torrent.encryption` | enum off/prefer/require | prefer | F042. |
| `torrent.ratio_limit`, `torrent.seed_minutes` | float, int | 1.0, 0 | F047. |
| `torrent.max_active_down`, `torrent.max_active_seed`, `torrent.max_connections` | int ×3 | 5, 5, 200 | F049. |
| `sites.max_connections` | int 1–10 | 4 | F081. |
| `sites.idle_disconnect_min` | int 0–120 | 10 | 0 never. |
| `sites.show_hidden` | bool | false | F080. |
| `device.name` | string | "computer" | Name shown to others. |
| `device.visibility` | enum off/paired/everyone_10min | off | F103. |
| `device.own_devices_skip_quarantine` | bool | true | F030. |
| `history.keep_days` | int 0–3650 | 90 | F118. |
| `clipboard.watch` | bool | false | F018. |
| `background.resume_at_login` | bool | false | Needs `bg.autostart`. |

## 9. Accessibility and languages

- Progress is exposed as value and text ("62 percent, 8.1 megabytes per second, 2 minutes 40 seconds left") on each row; the piece map has a text summary ("412 of 1 024 pieces, 6 gaps").
- The two-pane view announces which pane has focus and the comparison word for each row.
- Pairing codes are read digit by digit, and the dialog offers "Read code again".
- Byte units follow the locale and a setting (binary KiB/MiB or decimal kB/MB); speeds use the same base.
- File names from remote systems may be in any encoding: invalid UTF-8 is shown with replacement marks and the raw bytes are kept for the transfer; right-to-left names are isolated so they cannot reorder the extension (bidi override characters are shown as visible marks).

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame | 150 ms; queue of 10 000 rows restored within 400 ms |
| HTTP throughput | limited by disk or link up to 1 GiB/s on loopback tests; hashing on the fly at 1 GiB/s or more (BLAKE3), SHA-256 at hardware speed |
| Torrent session | 500 torrents loaded, 50 active, 2 000 connections, under 400 MiB in the helper |
| Directory listing | 100 000 entries shown in under 1 s after receipt; scroll frame 4 ms |
| UI update rate | at most 4 updates per second per row; zero frames when nothing changes |
| Recheck | 1 GiB/s or disk speed, whichever is lower |
| Device send | within 10% of raw link speed on gigabit Ethernet |
| Memory, app host | 120 MiB with 10 000 history rows |

## 11. Testing and proof

Nothing has run. Targets only.

- Unit: queue ordering and limits allocation, scheduler grid, retry back-off, name sanitiser, conflict rules, rule matching, magnet parsing, site URL parsing.
- Golden renders: every view and dialog of section 5, three themes.
- Scripted UI through the a11y tree: tasks T1–T13.
- Protocol tests against local test servers started in jails: HTTP with ranges and faults (truncation, changed ETag, slow mirrors), FTP and FTPS, SFTP, WebDAV, an S3-compatible server, an rsync daemon, a tracker and a swarm of reference BitTorrent clients.
- Fuzz: bencode, Metalink, checksum files, FTP listings, HTTP responses, peer wire messages, DHT packets, uTP packets, device-send frames, SFTP packets (at the wrapper boundary), the submit interface.
- Differential: torrent piece hashing and v2 Merkle roots against a reference implementation; rsync results against reference rsync on a file-tree corpus; checksum outputs against known vectors.
- Leak tests: packet capture on all interfaces while a VPN-bound torrent runs and the route is cut, restored and cut again; zero packets off-route, including DNS, DHT and tracker traffic.
- Path-escape tests: hostile torrents, archives of listings and rsync file lists with traversal names; no write outside the destination handle.
- Power-cut tests: kill at random points; every transfer resumes with verified data.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Name and path sanitiser | Output path is always inside the destination | T1 | PRISM proof over the normalisation function | — |
| Bencode, Metalink, checksum parsers | No runtime errors; bounded memory | T2 | PRISM; fuzz | — |
| Segment and piece bookkeeping | Bytes marked done were written and hashed; no overlap | T1 target | PRISM model of the segment map; property tests | Proof effort |
| Torrent helper wire and DHT code | No runtime errors on hostile input | T2 | PRISM; fuzz; swarm tests | Size of the implementation |
| Route binding | Sockets exist only on the bound route | T1 (inherited from FORGE-WP-21) | As FORGE-WP-21; leak tests | Route mechanism Unverified |
| Device-send protocol | Key exchange and transcript checks as specified | T1 for the state machine, T2 for code | Model check of the protocol; PRISM | Choice of key exchange not final |
| FTP helper | No runtime errors | T2 | PRISM; fuzz | — |
| SSH helper (library) | None provable for the library | T4 (wrapper T2) | Fuzz, sanitiser builds, confinement tests | Third-party C code |
| rsync helper (ported code) | None provable initially | T4 | Differential and fuzz | Third-party C code |
| App logic | Memory safety by construction | T3 | ForgeScript guarantees; unit tests | — |

## 12. Build plan

| ID | Work item | Depends on |
|---|---|---|
| | **Phase 1: usable core (web downloads)** | |
| IN03-W01 | Store, journal, queue model, limits allocator | G-02, G-07 |
| IN03-W02 | HTTP helper: resume, retries, name handling, on-the-fly hashing | FORGE-WP-21, FORGE-WP-22, G-08 |
| IN03-W03 | Queue UI, sidebar, detail tabs, status bar, add dialog | G-02, FUSE-WP-28 |
| IN03-W04 | Provenance record and quarantine tag | IN03-GAP-4, SE05 |
| IN03-W05 | Checksum verification dialog | W02 |
| IN03-W06 | `org.pbsd.transfers.submit` interface; browser hand-off | G-08, IN01 |
| IN03-W07 | Background running, notifications, crash-safe resume | G-09, FORGE-WP-11 |
| | **Phase 2: full feature** | |
| IN03-W08 | Segments, mirrors, Metalink, checksum-file detection | W02 |
| IN03-W09 | Signature verification through SE05 | SE05, SE04 |
| IN03-W10 | Routes per transfer, per-route helpers, fail closed, leak tests | SE03, IN01-GAP-4 |
| IN03-W11 | Torrent helper: metadata, peer wire, pieces, trackers | W10, IN03-GAP-2 |
| IN03-W12 | Torrent helper: DHT, PEX, uTP, encryption, v2 and hybrid, seeding rules | W11 |
| IN03-W13 | Torrent UI: add sheet, files, trackers, peers, piece map | W11, IN03-GAP-1 |
| IN03-W14 | SSH helper with SE04 agent; host-key store; SFTP | SE04, IN03-GAP-5 |
| IN03-W15 | FTP and FTPS helper; WebDAV on the HTTP helper | W02 |
| IN03-W16 | Two-pane view, site manager, conflict rules, raw log | W14, W15 |
| IN03-W17 | Device send: discovery, pairing, channel, devices view | IN03-GAP-3, FORGE-WP-22 |
| IN03-W18 | Scheduling grid, priorities, network conditions, history | W01, `svc.power` |
| | **Phase 3: advanced** | |
| IN03-W19 | S3-compatible client with multipart resume | W02 |
| IN03-W20 | rsync helper, jobs view, dry run, schedules | W14 |
| IN03-W21 | Folder comparison, synchronised browsing, remote edit, site-to-site | W16, OF01 |
| IN03-W22 | Create torrent, watch folders, web seeds, super-seeding, block lists | W12 |
| IN03-W23 | Pairing interface for IN01 and AX05; QR pairing; resume of sends | W17, ME04 |
| IN03-W24 | `remote-fs` interface for SY02 | W16, SY02 |
| IN03-W25 | Safe-subset SSH transport to replace the library (shared work) | PRISM, owners of SY03, DV03, VM04 |

## 13. Open questions and risks

1. PLATFORM section 6 has no capability that fits BitTorrent (IN03-GAP-2) or local-link discovery (IN03-GAP-3). Until it does, the manifest over-asks with `net.user`.
2. The route mechanism (per-app FIB, jail or proxy) is owned by SE02, SE03 and the OS and is marked Unverified in `FORGE_BUILD_PLAN.md` (FORGE-WP-21 risks). UDP and inbound ports over a route are needed for torrents and may not be offered.
3. SE03's backing repo is a consortium mixnet (`ANONYMOUS:README.md`). A general VPN and Tor are assumed to come from SE03 as well; not established here.
4. An own BitTorrent implementation is large. The alternative is an established C++ library confined in the helper at T4; decide after W11.
5. SSH and rsync rely on third-party C code at first. The choice of SSH library should be made once for the whole suite.
6. The quarantine tag format (IN03-GAP-4) must be fixed before phase 1 ends or IN01 and IN03 will diverge.
7. Extended attributes do not survive every file system or archive; the store keeps a copy keyed by hash, but a file moved to a FAT drive loses its tag.
8. Legal: the app is a general transfer tool; no content indexes or search of torrents are included.
9. The device-send key exchange (CPace or SPAKE2) needs review by whoever owns platform cryptography (FORGE-WP-22).
