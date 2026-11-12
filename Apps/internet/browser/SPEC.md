<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Web Browser

By Odin Loch

| | |
|---|---|
| ID | `IN01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.browser` |
| Category | internet |
| Priority | P1 |
| Backing repos | FUSE (chrome; `webSurface` is planned in FUSE-WP-51, not built). FORGE (app host, network service FORGE-WP-21, planned). ANONYMOUS (route provider behind SE03). No web engine exists in any repo read. |
| Status | Planned. Nothing built. |

## 1. Purpose

The Web Browser shows web pages on an OS that forbids JIT compilers, writable-executable memory and Qt. It is for every user of the desktop, and it replaces Firefox, Chromium, Falkon and Tor Browser as packaged on the Plasma plan. Each site runs in its own process with its own capability set; identities are separated by container, each with its own storage and network route. This spec also carries the engineering analysis for platform gap G-14 (section 6.5).

## 2. Scope

In scope:

- Browsing with tabs, windows, containers, private windows and disposable sessions.
- The engine integration: content, network and helper processes, and their confinement.
- Blocking of trackers and adverts, fingerprinting resistance, HTTPS-only, DNS policy as seen by the browser.
- Site permissions, mapped to device portals.
- Bookmarks, history, search engines, reader mode, page archive, translation, find, print to PDF.
- Autofill of passwords and passkeys from SE04 over a trusted path.
- Developer tools, kiosk mode, policy files, session restore, sync between the user's own devices.

Out of scope:

| Item | Owner |
|---|---|
| Storing passwords, passkeys, TOTP, certificates of the user | SE04 Passwords and Keys |
| Running Tor, VPN and mixnet routes; route health | SE03 Anonymity |
| System firewall and system DNS resolver | SE02 Firewall |
| Download queue, resume, segmented and torrent transfers | IN03 Transfers (the browser starts a download and hands it off) |
| Scanning, quarantine release, encryption of downloaded files | SE05 Vault |
| PDF viewing and annotation | OF03 Document Viewer |
| Feed subscription | IN02 Feeds (browser detects feeds and hands off) |
| Video and audio outside a page | ME03 Video, ME02 Music |
| Capability prompts, file chooser, share sheet, screen picker | SH04 Portals and Prompts |
| Restricted HTML in mail and help | CO01, UT05 through FUSE-WP-42 (section 6.5 track A) |

## 3. Users and top tasks

Personas:

- **Everyday user**: reads news, banks, shops, watches video. Wants pages to work and to be told the truth about what a site can do.
- **Privacy-focused user**: keeps identities apart, uses Tor for some of them, never wants a cross-identity leak.
- **Developer**: inspects pages, network and storage; tests local sites.
- **Administrator**: deploys kiosks and locked-down profiles by policy file.
- **Screen-reader user**: browses by headings, landmarks and links.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a site from the address bar | type, Enter |
| T2 | Search the web with a chosen engine | type, Enter (or keyword, Tab, type, Enter) |
| T3 | Open a link in another container | right click, pick container (2) |
| T4 | See and change what this site may do | click site badge, flip a toggle (2) |
| T5 | Sign in with a saved password or passkey | focus field, pick entry in the trusted popup, confirm (3) |
| T6 | Download a file and find it | click link, toast "Show in Transfers" (2) |
| T7 | Read an article without clutter | `Ctrl+Alt+R` (1) |
| T8 | Save a page for offline reading | `Ctrl+Shift+S` (1) |
| T9 | Find a tab among 200 | `Ctrl+Shift+A`, type, Enter (3) |
| T10 | Open a suspicious link safely | right click, "Open in disposable session" (2) |
| T11 | Clear everything a site stored | site badge, "Forget this site" (2) |
| T12 | Translate a page offline | click translate badge, "Translate" (2) |
| T13 | Bookmark a page with tags | `Ctrl+D`, type tags, Enter (3) |
| T14 | Restore the last session after a crash | start the app (0; automatic) |
| T15 | Send a tab to another of my devices | right click tab, "Send to device", pick (3) |

## 4. Features

Priority: M must, S should, C could. Phase: 1 usable core, 2 full feature, 3 advanced.

### 4.1 Navigation and address bar

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F001 | Address bar | One `lineEdit` for URLs and searches. Shows scheme, registrable domain in `--fg-0`, rest in `--fg-1`. Never hides the domain. | M | 1 |
| IN01-F002 | Input classification | Input with a dot or scheme and no space is a URL; otherwise a search. `?text` forces search; `!` prefix forces URL. No request is sent while typing. | M | 1 |
| IN01-F003 | Local suggestions | Dropdown from bookmarks, history, open tabs and search keywords. Ranked by frecency. Each row labelled by source. | M | 1 |
| IN01-F004 | Remote suggestions off | Search-engine suggestions are off by default; enabling is per engine and per container and shows a `[net]` note. | M | 2 |
| IN01-F005 | IDN display | Mixed-script and confusable labels are shown as punycode; whole-script labels in the user's languages are shown in Unicode. Rule table is a setting. | M | 1 |
| IN01-F006 | Back, forward, reload, stop | `Alt+Left`, `Alt+Right`, `F5`, `Esc`. Long press on Back shows the tab's history list. `Ctrl+F5` reloads without cache. | M | 1 |
| IN01-F007 | Home and new-tab page | Local page: search field, pinned sites, recent bookmarks. No remote content, no sponsored tiles. Can be blank. | M | 1 |
| IN01-F008 | Copy clean link | Copies the URL without known tracking parameters (`utm_*`, `fbclid`, `gclid`, list is updatable). | S | 2 |
| IN01-F009 | Security indicator | Left of the address: site badge with icon and word: Secure, Not secure, Local file, Internal page, Certificate error. Click opens the site panel (5.3.1). | M | 1 |
| IN01-F010 | Container and route chip | Right of the badge: container name and colour, and route (Clear, VPN, Tor). Always visible. | M | 1 |
| IN01-F011 | Link target preview | Hovering or focusing a link shows the real target in the status overlay; punycode rule of F005 applies. | M | 1 |
| IN01-F012 | Scheme hand-off | `mailto:`, `tel:`, `magnet:`, `geo:`, `feed:` and others go to `svc.intent` after a prompt naming the target app. A page cannot trigger this without a user gesture. | M | 2 |
| IN01-F013 | Internal pages | `about:` pages for settings, bookmarks, history, downloads list, permissions, processes, policies, certificates, blocked items. Rendered by FUSE UI, not by the engine. | M | 1 |
| IN01-F014 | Open local files | `file:` only through a portal handle. A local page gets a unique opaque origin and no access to other files. | M | 2 |

### 4.2 Tabs and windows

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F015 | Horizontal tab strip | `tabBar` under the header bar: favicon, title, audio icon, container colour stripe, close button. Scrolls when full. | M | 1 |
| IN01-F016 | Vertical tabs | Sidebar `treeView` (220 px, collapsible to 40 px icons). Toggle `Ctrl+Alt+V`. | M | 2 |
| IN01-F017 | Tree tabs | In vertical mode a tab opened from a link is a child of its opener. Collapse and expand subtrees; close subtree; promote. | S | 2 |
| IN01-F018 | Tab groups | Named, coloured groups; collapse to a chip; move group to a window; save a group as a bookmark folder. | S | 2 |
| IN01-F019 | Pinned tabs | Icon-only, leftmost, kept across restarts, protected from "close others". | M | 1 |
| IN01-F020 | Tab search | `Ctrl+Shift+A`: `commandPalette`-style list over title, URL, container and group of all windows. Also finds recently closed tabs. | M | 1 |
| IN01-F021 | Reopen closed | `Ctrl+Shift+T` reopens closed tabs and windows in order, with history and scroll. Last 50 kept; none kept for private windows. | M | 1 |
| IN01-F022 | Tab unloading | Background tabs idle for 30 min (setting) have their content process ended and state serialised. Shown dimmed. Reload on focus. Pinned and audible tabs are exempt. | M | 2 |
| IN01-F023 | Multi-select tabs | Shift and Ctrl select; actions apply to the set: close, move, group, bookmark, reload, send to device, change container (reopen). | S | 2 |
| IN01-F024 | Drag tabs | Reorder; drag out to make a window; drag between windows; drag to an edge for split view (DESIGN_SYSTEM section 3). | M | 2 |
| IN01-F025 | Split view | Two tabs side by side in one window with a `splitter`. Each keeps its own address bar row. | C | 3 |
| IN01-F026 | Mute and audio indicator | Speaker icon on audible tabs; click mutes. Global "mute all background tabs". | M | 2 |
| IN01-F027 | Open-link placement | New tabs open next to the opener (setting: at end). Background by default with middle click. | M | 1 |
| IN01-F028 | Pop-up policy | `window.open` without a user gesture is blocked and counted in the site badge. With a gesture it opens a tab, never a chromeless window. | M | 1 |
| IN01-F029 | Window types | Normal, private, disposable, kiosk, developer tools. Each has a distinct header bar tint and word label so they cannot be confused. | M | 1 |

### 4.3 Engine, processes and site isolation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F030 | No-JIT execution | Script runs in an interpreter only. The engine build has no code generator linked in. A start-up self-test maps a W+X page and expects failure; on success the browser refuses to run. | M | 1 |
| IN01-F031 | WebAssembly policy | Off by default. May be on per site only if the engine has a pure interpreter for it (to verify per engine, 6.5). Never compiled. | M | 2 |
| IN01-F032 | Site isolation | One content process per site (scheme plus registrable domain) per container. Cross-site frames run in the frame site's process if the engine supports out-of-process frames (to verify, 6.5); otherwise cross-site frames are blocked by default with click-to-load. | M | 1 |
| IN01-F033 | Content process sandbox | Capsicum capability mode before any page byte is parsed. Holds only: IPC socket, shared-memory surface, read-only fonts and resources. No file system, no sockets, no devices. | M | 1 |
| IN01-F034 | Network process per container | One network helper per container. Only it holds sockets. It enforces route, HTTPS-only, blocking rules, cookie jar and cross-origin read rules for its content processes. | M | 1 |
| IN01-F035 | Per-site capability set | Each site has a record of granted permissions. The broker passes a device or file handle to the site's process only while a grant is live; revoking closes the handle (SI-6). | M | 1 |
| IN01-F036 | Process limit and reuse | Soft limit on content processes (default 64). Above it, same-site tabs share a process; different sites never share. | M | 2 |
| IN01-F037 | Crash containment | A crashed content process shows a "This page stopped" card in its tab only, with Reload. Three crashes in 60 s stop auto-reload. | M | 1 |
| IN01-F038 | Hang detection | A content process that does not answer input for 5 s gets a "Page is not responding: Wait / Stop" bar. Stop kills the process. | M | 1 |
| IN01-F039 | Resource limits | Per content process: memory cap (default 2 GiB), CPU share for background tabs, descriptor cap, no core dumps. Set with `rctl`/`setrlimit` by the broker. | M | 2 |
| IN01-F040 | Process viewer | `about:processes`: tree of broker, network, content, decoder processes with site, container, memory, CPU, handles held, uptime; End process. | S | 2 |
| IN01-F041 | Decoders out of process | Images, fonts, audio and video are decoded in helper processes with no network (image and font decode helpers; media via G-11). Content processes receive pixels or validated tables. | M | 2 |
| IN01-F042 | GPU access policy | Content processes never open the GPU. Raster is CPU in the content process or done by a compositing helper. WebGL and WebGPU are off; a later software path is C. | M | 1 |
| IN01-F043 | Engine version and integrity | `about:engine` shows engine name, version, build flags (JIT absent), patch level and the signature state of every engine binary. | M | 1 |
| IN01-F044 | Engine update channel | Engine security updates arrive through `svc.update` as a separate signed package so they can ship within days of an upstream fix. Browser warns when the engine is older than 30 days past a known fix. | M | 1 |

### 4.4 Containers, identities and routes

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F045 | Containers | Named identities with colour and icon. Each has its own cookies, storage, cache, HSTS state, permissions, connection pool, TLS session cache and network process. Nothing is shared between containers. | M | 1 |
| IN01-F046 | Default containers | Personal, Work, Banking, Shopping, plus Disposable. User can add, rename, recolour, delete. | M | 1 |
| IN01-F047 | Route per container | Clear, VPN (named profile) or Tor, chosen from the routes SE03 offers. The network process socket is bound to that route by the network service; it cannot open any other. | M | 2 |
| IN01-F048 | Fail closed | If the route is down, requests fail with a route error page. No fallback to another route. | M | 2 |
| IN01-F049 | Site-to-container rules | "Always open example.org in Banking". A navigation to that site from another container asks once, then follows the rule. | S | 2 |
| IN01-F050 | First-party partitioning | Inside a container, third-party cookies are blocked and all storage, cache and connections are keyed by top-level site. | M | 1 |
| IN01-F051 | Tracking parameter stripping | Known tracking query parameters are removed on navigation and on copy. List is a signed data file. | S | 2 |
| IN01-F052 | New identity | One action per container: close its tabs, end its processes, erase its storage, request a new circuit from SE03 (Tor routes). | M | 2 |
| IN01-F053 | Per-container settings | Script on or off, blocking level, fingerprint level, search engine, HTTPS-only, user agent family, language list. | S | 2 |
| IN01-F054 | Temporary containers | "Open in new temporary container": a container that is deleted when its last tab closes. | S | 2 |
| IN01-F055 | Tor container hardening | A container on a Tor route forces: fingerprint level Strict, letterboxing, script off for HTTP, no WebRTC, no remote fonts, English locale spoof option. | M | 3 |
| IN01-F056 | Onion sites | `.onion` names resolve only inside a Tor-route container; elsewhere they give an explanatory error. Onion-Location header shown as an offer, never auto-followed. Depends on SE03 offering Tor (to verify, section 13). | S | 3 |

### 4.5 Content blocking and fingerprinting resistance

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F057 | Request blocking | Filter lists in Adblock Plus syntax, matched in the network process before any connection or DNS query. | M | 1 |
| IN01-F058 | Cosmetic filtering | Element-hiding rules injected as a user style sheet in the content process. No script injection. | S | 2 |
| IN01-F059 | Bundled lists | Lists ship with the package (EasyList, EasyPrivacy, a malware-domain list; licences checked at packaging, to verify). Updates are signed data packages through `svc.update`, not fetched by the browser. | M | 1 |
| IN01-F060 | Blocking levels | Off, Standard (trackers), Strict (trackers, adverts, third-party scripts and frames). Per container and per site. | M | 1 |
| IN01-F061 | Custom rules and lists | User rules editor with syntax check; import a list from a file; subscribe to a URL only with an explicit `[net]` grant. | S | 2 |
| IN01-F062 | Blocked-items panel | Per page: counts and a `tableView` of blocked requests (URL, type, rule, list). Allow once, allow on this site, copy rule. | M | 2 |
| IN01-F063 | Script control | Per site: all script, first-party only, none. Default all in Standard; first-party only in Strict. | M | 1 |
| IN01-F064 | Fingerprint levels | Standard: reduce entropy (round timers to 1 ms with jitter, fixed font list, generic GPU strings, 2 CPU cores reported, no battery, no sensor APIs). Strict: uniform values for all users, letterboxing, canvas read-back prompt. | M | 2 |
| IN01-F065 | Letterboxing | Content size is rounded down to steps of 100×100 px; the margin is drawn by the chrome. On in Strict. | S | 3 |
| IN01-F066 | Canvas and audio read-back | Reads of canvas pixels and audio buffers return per-site, per-session noise (Standard) or need a permission (Strict). | M | 2 |
| IN01-F067 | Font exposure | Pages see only the bundled font set plus fonts needed for the user's languages. Local font enumeration API is absent. | M | 2 |
| IN01-F068 | User agent | One fixed string per engine release for all users. No OS version, no "HardenedBSD" token unless the user opts in. Client hints are not sent. | M | 1 |
| IN01-F069 | Referrer policy | Cross-site referrers are trimmed to origin; none across containers; none from private windows to other sites (setting). | M | 1 |
| IN01-F070 | Do-not-track signals | Global Privacy Control header on by default. The old DNT header is off (it adds entropy). | S | 2 |
| IN01-F071 | WebRTC address policy | No local addresses exposed. Media only through the container's route. In Tor containers WebRTC is absent. | M | 2 |
| IN01-F072 | Removed APIs | Not built or disabled: Battery, Bluetooth, USB, Serial, HID, NFC, generic sensors, Idle detection, Topics and other ad APIs, background sync, payment handler, FedCM. List shown in `about:engine`. | M | 1 |
| IN01-F073 | Storage limits and expiry | Site data of sites not visited for 30 days is deleted (setting). Bookmarked or pinned sites are exempt. | S | 2 |

### 4.6 Transport security, DNS and certificates

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F074 | HTTPS-only | Every navigation and subresource is upgraded to HTTPS. If HTTPS fails, a full-page interstitial offers "Continue to HTTP site once" or "for this site in this container". | M | 1 |
| IN01-F075 | Mixed content | Active mixed content is blocked. Passive mixed content is upgraded or blocked, never loaded over HTTP. | M | 1 |
| IN01-F076 | TLS floor | TLS 1.2 minimum, 1.3 preferred. Weak suites off. Hybrid post-quantum key exchange when the TLS library offers it (to verify with FORGE-WP-21's library choice). | M | 1 |
| IN01-F077 | HSTS | Preload list shipped as signed data; dynamic HSTS stored per container. | M | 1 |
| IN01-F078 | Certificate errors | Interstitial states the exact error in plain words. Bypass is not offered for HSTS sites or pinned sites. Bypass is per site, per container, for the session. | M | 1 |
| IN01-F079 | Certificate viewer | `about:certificate`: chain as a `treeView`, fields in a `propertyGrid`, fingerprints in monospace, SCT list, export PEM, "Pin this key". | M | 2 |
| IN01-F080 | Pinning | User pins (SPKI SHA-256) per site; shipped pins for the update server and bundled search engines. A pin failure is not bypassable. | S | 2 |
| IN01-F081 | Trust store | The system trust store managed in SE04. The browser reads it through the network service. User-added roots are marked in the site badge ("Verified by a locally added authority"). | M | 1 |
| IN01-F082 | Certificate change notice | Optional: warn when a site's key changes before the old certificate expired (trust on first use), per site. | C | 3 |
| IN01-F083 | Revocation | OCSP stapling required when the certificate says must-staple. No plain OCSP queries (privacy). CRL sets as signed data updates. | S | 2 |
| IN01-F084 | Certificate Transparency | Require SCTs for publicly trusted certificates; log list is signed data. | S | 3 |
| IN01-F085 | DNS policy | The browser never resolves names itself. The network service resolves on the container's route with the resolver SE02 sets. The site panel shows which resolver class was used (system, encrypted, Tor exit). | M | 1 |
| IN01-F086 | No speculative connections | No DNS prefetch, preconnect, prerender or link prefetch. Hover does nothing on the network. | M | 1 |
| IN01-F087 | Local network guard | A public site may not request private, link-local or loopback addresses unless the user grants "Local network" to that site. | M | 2 |

### 4.7 Site permissions

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F088 | Permission model | Two gates: the browser holds the device capability (shell prompt, PLATFORM section 6), and the site holds a per-site grant (browser prompt bar). Both must be true. | M | 1 |
| IN01-F089 | Prompt bar | Non-modal bar under the address bar: "example.org wants to use your microphone" with Allow once, Allow this session, Always, Block. Needs a user gesture on the page; no prompt on load. | M | 1 |
| IN01-F090 | Camera and microphone | Mapped to `svc.portal.camera` / `.mic`. Device handle is passed to a capture helper, frames go to the site's process. Shell indicator and tab icon while live. | M | 2 |
| IN01-F091 | Screen sharing | Mapped to `svc.portal.screen`; the shell picker chooses window or screen every time; no persistent grant. | S | 3 |
| IN01-F092 | Location | Mapped to `svc.portal.location`. Per site: Block, Coarse (rounded to 0.1 degree), Precise. Default Block; "Always" is not offered for Precise. | M | 2 |
| IN01-F093 | Notifications | Per site, delivered through `svc.notify` with the site domain and container shown. No background push service: notifications only while a tab of the site is open. | S | 2 |
| IN01-F094 | Clipboard | Write on user gesture is allowed. Read needs a per-site grant and shows a toast on each read. | M | 2 |
| IN01-F095 | File access | Only files chosen in the `svc.portal.file` picker, read-only by default. Directory and write access are separate grants, per session. | M | 1 |
| IN01-F096 | Autoplay | Audible autoplay blocked by default; muted allowed; per-site override. | M | 1 |
| IN01-F097 | Pop-ups, redirects, downloads | Per-site Allow or Block for pop-ups, for automatic redirects after 0 s, and for more than one automatic download. | M | 1 |
| IN01-F098 | Full screen and pointer lock | Allowed on gesture; a shell-drawn banner names the site for 3 s; `Esc` always exits. Keyboard lock is not offered. | M | 2 |
| IN01-F099 | Protected media (DRM) | Not supported. No closed decryption module is shipped or loaded. Stated on the error card. | M | 1 |
| IN01-F100 | Permissions page | `about:permissions`: `tableView` of site × permission × container × expiry, filter, revoke, revoke all. Mirrored into AD05 Permissions as sub-grants of the browser. | M | 2 |
| IN01-F101 | Permission expiry | "Always" grants unused for 90 days are removed. | S | 2 |
| IN01-F102 | Permission in-use indicators | Icons in the address bar and on the tab for each device in use; click stops it. | M | 2 |

### 4.8 Downloads

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F103 | Start and hand-off | A response classed as a download is offered to IN03 Transfers with URL, headers needed to resume, cookies scoped to that request, container and route. The browser's stream is used if IN03 is absent. | M | 1 |
| IN01-F104 | Destination | Default `~/Downloads` through a remembered portal grant; "Ask each time" setting uses the save dialog. | M | 1 |
| IN01-F105 | Provenance record | Stored with the file as extended attributes: source URL, referrer, top-level site, container, route, time, TLS certificate fingerprint, SHA-256, content type as sent and as sniffed. | M | 1 |
| IN01-F106 | Quarantine | Every download is tagged quarantined. Opening a quarantined file goes through SE05 (scan, then "open in a disposable sandbox" or release). | M | 2 |
| IN01-F107 | Dangerous types | Executables, scripts, archives with executables, disk images and shortcut files get a warning row and are never opened automatically. | M | 1 |
| IN01-F108 | Download bubble | `popover` from the toolbar button: last 5 items with progress, Open, Show in Files, Show in Transfers, Copy link, Verify checksum. | M | 1 |
| IN01-F109 | Checksum and signature check | Paste a hash or pick a `.sig`/`.asc`/`.minisig` file; verification is done by SE05. Result stored in the provenance record. | S | 2 |
| IN01-F110 | Drive-by limits | One automatic download per navigation without a gesture; others need consent (F097). Sandboxed frames cannot download. | M | 1 |
| IN01-F111 | Save page | Save as: single-file archive (F128), HTML only, text, PDF. | M | 2 |
| IN01-F112 | Private downloads | Files stay; the list entry is removed when the private window closes; provenance omits the referrer. | M | 2 |

### 4.9 Passwords, passkeys and autofill

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F113 | No built-in password store | All credentials live in SE04. The browser holds none on disk. | M | 1 |
| IN01-F114 | Trusted-path fill | Focusing a login field shows a key icon. Activating it asks the shell to draw the picker (outside the page and outside the browser's surfaces). The chosen secret goes from `svc.keyring` to the site's content process field directly. | M | 2 |
| IN01-F115 | Origin binding | The picker lists only entries whose stored origin matches the top-level origin and the frame origin. Cross-origin frames are not filled without a second confirmation naming both origins. | M | 2 |
| IN01-F116 | No fill on load | Nothing is filled until the user picks an entry. Hidden fields are never filled. | M | 2 |
| IN01-F117 | Save and update offers | After a successful form submit a shell-drawn offer "Save password for example.org in Passwords?" appears. The browser passes the candidate to SE04 and forgets it. | M | 2 |
| IN01-F118 | Passkeys (WebAuthn) | Platform authenticator is SE04; hardware tokens through SE04 too. The browser passes the client data and origin; SE04 shows the consent prompt and signs. | M | 2 |
| IN01-F119 | Insecure form warning | A password field on an HTTP page or posting to HTTP shows an inline warning and disables fill. | M | 1 |

### 4.10 Reading, finding and translating

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F120 | Find in page | `Ctrl+F` bar: match case, whole word, regular expression (linear-time matcher), count, highlight all, next `Ctrl+G`. | M | 1 |
| IN01-F121 | Zoom | Page zoom 30–500% per site and container; text-only zoom option; `Ctrl++`, `Ctrl+-`, `Ctrl+0`. | M | 1 |
| IN01-F122 | Reader mode | `Ctrl+Alt+R`. Extraction runs in a helper on the already-loaded DOM text; output is shown by the restricted rich-content view (FUSE-WP-42), with no script and no network. | M | 2 |
| IN01-F123 | Reader styling | Font, size, line width, line height, theme (light, dark, sepia, high contrast), justified or ragged. | S | 2 |
| IN01-F124 | Local translation | Language is detected locally. Translation runs through `svc.model` (`ai.local`); the page text never leaves the machine. Offer bar only when a model for the pair is installed (AI02). | S | 3 |
| IN01-F125 | Local page summary | Optional summary of reader text through `svc.model`; shown in the inspector; clearly labelled as machine-written. | C | 3 |
| IN01-F126 | Force dark and user styles | Per-site "dark page" (colour inversion with image exemption) and user CSS per site (signed or user-typed, no script). | C | 3 |
| IN01-F127 | Text encoding override | Menu for legacy pages. | S | 2 |
| IN01-F128 | Offline page archive | `Ctrl+Shift+S` saves the page as a single-file archive: sanitised HTML, CSS, images, fonts, no script, plus a metadata record and optional full-page PNG. Stored in the archive library and indexed by `svc.search`. | M | 2 |
| IN01-F129 | Archive library | `about:archive`: list, tags, full-text search, open (rendered script-free in a no-network process), export as WARC or PDF, delete. | S | 2 |
| IN01-F130 | View source | `Ctrl+U` opens source in a `codeEdit` (read-only) tab; no request is re-sent. | S | 2 |
| IN01-F131 | Selection actions | Search for selection, copy as plain text, copy as Markdown, copy link to text fragment, send to OF02 Notes, look up in a local dictionary (G-17). | S | 2 |
| IN01-F132 | Feed detection | A feed icon appears when the page advertises RSS, Atom or JSON Feed; click hands the URL to IN02. | S | 2 |

### 4.11 Bookmarks, history and search engines

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F133 | Bookmark a page | `Ctrl+D` popover: title, folder, tags, container to open in, keyword. | M | 1 |
| IN01-F134 | Bookmarks bar | Optional row under the address bar; folders as `popupMenu`. `Ctrl+Shift+B`. | S | 1 |
| IN01-F135 | Bookmark manager | `about:bookmarks`: folder `treeView`, `tableView` of items, tags, search, sort, bulk edit, find duplicates, find dead links (manual, `[net]`). | M | 2 |
| IN01-F136 | Import and export | Netscape bookmark HTML, JSON; import from Firefox and Chromium profile folders picked in the portal (bookmarks, history; never cookies or passwords, which go through SE04's importer). | M | 2 |
| IN01-F137 | History | Per container store: URL, title, visit time, transition type. Kept 90 days by default. | M | 1 |
| IN01-F138 | History view | `about:history` and sidebar panel: by day, by site, search, delete item, delete site, delete range. | M | 1 |
| IN01-F139 | Forget this site | One action removes history, cookies, storage, cache, permissions, HSTS and certificate exceptions for a site in one or all containers. | M | 1 |
| IN01-F140 | Clear data dialog | Time range and data kinds; per container; "on exit" rules. | M | 1 |
| IN01-F141 | Search engines | Bundled set chosen for privacy policy; none is paid placement. First run asks the user to choose; no engine receives anything before that. | M | 1 |
| IN01-F142 | Keywords and bangs | `w term` style keywords per engine; user-defined engines with `%s`; OpenSearch description import on user action. | S | 2 |
| IN01-F143 | Search in container | Search requests go through the container's route and partition; a dedicated "Search" container can be set so searches are unlinkable from logged-in sites. | S | 2 |
| IN01-F144 | No search telemetry | No search codes, no partner identifiers, no ping on result click (`ping` attribute ignored, F072). | M | 1 |
| IN01-F145 | System search contribution | Bookmarks, archived pages and (optionally) history are offered to `svc.search`. Private and disposable data never. | S | 2 |
| IN01-F146 | Full-text history | Optional local index of visited page text for history search; off by default; never for private, Tor or Banking containers. | C | 3 |

### 4.12 Private windows, disposable sessions and profiles

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F147 | Private window | `Ctrl+Shift+N`. Storage in memory only (tmpfs in the network process); no history, no session restore, no search contribution; own temporary container per window. | M | 1 |
| IN01-F148 | Disposable session | `Ctrl+Shift+D` or context "Open in disposable session". A new browser instance in a fresh sandbox: empty profile on a temporary dataset, own broker, no access to the main profile, no SE04 autofill, clipboard paste-in only on confirmation. Destroyed on close. | M | 2 |
| IN01-F149 | Disposable from other apps | `svc.intent` `open` with the hint `disposable` lets Mail, Messenger and Files open untrusted links this way. | S | 2 |
| IN01-F150 | Profiles | Separate data directories with separate settings, containers and bookmarks. Profile manager at `about:profiles` and `--profile <name>`. Each profile is its own app instance. | S | 2 |
| IN01-F151 | Profile encryption | A profile can live in an SE05 vault container; the browser opens only when it is unlocked. | C | 3 |
| IN01-F152 | Guest mode | A profile that is erased on exit, with no bookmarks editing; used by kiosk mode too. | S | 3 |

### 4.13 Add-ons policy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F153 | No code extensions | Decision: the WebExtensions API is not offered. Reasons in section 7.4. | M | 1 |
| IN01-F154 | Declarative add-ons | Signed data-only bundles: filter lists, search engines, themes, reader themes, user style sheets, tracking-parameter lists, dictionaries. They contain no script. | S | 2 |
| IN01-F155 | Add-on manager | `about:addons`: installed bundles, signer, version, what each can affect, enable, remove. Installed through SY05 Software. | S | 2 |
| IN01-F156 | Built-in replacements | Functions people install extensions for are built in: blocking (4.5), containers (4.4), password fill (4.9), reader, translation, archive, tab tree, user styles, dark pages. | M | 2 |
| IN01-F157 | User scripts (restricted) | Could: per-site user scripts in an isolated world with no network, no cross-origin access and no privileged API; off by default; shown in the site panel. Decided after phase 2 review. | C | 3 |

### 4.14 Developer tools

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F158 | Tools window | `F12` or `Ctrl+Shift+I`. A FUSE UI window or dock speaking the engine's remote inspection protocol over a local IPC channel. No TCP listener. | S | 2 |
| IN01-F159 | Elements | DOM `treeView`, styles `propertyGrid`, computed values, box model, edit attributes and CSS live. | S | 2 |
| IN01-F160 | Console | Log view and an evaluate line (`codeEdit`); evaluation runs in the page's interpreter. | S | 2 |
| IN01-F161 | Network | `tableView` of requests: method, status, type, size, time, blocked-by rule, route; headers, body preview, timing; export HAR. | S | 2 |
| IN01-F162 | Accessibility inspector | The a11y subtree the page exposes, with role, name, state; contrast checker. | S | 3 |
| IN01-F163 | Security panel | Per origin: TLS version, suite, chain, HSTS, CSP, mixed content, isolation state, process id. | S | 2 |
| IN01-F164 | Tools disabled by policy | Policy `devtools.enabled=false`; always off in kiosk and disposable sessions. | M | 2 |

### 4.15 Print, capture and media

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F165 | Print | `Ctrl+P` opens the shell print dialog (`svc.portal.print`). The engine paginates to PDF (the spool format, G-16). | M | 2 |
| IN01-F166 | Save as PDF | Same path; PDF gets document metadata scrubbed except title. | M | 2 |
| IN01-F167 | Media playback | Audio and video in pages decode in the media helper (G-11) with open codecs first; `svc.media` shows now-playing and takes transport keys. | M | 2 |
| IN01-F168 | PDF in pages | PDFs are not rendered in the engine. They are downloaded (quarantined) and opened in OF03 in a disposable sandbox. | M | 2 |

### 4.16 Session, sync and data

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F169 | Session restore | Windows, tabs, groups, tree, containers, history per tab and scroll are journalled every 15 s and on change. After a crash everything returns; tabs load lazily. | M | 1 |
| IN01-F170 | Restore choice | Setting: restore always, ask, or start fresh. Crash-loop guard: after two failed restores the session opens with all tabs unloaded. | M | 1 |
| IN01-F171 | Own-device sync | Bookmarks, open tabs list, history (optional), settings, site rules. Never cookies; passwords are SE04's concern. End-to-end encrypted with a key shared by pairing code or QR. No account, no vendor server. | S | 3 |
| IN01-F172 | Sync transports | Local network between paired devices (using the pairing facility IN03 specifies), or a user-supplied WebDAV folder as a blind store of encrypted records. | S | 3 |
| IN01-F173 | Send tab to device | Encrypted one-shot message to a paired device over the same transports. | C | 3 |
| IN01-F174 | Conflict handling | Record-level last-writer-wins with tombstones; bookmarks merge by GUID; a merge log is viewable. | S | 3 |
| IN01-F175 | Storage report | `about:storage`: bytes per site per container, cache size, archive size; delete from here. | S | 2 |

### 4.17 Kiosk and policies

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F176 | Kiosk mode | `--kiosk <url>`: full screen, no address bar, guest profile, allow-list of sites, idle reset after N minutes, on-screen navigation bar optional, exit only by admin key chord plus authentication. | S | 3 |
| IN01-F177 | Policy files | Signed JSON in `/etc/pbsd/browser/policies/*.json` (read through `fs.path`), merged in order. Managed settings show a lock icon and the file that set them. | S | 2 |
| IN01-F178 | Policy coverage | Allow and block lists, home page, containers, routes, permissions defaults, downloads off, devtools off, private windows off or forced, add-on allow-list, certificate pins, extra roots. | S | 3 |
| IN01-F179 | Policy page | `about:policies` lists every active policy, source file, signer and errors. | S | 2 |

### 4.18 Accessibility, settings and recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN01-F180 | Page a11y tree | The engine's a11y tree is grafted under the `webSurface` node (FUSE-WP-51.3) and validated. | M | 1 |
| IN01-F181 | Caret browsing | `F7`: move a text caret through the page with the keyboard and select text. | S | 2 |
| IN01-F182 | Keyboard link hints | `Alt+;` shows letter hints on every link and control. | C | 3 |
| IN01-F183 | Minimum font size and forced colours | Settings; forced colours follows the system high-contrast theme. | M | 2 |
| IN01-F184 | Settings pages | `about:settings` with search; every setting of section 8. | M | 1 |
| IN01-F185 | Troubleshoot mode | Start with blocking off, add-ons off, default settings, one container; nothing is deleted. | S | 2 |
| IN01-F186 | Default browser | Registers `http`, `https` and HTML MIME types with `svc.intent`; a rule table chooses the container for links from other apps. | M | 1 |
| IN01-F187 | Command line | `--private`, `--disposable`, `--container <name>`, `--profile <name>`, `--kiosk <url>`, `--new-window`, URLs. | S | 1 |

## 5. GUI

### 5.1 Window map

```
Browser window (normal | private | disposable | kiosk)
├─ Header bar: menu, tab strip or title, command palette, [net] badge, device indicators
├─ Navigation row: back, forward, reload, site badge, container chip, address bar, page actions, toolbar buttons
│   ├─ Site panel (popover)            5.3.1
│   ├─ Address suggestions (popup list)
│   ├─ Page actions: reader, translate, feed, archive, bookmark star
│   ├─ Downloads bubble (popover)
│   └─ Blocked-items panel (popover)
├─ Bookmarks bar (optional)
├─ Prompt bar slot: permission, pop-up blocked, translate offer, hang, restore
├─ Sidebar (F9): vertical or tree tabs | bookmarks | history | archive | containers
├─ Content: webSurface (one per visible tab), find bar, status overlay, interstitials
├─ Inspector (F4): page info, summary, reader settings
└─ Developer tools dock (F12) or separate window
Dialogs and sheets
├─ Clear data · Forget this site · Bookmark editor · Container editor · New disposable session
├─ Certificate viewer · Tab search · Send to device · Pair device · Import data
└─ Shell-drawn (SH04): capability prompts, file chooser, print, screen picker, credential picker, passkey consent
Internal pages (FUSE UI in a tab): settings, bookmarks, history, permissions, processes, storage,
  addons, policies, profiles, archive, engine, certificate
```

### 5.2 Main window

Shown at 1280×800, wide layout, horizontal tabs, sidebar closed.

```
+----------------------------------------------------------------------------------------+
|[=] |▌News ×|▌Bank ×|▌Docs — spec ×| [+]            [search ⌘K]     [net] [▁] [▢] [✕]  | (1)
+----------------------------------------------------------------------------------------+
|[<][>][⟳] [▣ Secure] [Personal · Clear] [ https://example.org/articles/42          ] | (2)
|                                         [Rd][Tr][☆]  [◈ 12] [↓] [⋯]                  | (3)
+----------------------------------------------------------------------------------------+
| ▸ News  ▸ Work  ▸ Reference  example.org  wiki                                         | (4)
+----------------------------------------------------------------------------------------+
| example.org wants to use your microphone   [Allow once] [This session] [Always] [Block]| (5)
+----------------------------------------------------------------------------------------+
|                                                                                        |
|                          web content (webSurface)                                      | (6)
|                                                                                        |
|                                            +------------------------------------------+|
|                                            | Find: [ capability      ] 3 of 17 [^][v] || (7)
|                                            | [x] Case [ ] Word [ ] Regex [x] All  [✕] ||
|                                            +------------------------------------------+|
| https://example.org/next-page                                                          | (8)
+----------------------------------------------------------------------------------------+
```

States: loading (progress line under row 2, reload becomes stop), error page, interstitial, offline, crashed tab card, unloaded tab, private (header tinted `--secret`, word "Private"), disposable (header tinted `--warn`, word "Disposable"), policy-managed (lock icon in menu).

1. Header bar, 36 px. `tabBar` with a 3 px container colour stripe at the left of each tab. With vertical tabs on, this row shows the page title instead.
2. Navigation row, 40 px. `toolButton` ×3, site badge (`toolButton` with icon and word), container chip (`toolButton`, colour dot), address `lineEdit` that fills the row.
3. Page actions inside the address field's right edge (reader, translate, bookmark) and toolbar buttons: blocked count, downloads, overflow. All are `toolButton`.
4. Bookmarks bar, 28 px, optional. `toolButton` row; folders open `popupMenu`.
5. Prompt bar slot, 36 px, only when needed. One bar at a time; others queue with a count. `panel` with `label` and `button`s.
6. `webSurface` (FUSE-WP-51). Pixels come from the tab's content process; the chrome never paints inside it and the content cannot paint outside it.
7. Find bar, overlay at the top right of the content, 360×64. `lineEdit`, `checkBox` ×4, `toolButton`.
8. Status overlay, bottom left, shows a link target on hover or focus; drawn by the chrome, max half the width, elided in the middle of the path, never in the domain.

Narrow (<640): navigation row keeps back, badge, address, overflow; other buttons move to the overflow menu; sidebar is an overlay; tabs become a count button that opens tab search.

### 5.3 Other views and dialogs

#### 5.3.1 Site panel (popover from the site badge, 420×520)

```
+----------------------------------------------------------+
| example.org                               [Forget site…] |
| ▣ Connection is secure · TLS 1.3          [Certificate] |
| Container: Personal · Route: Clear · DNS: encrypted      |
| Process: 4812 (site-isolated) · 212 MiB                  |
+----------------------------------------------------------+
| Protection                                               |
|  Blocking       [Standard v]     12 blocked [Details]    |
|  Script         [All v]                                  |
|  Fingerprint    [Standard v]                             |
|  HTTPS-only     [on ]                                    |
+----------------------------------------------------------+
| Permissions                                              |
|  Microphone     [Ask v]          in use ● [Stop]         |
|  Camera         [Block v]                                |
|  Location       [Block v]   ( ) Coarse ( ) Precise       |
|  Notifications  [Ask v]                                  |
|  Clipboard read [Block v]                                |
|  Pop-ups        [Block v]        2 blocked [Show]        |
|  Local network  [Block v]                                |
|  [All permissions…]                                      |
+----------------------------------------------------------+
| Data: 3 cookies · 1.2 MiB storage          [Clear data]  |
+----------------------------------------------------------+
```

`popover` containing `label`s, `comboBox` per row, `toggleSwitch`, `link`, `button`. Changes apply at once; those needing a reload show a "Reload to apply" `link`. States: internal page (only the first block), certificate error (first block in `--danger`, no permissions), local file.

#### 5.3.2 Vertical tree tabs with sidebar (window 1280×800)

```
+--------------------------+-------------------------------------------------------------+
| [Tabs][Bkm][Hist][Arch]  | [<][>][⟳] [▣ Secure] [Work · VPN] [ https://…          ] |
| [ filter tabs…         ] +-------------------------------------------------------------+
| ▾ ▌Work (group, 5)       |                                                             |
|   > Tracker — issue 12 × |                                                             |
|     ▾ Review 88        × |                    web content                              |
|         Diff view      × |                                                             |
|     Wiki — design   ♪ × |                                                             |
| ▾ ▌Banking (1)           |                                                             |
|     My bank            × |                                                             |
| ▸ ▌Reading (14)          |                                                             |
|   Unloaded: Long article |                                                             |
| ------------------------ |                                                             |
| [+ New tab] [+ Group]    |                                                             |
+--------------------------+-------------------------------------------------------------+
```

`tabBar` for the four sidebar modes, `lineEdit` filter, `treeView` with inline close `toolButton`. Rows 24 px (compact) or 32 px. Unloaded tabs in `--fg-2` italics with the word "Unloaded". Drag a row onto another to make it a child; onto a group header to join.

#### 5.3.3 Certificate or HTTPS interstitial (in the content area)

```
+----------------------------------------------------------------------------------------+
|   !  This site's certificate is not valid for shop.example.net                         |
|   The certificate is for *.example.com. Someone may be intercepting the connection,    |
|   or the site is misconfigured.                                                        |
|   Error: name mismatch · Issuer: Example CA R3 · Valid: 2026-09-01 to 2026-11-30       |
|   Container: Shopping · Route: Clear                                                   |
|   [Go back]   [View certificate]   [Try again]                                         |
|   ▸ Advanced                                                                           |
|       [Continue once in this container]   (not offered: site uses HSTS)                |
+----------------------------------------------------------------------------------------+
```

Drawn by FUSE UI in the chrome process, not by the engine, so a page cannot imitate or script it. Primary button is "Go back". Variants: HTTP-only site, pin failure (no bypass), route down, DNS failure, blocked by policy, blocked by filter list, local-network guard.

#### 5.3.4 Containers manager (`about:containers`, 900×600)

```
+-------------------+--------------------------------------------------------------------+
| Containers        | Banking                                              [Delete…]     |
| > ● Personal      | Name   [ Banking          ]   Colour [● v]   Icon [▣ v]            |
|   ● Work          | Route  ( ) Clear  (•) VPN [ home-vpn v ]  ( ) Tor                  |
|   ● Banking       |        Route state: up · exit: (shown by SE03)   [Open Anonymity]  |
|   ● Shopping      | Protection  Blocking [Strict v]  Script [First-party v]            |
|   ● Search        |             Fingerprint [Standard v]  HTTPS-only [on ]             |
|   ◌ Disposable    | Search engine [ (profile default) v ]                              |
|                   | Sites always opened here                                           |
| [+ New container] |   mybank.example            [Remove]                               |
|                   |   [ add site…            ] [Add]                                   |
|                   | Data  14 sites · 6.1 MiB   [New identity]  [Clear data…]           |
+-------------------+--------------------------------------------------------------------+
```

`listView`, `lineEdit`, `colorPicker` (palette of 12), `radioButton`, `comboBox`, `toggleSwitch`, `listView`, `button`.

#### 5.3.5 Credential picker (drawn by the shell, SH04, 420×300)

```
+------------------------------------------------------+
| Passwords — fill on example.org        (shell badge) |
| Requested by: Web Browser · Personal                 |
| > alice@example.org        password · passkey        |
|   alice.work@example.org   password · TOTP           |
| [ search…                ]                           |
| [Suggest a password]         [Cancel] [Fill]         |
+------------------------------------------------------+
```

Not a browser surface. The browser sends origin, frame origin, field kind and container; it receives nothing back except "filled" or "cancelled". The compositor-drawn border marks it as a system prompt.

### 5.4 Menus

```
File      New tab Ctrl+T · New window Ctrl+N · New private window Ctrl+Shift+N ·
          New disposable session Ctrl+Shift+D · New tab in container ▸ · Open file… Ctrl+O ·
          Save page as… Ctrl+S · Save to archive Ctrl+Shift+S · Print… Ctrl+P · Capture page… ·
          Import data… · Export data… · Close tab Ctrl+W · Close window Ctrl+Shift+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut Ctrl+X · Copy Ctrl+C · Paste Ctrl+V ·
          Paste as plain text Ctrl+Shift+V · Select all Ctrl+A · Find… Ctrl+F · Find next Ctrl+G ·
          Find previous Ctrl+Shift+G · Preferences Ctrl+,
View      Sidebar F9 ▸ (Tabs, Bookmarks, History, Archive) · Vertical tabs Ctrl+Alt+V ·
          Bookmarks bar Ctrl+Shift+B · Inspector F4 · Zoom in Ctrl++ · Zoom out Ctrl+- · Reset Ctrl+0 ·
          Reader mode Ctrl+Alt+R · Text encoding ▸ · Page source Ctrl+U · Full screen F11 ·
          Caret browsing F7
History   Back Alt+Left · Forward Alt+Right · Home Alt+Home · Reopen closed tab Ctrl+Shift+T ·
          Recently closed ▸ · Show all history Ctrl+H · Clear data… Ctrl+Shift+Delete ·
          Forget this site…
Bookmarks Bookmark this page Ctrl+D · Bookmark all tabs… · Manage bookmarks Ctrl+Shift+O ·
          (bookmark folders)
Tabs      Search tabs Ctrl+Shift+A · Next Ctrl+Tab · Previous Ctrl+Shift+Tab · Go to tab 1–8 Alt+1…8 ·
          Last tab Alt+9 · Move left Ctrl+Shift+PgUp · Move right Ctrl+Shift+PgDn · Pin · Duplicate ·
          Mute Ctrl+M · Add to group ▸ · Reopen in container ▸ · Unload · Send to device ▸ ·
          Close others · Close to the right
Site      Site panel Ctrl+I · Permissions… · Blocked items… · Certificate… · New identity for container ·
          Translate page · Open in disposable session
Tools     Downloads Ctrl+J · Containers… · Add-ons · Developer tools F12 · Console Ctrl+Shift+J ·
          Network Ctrl+Shift+E · Processes Shift+Esc · Storage report · Policies · Profiles…
Help      Browser help F1 · Keyboard shortcuts · Troubleshoot mode… · About the engine · About
```

### 5.5 Keyboard and pointer

| Keys | Action | Keys | Action |
|---|---|---|---|
| `Ctrl+L`, `Alt+D`, `F6` | focus address bar | `Ctrl+E` | focus address bar in search mode |
| `Ctrl+Enter` | open typed text in a new tab | `Alt+Enter` | duplicate into a new tab |
| `Space`, `Shift+Space` | page down, up | `Ctrl+Shift+R`, `Ctrl+F5` | reload without cache |
| `Esc` | stop, close bar, leave full screen | `Alt+;` | link hints |
| `Ctrl+Shift+K` | switch container of the new tab | `Ctrl+Shift+U` | new identity for the container |

Context menus:

- Page: Back, Forward, Reload, Bookmark, Save page, Save to archive, Print, Translate, Reader mode, View source, Inspect.
- Link: Open in new tab, in new window, in private window, in container ▸, in disposable session; Copy link, Copy clean link; Bookmark link; Download link; Send link to device.
- Image and media: Open in new tab, Save as, Copy image, Copy address, Block this image, Open in Photos / Video / Music.
- Selection: Copy, Copy as Markdown, Search with ▸, Translate, Look up, Send to Notes, Copy link to selection.
- Editable field: Undo, Cut, Copy, Paste, Paste as plain text, Fill from Passwords, Suggest password, Spelling ▸, Input method.
- Tab: New tab to the right, Reload, Mute, Pin, Duplicate, Reopen in container ▸, Add to group ▸, Move to window ▸, Unload, Send to device ▸, Bookmark, Close, Close others, Close to the right, Reopen closed.

Drag and drop: tabs (5.2, F024); a link or text onto the tab strip opens it; a link onto the bookmarks bar bookmarks it; files onto a page reach the page only after the drop lands on a file input or a page drop zone, as read-only handles for exactly those files; text and images dragged out of a page are offered as plain text, URL list and PNG. Dragging out of a private or disposable window to another app asks first.

Gestures: two-finger swipe for back and forward with a 140 ms preview; pinch zoom; `Ctrl+wheel` zoom; middle click opens in a background tab; middle click on a tab closes it.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty (first run) | Welcome page: pick search engine, pick blocking level, import data. Nothing contacts the network. |
| Loading | 2 px progress line; tab shows a spinner; title shows the domain until a title arrives. |
| Error | Chrome-drawn error page with the cause, the route and "Try again". Never a search-engine redirect. |
| Offline | Status chip "Offline"; archived copies of the requested URL are offered. |
| Route down | Per-container banner "Route VPN home-vpn is down. Pages in Work will not load." with "Open Anonymity". |
| Locked | If the profile sits in a locked vault (F151), the window shows only an unlock button. |
| Read-only | Policy-managed settings show a lock and are disabled with the policy name in a tooltip. |
| Crashed tab | Card with Reload and "Report details" (local only, AD04). |
| Engine out of date | Persistent `--warn` bar with "Open Software". |

### 5.7 Widgets

| Region | FUSE widget | Note |
|---|---|---|
| Tab strip | `tabBar` | needs colour stripe, pin, audio icon, tear-off (FUSE-WP-31) |
| Tree tabs, bookmarks, DOM | `treeView` | drag re-parenting (FUSE-WP-28, FUSE-WP-33) |
| Address bar | `lineEdit` + `popupMenu`-style suggestion list | needs segmented colouring of the URL: IN01-GAP-2 |
| Web content | `webSurface` / `remoteSurface` | Proposed in FUSE-WP-51; G-14 |
| Prompt and info bars | `panel`, `label`, `button` | |
| Popovers | `popover` (FUSE-WP-30, Proposed), `comboBox`, `toggleSwitch` | |
| Internal pages | `tableView`, `modelListView`, `propertyGrid`, `scrollArea`, `markdownView` | |
| Reader view, archive viewer (script-free) | restricted rich-content view | FUSE-WP-42, G-10 |
| Developer tools | `dockWidget`, `tabWidget`, `codeEdit`, `timeline`, `chart` | |
| Certificate viewer | `treeView`, `propertyGrid` | |

Gaps: **IN01-GAP-1** engine-side producer for `webSurface` (frames, input, IME, a11y, popups as data); **IN01-GAP-2** URL-aware `lineEdit` (domain emphasis, elision that never hides the domain); **IN01-GAP-3** shell-drawn credential picker and passkey consent with direct keyring-to-process delivery (SH04, SE04); **IN01-GAP-4** per-socket route binding with fail-closed behaviour in the network service (FORGE-WP-21, SE03); **IN01-GAP-5** nested confinement: an app that starts many helper processes each with a different, narrower capability set at run time (FORGE-WP-27).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Browser logic (broker) | ForgeScript | app host | Tabs, containers, navigation policy, permission decisions, session journal, bookmarks, history, settings, policy merge. Decides which process may load which site. |
| Chrome UI | FUSE UI | app host (UI thread) | All windows, bars, internal pages, interstitials, `webSurface` widgets. |
| Engine supervisor | C++23 (safe subset) | helper `engine-supervisor`, one per profile | Starts and reaps engine processes with the exact handles the broker lists; applies `rctl` limits; relays validated IPC records between engine processes and the app host. |
| Content process | imported engine code (C/C++, not safe subset) | one per site per container | Parse, script interpreter, style, layout, paint into a shared-memory surface. Capability mode, no sockets, no files. |
| Network process | C++23 safe subset on FORGE-WP-21's protocol helper | one per container | TLS, HTTP/1.1, HTTP/2, cache, cookie jar, HSTS, blocking, cross-origin read checks, route binding. Sockets come from the network service only. |
| Filter engine | C++23 (safe subset) | inside each network process | Compiled filter lists; URL match under 50 µs. |
| Image and font decode helpers | C++23 (safe subset), shared with `svc.thumbnail` decoders where possible | pool, one job per process lifetime slot | Hostile bytes to pixels or validated tables. |
| Media helper | per G-11 (ME03 section 6.5) | per site | Demux and decode for page media. |
| Reader and archive helper | C++23 (safe subset) | on demand | Article extraction, HTML sanitising, single-file archive read and write, WARC export. |
| Storage service | ForgeScript + `SqliteModel` | app host | Bookmarks, history, sessions, permissions, containers. |
| Sync helper | C++23 (safe subset) | on demand, `net` only to paired devices or the user's WebDAV host | Encrypt, diff, merge records. |
| Developer tools | ForgeScript + FUSE UI | app host | Client of the engine's inspection protocol over the supervisor relay. |

```
 shell (SH04 prompts, credential picker) ── svc.keyring (SE04) ─────────────┐ secret, direct
        │ prompts                                                          ▼
 app host: broker (ForgeScript) + chrome (FUSE UI) ── engine-supervisor ── content process (site A, container P)
        │   bookmarks/history/session (SQLite)             │           ├── content process (site B, container P)
        │                                                  │           ├── content process (site A, container W)
        │                                                  ├── network process (container P) ── net service ── route Clear
        │                                                  ├── network process (container W) ── net service ── route VPN (SE03)
        │                                                  ├── decode helpers (image, font), media helper (G-11)
        └── svc.intent ─ IN03 Transfers, SE05 Vault, OF03  └── reader/archive helper
```

Rules: content processes talk only to the supervisor. A content process for site S in container C may ask the network process of C only for URLs the broker's navigation policy allows for S. The broker treats every record from an engine process as hostile and range-checks it (T2 target).

### 6.2 Data model

Stores under `~/.local/share/org.pbsd.browser/profiles/<profile>/`. SQLite with `schema_version` in every file; forward-only migration after a copy.

| Entity (store) | Fields |
|---|---|
| `container` (`profile.db`) | `id u32`, `name text`, `colour u8`, `icon text`, `route enum{clear,vpn,tor}`, `route_profile text`, `block_level u8`, `script_mode u8`, `fp_level u8`, `https_only bool`, `search_engine_id u32?`, `temporary bool`, `created i64` |
| `site_rule` (`profile.db`) | `site text` (scheme + registrable domain), `container_id u32` (0 = all), `always_open_in u32?`, `block_level u8?`, `script_mode u8?`, `fp_level u8?`, `zoom_pct u16`, `never_remember bool`, `user_css_id u32?` |
| `permission` (`profile.db`) | `site text`, `container_id u32`, `kind enum{camera,mic,screen,location_coarse,location_precise,notify,clipboard_read,popups,redirects,autoplay,downloads,local_net,storage_quota,wasm}`, `state enum{ask,allow,block}`, `scope enum{once,session,always}`, `granted i64`, `last_used i64`, `expires i64?` |
| `bookmark` (`bookmarks.db`) | `guid blob16`, `parent_guid blob16`, `position u32`, `kind enum{url,folder,separator}`, `title text`, `url text`, `tags text[]`, `keyword text?`, `open_in_container u32?`, `read_later bool`, `archive_id u64?`, `added i64`, `modified i64`, `deleted bool` |
| `visit` (`history-<container>.db`) | `id u64`, `url_id u64`, `time i64`, `transition enum{typed,link,bookmark,redirect,reload,restore}`, `from_visit u64?`; `url(id, url, title, frecency i32, last_visit i64)` |
| `session` (`session.journal`, append-only, compacted) | `window{id, kind, bounds, sidebar}`, `tab{id, window_id, parent_tab_id?, group_id?, container_id, pinned, muted, unloaded, entries[{url, title, scroll_x, scroll_y, state_blob}], current_index}`, `group{id, name, colour, collapsed}`, `closed[]` |
| `archive` (`archive/index.db` + `archive/<id>.pagearc`) | `id u64`, `url text`, `title text`, `saved i64`, `container_id u32`, `sha256 blob32`, `size u64`, `tags text[]`, `has_screenshot bool`, `text_index_id u64?` |
| `search_engine` (`profile.db`) | `id u32`, `name text`, `url_template text`, `suggest_template text?`, `keyword text`, `post bool`, `source enum{bundled,user,addon}` |
| `cert_exception`, `pin` (`profile.db`) | `host text`, `container_id u32`, `spki_sha256 blob32`, `kind enum{exception,pin}`, `session_only bool`, `added i64` |
| Per-container web storage (`containers/<id>/`) | Cookie jar, cache, HSTS, site storage. Owned and opened only by that container's network process. Private and disposable: tmpfs. |
| `download_ref` (`profile.db`) | `id u64`, `transfer_id text` (IN03), `url text`, `site text`, `container_id u32`, `path_handle_id text`, `sha256 blob32?`, `quarantine enum{held,released,deleted}`, `started i64` |

`.pagearc` (Proposed): a ZIP-compatible container with `manifest.json` (URL, time, hashes, TLS fingerprint), `index.html` (sanitised, script removed), `res/*`, `screenshot.png`, and a detached Ed25519 signature made with the device key, so later tampering shows.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| HTML, CSS, SVG, MathML, JS, DOM APIs | yes | — | imported engine (6.5) | content process |
| HTTP/1.1, HTTP/2, WebSocket, TLS 1.3 | yes | yes | FORGE-WP-21 protocol helper (Proposed; TLS library not chosen) | network process |
| HTTP/3 (QUIC) | later | later | not planned before phase 3; UDP needs an explicit capability | network process |
| PNG, JPEG, WebP, GIF, AVIF, ICO | yes | PNG | platform image decoders (shared with ME01; to be specified there) | decode helper |
| WOFF2, OpenType | yes | — | sanitised in a helper, then FUSE text (FUSE-WP-17) or the engine's shaper | font helper |
| Audio and video in pages | yes | — | media pipeline G-11 | media helper |
| Adblock Plus filter syntax | yes | yes (user rules) | own code | network process (compiled form), app host (editor) |
| Netscape bookmarks HTML, bookmarks JSON | yes | yes | own code | reader/archive helper |
| `.pagearc`, WARC, MHTML (import only) | yes | `.pagearc`, WARC | own code | reader/archive helper |
| PDF (print output) | — | yes | engine paginator to the print pipeline (G-16) | content process, then `svc.portal.print` |
| X.509, OCSP staples, SCTs | yes | — | TLS library of FORGE-WP-21 | network process |
| Policy JSON, add-on bundles | yes | — | own code; signature check by the app host | app host |
| Engine inspection protocol | yes | yes | engine's own protocol (to verify per engine) | app host validates records |
| HAR | — | yes | own code | app host |

### 6.4 Integration

- Intents handled: `open`, `view` for `http`, `https`, `file` (HTML), `about`; `search`; hint `disposable`, hint `container=<name>`. Emitted: `open` (downloads to OF03, ME03 and others), `share`, `print`, `compose` (`mailto:`), scheme hand-offs, "subscribe" to IN02, "transfer" to IN03.
- MIME: view `text/html`, `application/xhtml+xml`, `image/svg+xml`, `application/x-pbsd-pagearc`; export `application/pdf`, `image/png`, `application/warc`.
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `.print`, `.camera`, `.mic`, `.screen`, `.location`, `svc.intent`, `svc.search`, `svc.keyring` (indirect, through the shell picker), `svc.clipboard`, `svc.media`, `svc.power`, `svc.model`, `svc.update`, `svc.backup`, `svc.audit`, `svc.a11y`.
- Other apps: IN03 (downloads), SE05 (quarantine, verify), SE04 (credentials, passkeys, trust store), SE03 (routes, new circuit), SE02 (DNS policy, per-app rules), IN02 (feeds), OF03 (PDF), AD05 (grants mirror), AD04 (crash reports), SY05 (engine and add-on packages), CO01 and UT05 (share the restricted HTML path, not the engine, in phase 1).
- Backing repos. Verified: FUSE has no web engine and states that as a non-goal (cited in FUSE-WP-51 as `FUSE_UI_PLAN.md:53`; read through the build plan, not re-read here). ANONYMOUS is AEGIS, a metadata-hiding transport for a permissioned consortium (`ANONYMOUS:README.md`); it is not a general web exit network. Proposed: everything else in this section.

### 6.5 Engine analysis for G-14

Statements about outside projects below are from general knowledge, not from source read for this spec. Each "to verify" must be checked against the pinned upstream release before a decision is final.

**Option (a): import an engine.**

| | Gecko + SpiderMonkey | WebKit + JavaScriptCore | Chromium (Blink + V8) | Servo | Ladybird (LibWeb + LibJS) |
|---|---|---|---|---|---|
| Licence | MPL-2.0 | LGPL-2.1 (WebCore) and BSD-2-Clause | BSD-3-Clause and many bundled licences | MPL-2.0 | BSD-2-Clause |
| Language, memory safety | C++ with a growing share of Rust (style system, parts of media and networking). Not memory safe overall. | C++. Hardened allocator and pointer caging in JSC (to verify what remains useful without JIT). Not memory safe. | C++ with some Rust; partitioned allocator, checked pointers. Not memory safe. | Rust for layout, style, DOM glue; script is SpiderMonkey (C++). Safe in part. | C++; a move towards a memory-safe language has been announced (to verify which and how far). Not memory safe today. |
| Without JIT | Interpreter-only builds and run-time switches exist (to verify: exact configure flag, and whether the faster "baseline interpreter" needs generated code). Expect large slowdowns on script-heavy sites. | A low-level interpreter is pre-built at compile time; a portable C++ interpreter back end also exists. JIT-off is a shipped mode on another vendor's lockdown profile (to verify), so it is exercised in the field. | `--jitless` runs the bytecode interpreter and an interpreted regular-expression engine (to verify for the pinned version). It is a secondary configuration upstream. | Same as SpiderMonkey. | LibJS is a bytecode interpreter; an earlier JIT experiment was removed (to verify). No JIT is the normal mode. |
| WebAssembly without JIT | To verify; believed to need a compiler tier, so WebAssembly would be off. | An interpreter tier for WebAssembly exists (to verify that it runs with all JIT off). | A WebAssembly interpreter was added for jitless mode (to verify state and speed). | As SpiderMonkey. | LibWasm is an interpreter (to verify coverage). |
| FreeBSD / HardenedBSD | In FreeBSD ports. On HardenedBSD it has needed MPROTECT exemptions because of the JIT (to verify that a JIT-less build needs none). | WebKitGTK is in ports; the toolkit-less WPE port's state on FreeBSD is to verify. | In ports with a large local patch set; no upstream BSD support; heavy upkeep on every release. | To verify. | To verify. |
| Sandbox today, and mapping | Process sandbox on FreeBSD is absent or partial (to verify). Content, socket and decoder process split exists, which maps well. | Non-Apple ports sandbox with Linux namespaces and seccomp. No Capsicum code (to verify). Web, network and GPU process split maps onto section 6.1. | The strongest process model and site isolation. The FreeBSD port's sandbox state is to verify. The broker pattern (pre-opened descriptors, no ambient rights) maps directly onto Capsicum. | Multi-process and sandbox support incomplete (to verify). | A content process per tab plus request and image-decoder processes; designed around pledge/unveil on its original OS; no Capsicum (to verify). |
| Site isolation (cross-site frames out of process) | Yes on desktop (to verify on BSD). | In progress upstream (to verify). | Yes, mature. | No (to verify). | No (to verify). |
| Chrome decoupling | Hard. The chrome is itself Gecko content and the widget layer on Unix is GTK. No supported embedding API on desktop. | Good. The API is a view with a UI-process side; WPE draws into buffers with no toolkit. It depends on GLib and, for networking, libsoup (a curl back end exists in another port; to verify). GLib is not GTK, but it is a large C dependency. | Possible. The content layer and its platform abstraction allow a headless or custom platform; the embedding layer is unstable and huge. | An embedding API is in development (to verify). | UI is separate from the engine library, with Qt, AppKit and headless front ends (to verify); a FUSE front end is a small job by comparison. |
| Web compatibility | High. | High. | Highest. | Low to medium. | Low to medium, rising. |
| Build size and upkeep | Very large; Rust toolchain needed. | Large; C++ only plus Python, Ruby and Perl at build time (to verify). | Largest; own toolchain expectations; vendor services to strip. | Large; Rust; network at build unless vendored. | Medium; uses third-party libraries (graphics, shaping, HTTP; to verify list). |

How any imported engine maps onto Capsicum and PBSD capabilities: the supervisor opens what a process needs (IPC socket, shared memory, fonts, ICU data, the container's storage directory for the network process), passes them, and the child calls `cap_enter()` before reading hostile bytes. Engines assume they can open files, sysctls, `/dev/urandom`, timezone and locale data late; each such site must be patched to use a pre-opened descriptor or a broker request. That list is found by running under capability mode and fixing each `ECAPMODE`. PBSD handles add lineage: a camera handle given to site A's process is a child of the browser's grant and is revoked alone.

An imported engine is not in the safe C++23 subset and cannot reach T1 or T2. It needs an explicit exception to PLATFORM section 2, justified only by confinement: one site per process, no sockets, no files, no JIT, HardenedBSD mitigations on (ASLR, PaX MPROTECT, CFI where it builds). This exception is for the owner to accept (section 13).

**Option (b): write an engine.** A full web engine is the work of hundreds of engineers over many years; the standards are still growing. The suite cannot fund that. A small own engine is realistic for a fixed subset: HTML parsing to the standard's tree-building rules, a CSS subset, block and inline layout, tables, images, no script. That is useful for mail, help, feeds, reader mode and archives, can be in the safe subset with T2 targets, and has no JIT question at all.

**Recommendation: two tracks.**

- **Track A (own, script-free).** The restricted rich-content path of FUSE-WP-42 grows into a script-free document renderer in a sandboxed helper. It serves CO01, UT05, IN02, reader mode and the archive viewer. It closes G-14 for those apps without any imported engine.
- **Track B (imported, general web).** First choice: WebKit with JavaScriptCore built interpreter-only, as a new toolkit-less port modelled on WPE, drawing into `webSurface`. Reasons: JIT-off is an exercised configuration; the port layer is designed for embedders without a toolkit; the process split matches 6.1; the licence allows it; the code base is smaller than Chromium's. Costs: Capsicum sandbox must be written; cross-site frame isolation may be missing, so F032's fallback (block or click-to-load third-party frames) is the default; GLib and the HTTP stack must be replaced or confined (network goes through our network process either way).
- **Fallback for Track B:** Chromium content layer with V8 jitless, if the WebKit spike fails on compatibility or sandboxing. It gives the best isolation and the worst upkeep.
- **Watch:** Ladybird and Servo. Either could replace Track B later with a smaller, cleaner, more permissive base. Re-evaluate at each phase gate with the same test set.
- **Not recommended:** Gecko, because the chrome cannot be separated from GTK and XUL-era front-end code without forking most of the application. It stays the reference for fingerprinting resistance and Tor-route behaviour.

Phases: **E0 spike (before any browser work)**: build JSC interpreter-only and the toolkit-less WebKit port on HardenedBSD with MPROTECT enforced; run under `cap_enter()`; measure the top 100 sites for function and speed; same for Chromium jitless if time allows; record results. **E1**: `webSurface` producer, input, IME, a11y; network through our network process; one process per site. **E2**: decoders out of process, media through G-11, print, inspection protocol. **E3**: frame isolation or the best available substitute; WebAssembly interpreter decision.

Unknown until E0 runs: whether each engine builds and runs with no executable-memory request at all (some allocate executable pools at start even with JIT off; to verify); real-site speed without JIT; the number of capability-mode violations to patch; whether upstream will take BSD and Capsicum patches; staffing needed to track security releases within days.

## 7. Security and privacy

### 7.1 Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | Profiles, containers, session journal, archive. |
| `fs.portal` | Downloads folder, uploads, open and save page. |
| `fs.path:/etc/pbsd/browser/policies:r` | Policy files (F177). |
| `net.user` | The user types the endpoints. Each first contact is logged to `svc.audit` per container (domain only; not for private or disposable windows). |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-container routes; each network process is narrowed to one. |
| `proc.helper:engine-supervisor`, `:content`, `:network`, `:decode-image`, `:decode-font`, `:media`, `:reader-archive`, `:sync` | The processes of 6.1. Each gets a strict subset. |
| `dev.camera`, `dev.mic`, `dev.screen`, `dev.location`, `dev.print` | Site permissions (4.7); prompt on first use; second gate per site. |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.media`, `svc.power`, `svc.update`, `svc.backup`, `svc.audit`, `svc.a11y`, `svc.portal.*` | As listed in 6.4. `svc.keyring` holds only sync keys and the device signing key for the browser itself; site credentials never pass through the app. |
| `svc.model`, `ai.local` | Translation and summaries (F124, F125). Off until used. |
| `bg.run` | Only while a download owned by the browser stream or page audio continues after the last window closes; off by default. |

Not requested: `proc.exec`, `dev.usb:*`, `dev.gpu.compute`, `bg.autostart`.

### 7.2 Threat model

| | |
|---|---|
| Assets | Credentials and sessions; browsing history; identity separation between containers; the user's files and devices; the rest of the system. |
| Adversaries | Malicious pages and adverts; compromised sites; network attackers on the route; trackers and fingerprinters; a hostile download; a malicious add-on author; another local app; someone with brief access to the unlocked session. |
| Attack surface | The engine (parsers, script interpreter, layout, media); network stack; IPC from engine processes to the broker; filter lists and add-on bundles; policy files; sync records; UI spoofing (prompts, address bar, full screen). |

### 7.3 Mitigations

- Engine memory-safety bugs are assumed. A compromised content process holds no sockets, files or devices, sees one site in one container, and can only send records the broker validates. No JIT removes the easiest path from a bug to code execution; MPROTECT blocks injected code.
- Cross-container leaks: separate network processes, storage directories and routes; the broker refuses a request from process (S, C) for storage of any other pair.
- Network attacker: HTTPS-only, HSTS preload, pins, no speculative requests, DNS on the route, fail closed.
- UI spoofing: prompts, interstitials and pickers are drawn by the chrome or the shell; content is clipped to its surface; full screen shows a shell banner; the domain is never elided.
- Downloads: provenance, quarantine, disposable-sandbox open through SE05.
- Sync: records encrypted and authenticated on the device; the transport learns sizes and times only.
- Residual risks: a content-process bug still exposes that site's data in that container; engine patch lag; fingerprinting cannot be made perfect on a rare OS; without out-of-process frames a third-party frame shares the embedding site's process if the user allows it.

### 7.4 Why no code extensions

Extensions run with broad access to every page, are the largest fingerprinting vector after the engine, are a common supply-chain attack path, and need a second script world, background pages and a privileged API that would have to be designed, sandboxed and proved. The main needs (blocking, containers, password fill) are built in and stronger. Declarative, signed, data-only add-ons cover the rest. Cost: users lose niche extensions; this is stated in the first-run page.

### 7.5 Stored and sent

Stored: the entities in 6.2, in the profile; nothing in private or disposable modes survives. Sent: only requests the user's navigation causes, on the container's route. No telemetry, no crash upload, no safe-browsing lookups, no update pings (updates come through `svc.update`), no captive-portal probe by the browser (the shell owns that).

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `startup.restore` | enum always/ask/never | always | Session restore (F170). |
| `tabs.layout` | enum horizontal/vertical/tree | horizontal | Tab strip form. |
| `tabs.unload_after_min` | int 0–1440 | 30 | 0 disables F022. |
| `engine.max_content_processes` | int 8–256 | 64 | Soft limit (F036). |
| `engine.content_memory_mib` | int 256–8192 | 2048 | Per-process cap (F039). |
| `engine.wasm` | enum off/ask/on | off | F031; only shown if an interpreter exists. |
| `privacy.block_level` | enum off/standard/strict | standard | Default for new containers. |
| `privacy.fp_level` | enum off/standard/strict | standard | F064. |
| `privacy.script_mode` | enum all/first_party/none | all | F063. |
| `privacy.strip_tracking_params` | bool | true | F051. |
| `privacy.site_data_expiry_days` | int 0–365 | 30 | F073. |
| `security.https_only` | bool | true | F074. |
| `security.local_network_guard` | bool | true | F087. |
| `history.keep_days` | int 0–3650 | 90 | 0 keeps none. |
| `search.default_engine` | id | unset | Chosen at first run. |
| `search.suggestions` | bool | false | F004. |
| `downloads.handoff` | enum transfers/builtin | transfers | F103. |
| `permissions.expire_unused_days` | int 0–365 | 90 | F101. |
| `reader.font`, `reader.size_pt`, `reader.width_ch`, `reader.theme` | string, int, int, enum | system, 15, 70, auto | F123. |
| `sync.enabled`, `sync.kinds`, `sync.transport` | bool, set, enum lan/webdav | false, {bookmarks,tabs,settings}, lan | F171–F172. |
| `a11y.min_font_px`, `a11y.caret_browsing`, `a11y.stop_animations` | int, bool, bool | 0, false, false | 4.18. |
| `devtools.enabled` | bool | true | F164. |

## 9. Accessibility and languages

- The page a11y tree is grafted and validated (F180); AX01 gets headings, landmarks, links, form controls and live regions with the same commands as in other apps.
- Chrome-drawn bars and interstitials are announced as alerts; permission prompts never steal focus but are reachable with `F6` cycling (address bar, bars, content, sidebar).
- Page zoom, text zoom, minimum font size, forced colours, reduced motion and caret browsing are per-site where it makes sense.
- Chrome strings come from tables. Page text needs the FULL text profile (G-03) for complex scripts; right-to-left pages are laid out by the engine, chrome mirrors by locale.
- `Accept-Language` is the container's list; in Strict fingerprint level it is fixed to `en-US, en` unless the user changes it.

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first chrome frame | 400 ms; restored tabs load lazily |
| New tab (chrome ready) | 50 ms; content process spawn 150 ms |
| Input to chrome frame | 8 ms; chrome never blocks on an engine process |
| Memory | broker + chrome 150 MiB; empty content process 60 MiB (to measure in E0); network process 40 MiB |
| Filter match | 50 µs per request with 150 000 rules |
| Session journal write | 2 ms, off the UI thread |
| Open tabs | 2 000 tabs in a session (most unloaded) with tab search under 30 ms |
| Script speed | No fixed budget until E0 measures interpreter-only speed; target is "top 100 sites usable" |

## 11. Testing and proof

Nothing has run. Targets only.

- Unit: URL classifier, IDN display rules, frecency, navigation policy (which process may load which site), permission state machine, session journal replay, filter syntax parser, bookmark merge.
- Golden renders: every chrome view of section 5 in light, dark and high contrast.
- Scripted UI through the a11y tree: tasks T1–T15.
- Engine conformance: the public web-platform test suite, run interpreter-only, with a tracked expected-failure list; JavaScript conformance suite likewise.
- Fuzz: IPC records from engine processes, filter lists, bookmark HTML, `.pagearc`, policy JSON, sync records, HTTP parsers (shared with FORGE-WP-21).
- Confinement tests: a test content process tries `open`, `socket`, `connect`, `mmap(PROT_WRITE|PROT_EXEC)`, `mprotect(+x)`, `fork`, `sysctl`; every call must fail. A cross-container test asks for another container's cookie; the broker must refuse.
- Leak tests: packet capture on every interface while browsing in a VPN and a Tor container with the route cut; zero packets outside the route. DNS included.
- Differential: blocking decisions against a reference filter-list implementation on a recorded request corpus.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Navigation policy (broker) | A process for (site, container) never receives data of another pair | T1 | Model in PRISM over the policy function; property tests | ForgeScript verification support in PRISM (to verify) |
| IPC record validator | Every field range-checked; no out-of-bounds read | T2 | PRISM absence-of-runtime-error job; fuzz | — |
| Engine supervisor | Children start with exactly the listed descriptors | T2 | PRISM plus descriptor-table inspection test | — |
| Filter engine | No runtime errors; match is total and terminates | T2 | PRISM; differential test | — |
| Network process (own code) | Parser safety; socket only on the bound route | T2 (parsers), T1 (route rule, inherited from FORGE-WP-21) | As FORGE-WP-21 | TLS library is third-party: T4 for that part |
| Reader/archive helper | Sanitiser output contains no script or remote reference | T2, with T1 aim for the allow-list property | PRISM; fuzz with a script-detection oracle | HTML parsing complexity |
| Imported engine | None provable | T4 | Sanitiser builds, fuzzing, upstream tests, confinement tests | Not in the safe subset; size |
| Track A renderer | Parser and layout free of runtime errors | T2 | PRISM; fuzz | — |
| ForgeScript logic | Memory safety by construction | T3 | Interpreter guarantees (V10, V11); unit tests | — |

## 12. Build plan

| ID | Work item | Depends on |
|---|---|---|
| | **Phase 0: engine decision** | |
| IN01-W01 | E0 spike: build WebKit interpreter-only, toolkit-less, on HardenedBSD with MPROTECT; record results | G-18, FORGE-WP-02 |
| IN01-W02 | E0 comparison run for Chromium jitless (time-boxed) and a desk check of Ladybird and Servo | W01 |
| IN01-W03 | Decision record and the owner's exception for non-safe-subset engine code | W01, W02 |
| | **Phase 1: usable core** | |
| IN01-W04 | Engine supervisor and Capsicum start-up for content processes | W03, G-06, G-07, FORGE-WP-27 |
| IN01-W05 | `webSurface` producer in the engine port: frames, input, IME, a11y, popups as data | W03, FUSE-WP-51, G-01 |
| IN01-W06 | Network process on the platform protocol helper; cookie jar, cache, HSTS, HTTPS-only | FORGE-WP-21, G-08 |
| IN01-W07 | Broker: navigation policy, one process per site, containers (storage only) | W04, W06, G-02 |
| IN01-W08 | Chrome: tabs, address bar, site badge and panel, interstitials, find, zoom | G-02, FUSE-WP-29, FUSE-WP-30, FUSE-WP-31 |
| IN01-W09 | Filter engine, bundled lists, blocking levels, script control | W06 |
| IN01-W10 | Bookmarks, history, clear data, forget site, search engines, first run | W08, G-09 |
| IN01-W11 | Session journal and restore; crash and hang handling | W07 |
| IN01-W12 | Private windows; downloads with provenance; builtin stream | W06, `svc.portal.file` |
| IN01-W13 | Permission model and prompt bar (files, pop-ups, autoplay) | W07 |
| IN01-W14 | Engine package and update channel through `svc.update` | SY05 |
| | **Phase 2: full feature** | |
| IN01-W15 | Routes per container, fail closed, leak tests | SE03, IN01-GAP-4 |
| IN01-W16 | Vertical, tree and grouped tabs; unloading; tab search across windows | W08, FUSE-WP-28, FUSE-WP-33 |
| IN01-W17 | Device permissions: camera, mic, location, notifications, clipboard | W13, SH04, G-09 |
| IN01-W18 | Trusted-path credential fill and passkeys | SE04, SH04, IN01-GAP-3 |
| IN01-W19 | Hand-off to IN03; quarantine through SE05 | IN03, SE05 |
| IN01-W20 | Decode helpers out of process; media through G-11 | G-11 |
| IN01-W21 | Reader mode, archive (`.pagearc`), archive library | FUSE-WP-42, Track A helper |
| IN01-W22 | Fingerprint levels; partitioning audit | W07 |
| IN01-W23 | Print and PDF; capture | G-16 |
| IN01-W24 | Disposable sessions; intent hint | W04, FORGE-WP-07 |
| IN01-W25 | Developer tools: elements, console, network, security | W05 |
| IN01-W26 | Certificate viewer, pins, policy files, declarative add-ons, profiles | W10, SE04 |
| | **Phase 3: advanced** | |
| IN01-W27 | Tor containers and onion sites | W15, SE03 (Tor availability to verify) |
| IN01-W28 | Local translation and summaries | G-15, AI02, FORGE-WP-31 |
| IN01-W29 | Own-device sync and send tab | pairing facility (IN03), SE04 |
| IN01-W30 | Kiosk mode, full policy coverage, guest profile | W26, SH03 |
| IN01-W31 | Frame isolation or substitute; WebAssembly interpreter decision | W03 results |
| IN01-W32 | Remaining developer tools; split view; picture-in-picture | W25, `svc.media` |
| IN01-W33 | Re-evaluation of the engine choice against Ladybird and Servo | W03 |

## 13. Open questions and risks

1. The engine choice is unproven until IN01-W01 runs. Every "to verify" in 6.5 is open.
2. An imported engine breaks the "memory-safe C++23 only" rule. The owner must accept the exception or accept that there is no general web browser.
3. Filter-list redistribution licences must be checked at packaging; Strict fingerprint values should copy the largest comparable population (to decide). Interpreter-only script may make some heavy web apps unusable. No mitigation exists inside the rules; the answer is honest messaging and, for trusted apps, VM03 or a VM (VM01).
4. Engine security upkeep needs standing staff: upstream ships fixes every few weeks.
5. SE03's backing repo is a consortium mixnet (`ANONYMOUS:README.md`), not Tor. Whether SE03 will also drive Tor is not established here; F055, F056 and W27 depend on it.
6. Out-of-process frames may be missing in the chosen engine; the fallback (blocking third-party frames by default) will break embedded video and payment frames until click-to-load is used.
7. The TLS library for FORGE-WP-21 is not chosen; post-quantum key exchange, ECH and CT depend on it.
8. No DRM (F099) means some streaming sites will not play. This is a decision, not a gap.
9. GLib inside a WebKit-derived port conflicts with the spirit of "no GTK stack"; the port should aim to remove or confine it.
