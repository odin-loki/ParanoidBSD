<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Gap register

By Odin Loch

Work the app specs need that does not exist yet and is not one of the 18 platform gaps in [PLATFORM.md](PLATFORM.md) section 13. Each row was raised by a spec writer after reading the repos; the "Raised by" column gives the spec gap IDs so the detail can be found in that spec's sections 5.7, 6 and 13. Duplicates raised by several specs are merged into one row.

Owner tells you where the work lands. FUSE and FORGE rows are additions to [build-plans/FUSE_BUILD_PLAN.md](build-plans/FUSE_BUILD_PLAN.md) and [build-plans/FORGE_BUILD_PLAN.md](build-plans/FORGE_BUILD_PLAN.md); those plans already map the spec gaps they saw (FUSE plan section 7.3), so a row here may already have a work package there.

Status of every row: **open, nothing built**.

## 1. Compositor (`pbsd/compositor`)

The compositor's own README calls it a scaffold, and the protocol modules read are short opcode tables. The shell cannot be built until these exist.

| ID | Gap | Raised by |
|---|---|---|
| X-C01 | Toplevel management (activate, close, minimise, move to workspace); today only listing exists | SH01-GAP-1 |
| X-C02 | Window thumbnails and a capture source handed to shell clients as buffers; no capture module exists | SH01-GAP-2, SH04-GAP-3, SH05-GAP-2 |
| X-C03 | Session lock that fails closed | SH03-GAP-1 |
| X-C04 | Trusted layer above every app surface: compositor-drawn trust frame, secure attention key, excluded from capture, refuses synthetic input | SH03-GAP-2, AD05-GAP-3 |
| X-C05 | Idle notify, gesture recognition, global shortcut table (tap and hold), gamma, output management, refresh-rate switching | SH03-GAP-4, SH05-GAP-5, SH05-GAP-8, SH02-GAP-3, ME03-GAP-2 |
| X-C06 | Window policy in the compositor with rules as data (D-61), a frame painter with text, verified client identity | SH05-GAP-1, SH05-GAP-3, SH05-GAP-4 |
| X-C07 | Cross-client window tokens for tethering system windows and embedding applets | SH04-GAP-2, SH01-GAP-4, UT02-GAP-2 |
| X-C08 | Compositor's own in-use indicator when no panel is visible; pixel sampling for the colour loupe | SH02-GAP-6, SH04-GAP-4 |
| X-C09 | Zoom, view transform and colour-filter stage | AX02-GAP-1, AX02-GAP-2 |
| X-C10 | Virtual keyboard and pointer with hardware-provenance serials; a voice class of synthetic input; delivery of assigned switch inputs only | AX03-GAP-1, AX06-GAP-4, AX03-GAP-4 |
| X-C11 | Input-method interface (the text-input table present lists commit and pre-edit as requests where they should be events) | AX04-GAP-2 |
| X-C12 | Clipboard data-control bound to one service; a paste-gesture serial required to read (D-62) | AX05-GAP-1 |
| X-C13 | Reader key routing; a `secret` surface flag so password fields are redacted in capture and hidden from assistive reads | AX01-GAP-4, ME05-GAP-4 |
| X-C14 | Window types: picture-in-picture, always-on-top click-through pin, small keep-on-top utility | ME03-GAP-5, ME05-GAP-5, OF07-GAP-3 |
| X-C15 | A flag to exclude a window from screen capture | AI01 |
| X-C16 | Shortcut inhibit and pointer lock exposed to apps | VM01, VM04 |
| X-C17 | Headless sessions with virtual outputs; blanking of sensitive surfaces in a shared session; shell-owned sharing indicator and stop chord | VM04 |
| X-C18 | Per-output full-screen windows with stable output identity and hot-plug | OF06-GAP-2 |
| X-C19 | HDR output path | ME03-GAP-1 |
| X-C20 | Gamepad input delivered to sandboxed apps | GM01-GAP-3 |
| X-C21 | Keyboard layout data and a key-map compiler (libxkbcommon in a helper, D-87) | AX04-GAP-3 |

## 2. Kernel and base system

| ID | Gap | Raised by |
|---|---|---|
| X-K01 | Kernel revoke primitive. The lineage spec maps revoke to `cap_revoke`; none was found in the kernel sources. Total revocation (SI-6) for descriptors held by another process is new work | AD05-GAP-4 |
| X-K02 | The lineage tree cannot be listed: no child listing, no holder or resource metadata, fixed 4,096 nodes. A lineage index and an introspection interface are needed | AD05-GAP-4, SY04 |
| X-K03 | The Capsicum bridge maps 4 of the 8 PBSD rights | AD05 |
| X-K04 | A per-process capability-domain id, and a way to read another process's handle rights | SY04 |
| X-K05 | Socket-to-app label visible to pf as a tag; per-app routing table set at launch (D-23) | SE02-GAP-P1, IN01-GAP-4 |
| X-K06 | Hold a newly attached USB, Bluetooth or Thunderbolt device unbound until policy answers | VM05 |
| X-K07 | List per-process W^X and PaX exemptions; a user-space measured-boot event log | SE01 |
| X-K08 | TPM 2.0 userland (only the kernel driver is in base) | AD01-GAP-4 |
| X-K09 | Loader signing and a Secure Boot chain; boot-once with fallback in the loader | AD01-GAP-5, SY05 |
| X-K10 | A one-time hand-off token from disk unlock to the greeter; FUSE on a bare framebuffer for the unlock screen; speech and on-screen keyboard at disk unlock | SH03-GAP-6, SH03-GAP-7, AX01-GAP-5 |
| X-K11 | Real-time scheduling and locked memory grant for a named helper | CR03-GAP-2 |
| X-K12 | The package manager in this repo is a scaffold: no I/O, crypto, ZFS or on-disk format. The package format, signed repository and release keys are all to be built | SY05 |
| X-K13 | The hypervisor in this repo is a scaffold (59 modules, one placeholder VM); the driver host, device model and personalities do not exist yet. Virtual Machines, Jails, Compatibility and Devices wait on the OS plan's Phases 3 to 5 | VM01–VM05 |
| X-K14 | Printing and scanning: base has only `lpr`/`lpd`. The whole pipeline is new | UT06 |
| X-K15 | Rust toolchain and crates vendored for offline builds (D-21) | SE03, CO05 |
| X-K16 | NTFS and exFAT support was not found in the tree | AD02 |
| X-K17 | Document conflict: the driver-descriptor spec and code disagree on `vendor_id` width and on the number of device classes | VM05 |

## 3. Platform: capabilities to add to PLATFORM.md section 6

Adopted (DECISIONS.md D-30); listed in PLATFORM.md section 14.

| ID | Capability | For | Raised by |
|---|---|---|---|
| X-P01 | `net.listen:<addr>:<port>` and a Unix-socket form | Serving models, remote control of slides | AI02, OF06-GAP-3 |
| X-P02 | `net.local` | LAN discovery (mDNS) and one-shot listeners: casting, peer sync, device send, printers, clipboard sync | ME03-GAP-4, OF02-GAP-1, IN03-GAP-3, UT06-GAP-1, AX05-GAP-5 |
| X-P03 | `net.p2p:<route>` | Unnamed peers, inbound port, UDP, bound to one route (BitTorrent, calls) | IN03-GAP-2, CO05 |
| X-P04 | `net.device:<type>` | User-added endpoints of one device type (printers, scanners) | UT06-GAP-3 |
| X-P05 | Run-time `net.host` grants | Hosts the user adds while the app runs (database servers, data connections) | OF09-GAP-2 |
| X-P06 | `net.route` value `mix`; UDP named in the network capabilities | AEGIS route | SE03, CO05 |
| X-P07 | `dev.bluetooth:<profile>`, `dev.audio-capture`, `dev.midi`, `dev.optical`, `dev.input-inject`, `dev.gamepad`, `dev.haptics`, `dev.sensors` | Device portals the specs need | VM05, ME06, CR03-GAP-3, ME03-GAP-3, VM04, GM01, AX03-GAP-3, ME04-GAP-3 |
| X-P08 | `shell.*` (layer, toplevels, workspaces, launch, lock, trusted-path, session, shortcuts, device-policy, system-window); system-signed bundles only | The shell apps | SH01-GAP-6 |
| X-P09 | `svc.a11y:<right>` (tree.read, text.read, events, act, act.paste, keys.reader, overlay, secure.read, track, view.transform, input.synth, switch, textinput.broker); system-signed bundles only | Assistive tools | AX01–AX06 |
| X-P10 | An administrator-approval class: actions no per-app capability covers (system settings, bridging, passthrough, policy and trust edits) go through one signed action registry and a trusted prompt | Every admin app | AD01-GAP-6, SH03-GAP-3, AI02, VM01, VM05 |
| X-P11 | `net.capture` | Packet capture through a system capture service, scoped per app, with a prompt and an indicator | DV05 |

## 4. Platform: services to add to PLATFORM.md section 8

Adopted (D-30).

| ID | Service | Purpose | Raised by |
|---|---|---|---|
| X-S01 | `svc.auth` | Signed action registry and administrator approval | SH03-GAP-3 |
| X-S02 | `svc.session` | Seats, sessions, inhibitors, save queries, launch and supervision of apps | SH03-GAP-5, SH01-GAP-5 |
| X-S03 | `svc.config` | The only writer of system state (sysctls, `/etc`, service actions, signals to other users' processes) | SY01, SY04 |
| X-S04 | `svc.network`, `svc.bluetooth`, `svc.display`, `svc.audio`, `svc.users`, `svc.time`, `svc.services` | Per-domain back ends for Settings and Quick Settings | SY01, SH02-GAP-1..4 |
| X-S05 | `svc.storage` | Disks, pools, datasets, snapshots (list, read-only handles, diff), mounts, ZFS accounting, dry-run estimates | AD01, AD02, AD03, SY02, UT04-GAP-3, SE05 |
| X-S06 | `svc.install`, `svc.logs`, `svc.policy` | Installer engine; log store and query; grant store and policy export | AD01, AD04, AD05, SH04-GAP-5 |
| X-S07 | `svc.monitor` | Sampling and handle inspection | SY04 |
| X-S08 | `svc.security`, `svc.firewall`, `svc.route`, `svc.mix` | Sensors and incidents; rules and first-contact decisions; route control; AEGIS | SE01, SE02, SE03 |
| X-S09 | `svc.vm`, `svc.jail`, `svc.compat`, `svc.remote`, `svc.device` | Virtualisation, containers, foreign apps, remote sessions, device policy | VM01–VM05 |
| X-S10 | `svc.timer` | Start an app or run a job at a set time; wake timers. Replaces resident daemons for reminders, send later, refresh, re-forecast | CO02, CO04, IN02-GAP-2, IN05-GAP-2, AI03, UT02 |
| X-S11 | `svc.fonts` | Validated fonts handed to helpers as read-only descriptors; install, enable, disable | OF03-GAP-3, UT03-GAP-2 |
| X-S12 | `svc.spell` | Spelling, hyphenation, dictionaries (the interface for G-17) | OF04-GAP-3 |
| X-S13 | `svc.docs` | Registry of installed help, manual and info collections | UT05-GAP-3 |
| X-S14 | `svc.shortcuts` | Global shortcut registration, owned by Window Management | ME05-GAP-3, AI01, CO04 |
| X-S15 | `svc.discovery` | mDNS and DNS-SD by capability | UT06-GAP-1 |
| X-S16 | App-provided services: naming, granting, display in Permissions, on-demand start of a headless bundle entry | CO03 and others |
| X-S17 | Interfaces for services already listed but undefined: camera, microphone and screen portals; `svc.media` (device ownership, mixer, low-latency sink, capture taps, echo reference, bit-perfect mode, hardware-decode broker); `svc.accounts` (one DAV account shared by three apps); `svc.keyring` ("sign this digest after a prompt", SSH agent, release of a secret to a helper); `svc.update` (time-zone and data packs); `svc.power` (wake timers); location (coarse or precise with a stated radius) | ME04-GAP-1, ME05-GAP-1, ME06-GAP-1, ME02-GAP-1, ME06-GAP-2, AX01-GAP-2, AX06-GAP-3, CO02, OF03-GAP-2, IN03-GAP-5, IN04-GAP-5 |
| X-S18 | Per-app data sizes and "clear this app's cache" | UT04-GAP-4 |
| X-S19 | `svc.capture` | Privileged BPF holder with verified filters; a socket-owner feed (app, domain, route, granting capability per local socket) | DV05 |
| X-S20 | `svc.debug` | Launch under a debugger, attach, read-only process-memory handles, all under policy | DV01, DV04 |

## 5. Platform: app host, intents, portals, manifest

| ID | Gap | Raised by |
|---|---|---|
| X-A01 | Disposable-sandbox launch: start an app instance in a fresh domain with a narrowed handle set and no network; a flag on intents; scoped-shell launch | SY02, SY03, CO01, CO05 |
| X-A02 | Launch of an unsigned or developer-signed bundle as its own confined instance (play mode, the IDE's run button) | CR04-GAP-3, DV01 |
| X-A03 | Native-entry path in the host for the one native app (D-27) | CR04-GAP-5 |
| X-A04 | An app starting many helpers, each with a different, narrower capability set chosen at run time | IN01-GAP-5 |
| X-A05 | Path-to-handle resolver in the launcher for command-line arguments and `$EDITOR` | OF01-GAP-2 |
| X-A06 | Elevated save: a prompt that returns a one-shot write handle to one system-owned file | OF01-GAP-1 |
| X-A07 | Reply channel on intents; an embedded-object edit flow with save-back | AI01, AI03, OF08-GAP-2 |
| X-A08 | Change events on a remembered file or folder grant | AI01, CO04 |
| X-A09 | Third-party providers for the file portal (open containers, remote file systems) | SE05, IN03 |
| X-A10 | Trusted fill: the shell's credential picker delivers a secret straight to the target field; passkey consent | SE04-GAP-3, IN01-GAP-3 |
| X-A11 | Trusted browser hand-off for OAuth2 and single sign-on that does not wait for the web engine | CO01, CO02, CO05 |
| X-A12 | Prompt interface for assistive tools: deny, arm a choice (D-88) | AX06-GAP-5 |
| X-A13 | Promise-style drag and drop | IN03-GAP-6 |
| X-A14 | A platform-wide provenance and quarantine tag (`user.pbsd.origin`), written by the portal and every writer; platform MIME types for sensitivity, provenance and file handles | SY02, IN03-GAP-4, AX05-GAP-4 |
| X-A15 | New intents: `scan`, `encrypt`, `decrypt`, `sign`, `verify`, `hash`, `clean-metadata`, `erase`, `identity-reset`, `open-with-route`, `set-wallpaper`, `check`, `convert` | SE01, SE05, SE03, ME01-GAP-3, DV06, OF03, OF04 |
| X-A16 | URL schemes `pbsd-storage:`, `pbsd-backup:`, `pbsd-logs:`, `pbsd-permissions:`; MIME registrations for `.fpk`, GGUF, ONNX | AD02–AD05, AI02 |
| X-A17 | Manifest fields (adopted, D-31): `optional_capabilities`, `capability_notes`, `system_helpers`, `keyring_scope`, `global_shortcuts`, `applet`, `entry_kind`, `native_exception`, `extensions`, `backup` rules, third-party tools and settings an assistant may change | many |
| X-A18 | A home in the bundle format for a shared signed library used by several apps (D-32) | OF08-GAP-1 |
| X-A19 | One shared capability-summary component so install review, Permissions and prompts word a capability the same way | SY05, AD05, SH04 |
| X-A20 | Registration of generated manifests for foreign apps; a Wayland proxy and X11 server per sandbox; an accessibility bridge for foreign apps | VM03 |
| X-A21 | Pty hand-off to Terminal by intent | VM02, VM03 |
| X-A22 | Device portal: pick a block device and get a rights-limited handle, read-only by default | DV04, AD02 |
| X-A23 | Exec-limited sandbox (a program may run only its own sub-programs); toolchain packages mounted read-only into build sandboxes | DV03, DV01 |

## 6. FUSE (beyond G-01 to G-18)

Many of these have a work package in the FUSE build plan already; its section 7.3 maps the spec gap IDs it saw.

| ID | Gap | Raised by |
|---|---|---|
| X-U01 | Maths typesetting (MathML and a LaTeX subset, OpenType MATH font) | AI04-GAP-1, OF07-GAP-2, OF04-GAP-2 |
| X-U02 | Chart v2: real axes, time axis, legends, stacking, error bars and bands, dual axes, hover, zoom, pie, box, heat map, streaming series, vector output; 3D plots | OF05-GAP-2, AI03-GAP-1, AI02-GAP-2, AI04-GAP-3, SY04, SE01 |
| X-U03 | Spreadsheet grid; typed, asynchronous, worker-backed item model; editable table cells; tree-table with inline editors; 100-million-row windowed list; bottom-anchored list with stable prepend; selection across rows; sparkline and bar cells | OF05-GAP-1, OF09-GAP-1, AI02-GAP-1, SE02, AD04-GAP-1, CO05, SY04 |
| X-U04 | Rich text additions beyond the plan: change tracking in the engine, total-fit line breaking, structure tree for tagged PDF, margin lane, baseline grid, per-block Markdown source spans; streaming append for `markdownView`; word diff | OF04, OF02-GAP-2, AI01-GAP-1, AI01-GAP-2 |
| X-U05 | Input controls: secret field with reveal-on-hold, tag and chip input, token search field, URL-aware line edit, key-capture field, one-line code field, range-pick mode, filter-chip bar, breadcrumb path bar | AD01-GAP-2, SE04-GAP-1, UT01-GAP-2, CO01, ME01-GAP-2, SY01, OF07-GAP-1, OF05-GAP-3, AD04-GAP-3, SY02 |
| X-U06 | Views: Miller columns, kanban board, resource time grid, Gantt, calendar heat map, history timeline strip, timeline lanes with point marks, treemap and sunburst, piece map, index strip, display-arrangement canvas, segmented bar with draggable boundaries, bottom sheet, ruler | SY02, CO04, CO02, AD04-GAP-2, AD05-GAP-2, AD03-GAP-1, SE01, UT04, IN03, CO03, SY01, AD02-GAP-1, IN04, OF04-GAP-1 |
| X-U07 | Editor widgets: scopes and colour wheels, piano-roll grid, EQ curve, flame graph, histogram, gradient and dash editors; `timeline` holds 32-bit float seconds today and needs exact time; `nodeGraph` automatic layout and a read-only mode at about 5,000 nodes | CR02-GAP-1, CR02-GAP-2, CR03-GAP-1, CR03-GAP-4, CR04-GAP-2, CR01-GAP-1, CR01-GAP-2, OF08-GAP-3, AD02-GAP-2, AD05-GAP-1, DV07-GAP-2 |
| X-U08 | Page view with variable page counts and non-page anchors; one show renderer shared by live show, thumbnails, PDF and video export; custom-paint canvas; animated image frames | OF03-GAP-1, OF06-GAP-1, UT02-GAP-1, CO05 |
| X-U09 | `viewportWidget` external frame source (shared memory or dmabuf with damage); zero-copy GPU image sharing between a helper and the app | VM01, VM04, CR01-GAP-3, CR02-GAP-3, CR04-GAP-1 |
| X-U10 | Terminal grid: per-row overlays and image cells; scroll-bar marks | SY03 |
| X-U11 | Accessibility action set: paste, text ranges, selection, named actions | AX01-GAP-1, AX05-GAP-6, AX06-GAP-7 |
| X-U12 | ForgeScript binding for the game UI library; the game UI sound hook wired to audio | GM01-GAP-1, GM01-GAP-2 |
| X-U13 | Hidden Qt link: `fuse_editor_api` links Qt when Qt is found and Vulkan is on, even with the editor off | FUSE plan |
| X-U14 | Engine shader toolchain on HardenedBSD (D-50) | CR04-GAP-4 |
| X-U15 | In-process `fileDialog` must be routed to the portal in suite apps | SH04 |
| X-U16 | Grammar file format shared by Text Editor, IDE and Notes must be frozen with a version field; a pf grammar | OF01, DV01, OF02, SE02 |
| X-U17 | Designer hooks: forced pseudo-class state, layout trace, hit-test to node id, widget registration list with property metadata; `.fuseui` has no include or component key and no animation keys | DV02 |
| X-U18 | Shared widgets: image compare, popover, inline decorations in the code editor, drag-reorder table rows, zoomable plot surface | DV01, DV03, DV04, DV05 |

## 7. FORGE (beyond G-01 to G-18)

| ID | Gap | Raised by |
|---|---|---|
| X-F01 | AES, HMAC-SHA-256 and HKDF in the crypto set | CO05 |
| X-F02 | Model keys and remote-device secrets are read from environment variables and files; move them to the keyring | AI02 |
| X-F03 | LoRA adapter converter | AI02 |
| X-F04 | Model classes not confirmed in the plan: OCR, keyword spotting, speech, vision, embeddings (only text-decoder import templates exist today) | UT06-GAP-2, AX06-GAP-2, FORGE plan |
| X-F05 | CLDR emoji annotations | AX04-GAP-4 |
| X-F06 | Handle-based SQLite file layer so a database can be opened from a portal descriptor | OF09-GAP-3 |
| X-F07 | REPL line editing, history and completion (Terminal adds its own front end meanwhile) | SY03 |
| X-F08 | Regex: confirm linear time and streaming across buffer pieces | OF01, AD04 |
| X-F09 | Findings from the FORGE plan's baseline that every app depends on: capabilities default to all-on; the FreeBSD sandbox is `cap_enter()` only and untested; `forge-serve` is TCP with no authentication; no TLS, X.509, OpenPGP or post-quantum code; `co_await` rejected; host bindings cross as JSON; W^X is emulated, not enforced | FORGE plan section 2 |
| X-F10 | ForgeScript test runner, debugger protocol and language server (in the FORGE plan as work packages 35 to 37; the IDE is blocked on them) | DV01 |

## 8. The other repos

| ID | Repo | Gap | Raised by |
|---|---|---|---|
| X-R01 | PRISM | HardenedBSD build unverified; its execution jail is Linux-only; no record format for the RUNTIME and TESTED labels the OS plan uses; remote build-worker agent with signed result archives; Lean proof search is tested only with a fake prover; the gate script and statement-freeze manifest named in the OS plan are not in this repo | DV06-GAP-2, DV06-GAP-3, DV06-GAP-4 |
| X-R02 | RetDec fork | No library API; no per-function re-decompile; about 28,000 lines of Qt GUI to remove; HardenedBSD build unverified (old LLVM pin, data fetched at set-up) | DV07-GAP-3, DV07-GAP-4, DV07-GAP-5 |
| X-R03 | MathScript | Interpreter returns formatted text and one plot per session, so kernels need a typed-value API; GPU backend is CUDA only; no decimal or exact-radical type; narrow symbolic integration; no control flow found | AI04-GAP-4, AI04-GAP-5, OF07 |
| X-R04 | Cypha | HardenedBSD build unverified; forecast module under repair; `cypha_rest` has no authentication (D-03) | AI03 |
| X-R05 | SENTINEL | Every source file includes Qt; removal list is in the Security Centre spec section 1.2 | SE01 |
| X-R06 | ANONYMOUS / AEGIS | No client receive path, no tunnel interface; HardenedBSD build unverified | SE03 |
| X-R07 | NeuralVFX | HardenedBSD build unverified; fetches encoders and a GUI library at configure time | CR02 |

## 9. Shared engines, data and release infrastructure

| ID | Gap | Raised by |
|---|---|---|
| X-D01 | Media beyond playback: encode, frame-accurate random access, several decoders at once; a real-time stack for calls (ICE, DTLS-SRTP, RTP, congestion control, echo cancellation) | CR02-GAP-4, CO05 |
| X-D02 | One audio engine shared by Music, Sound Recorder, Video Editor and Audio Editor (D-29) | CR02, CR03, ME02, ME06 |
| X-D03 | Shared helpers, one copy each (D-32): image codecs, archive, PDF engine, XML parser, HTML sanitiser, QR and barcode, ZIP/XML/CSV/Parquet readers, vector core and graph layout, iCalendar and DAV sync, recurrence engine | ME01, UT01, OF03, IN02, CO01, SE03-GAP-1, ME04, OF05, OF08, CO02 |
| X-D04 | A build pipeline and mirror for signed map region packages | IN04-GAP-4 |
| X-D05 | Offline data packs: gazetteer and time-zone polygons, time-zone updates, currency rates, holiday and address-format tables, lexicons, the compatibility database | IN05-GAP-4, CO02, OF07, AX04, VM03 |
| X-D06 | Sync between a user's own devices with no server account, used by Notes, Browser, Clipboard, Tasks and Passwords | OF02, IN01, AX05 |

## 10. Counts

| Section | Rows |
|---|---:|
| Compositor | 21 |
| Kernel and base system | 17 |
| Capabilities | 11 |
| Services | 20 |
| App host, intents, portals, manifest | 23 |
| FUSE | 18 |
| FORGE | 10 |
| Other repos | 7 |
| Shared engines, data, infrastructure | 6 |
| **Total** | **133** |
