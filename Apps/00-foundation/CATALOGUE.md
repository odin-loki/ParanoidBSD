<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# App catalogue

By Odin Loch

The full suite: 78 apps in 15 categories. Roles of the backing repos were corrected after the repos were read; see [DECISIONS.md](DECISIONS.md) section 1. Each app lives at `Apps/<category>/<slug>/` with a `SPEC.md` and a draft `manifest.json`. Bundle id is `org.pbsd.<slug>`.

Priority: **P0** the system cannot be used without it. **P1** needed for a daily-driver desktop. **P2** completes the suite. **P3** extra.

## shell — the desktop itself (replaces Plasma)

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| SH01 | `desktop-shell` | Desktop Shell | Panel, launcher, task switcher bar, desktop surface, system-wide search, command runner | P0 |
| SH02 | `notifications` | Notifications and Quick Settings | Notification centre, quick settings panel, status indicators (network, camera, mic, screen), on-screen displays | P0 |
| SH03 | `login-lock` | Login, Lock and Session | Login greeter, lock screen, authentication prompts, power and session dialogs, user switching | P0 |
| SH04 | `portals` | Portals and Prompts | File chooser, capability prompts, share sheet, open-with, print dialog, screen and window picker, colour picker | P0 |
| SH05 | `window-management` | Window Management | Decorations, snapping and tiling, workspaces, overview, window switcher, window rules, hot corners | P0 |

## system — core system apps

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| SY01 | `settings` | Settings | Every system setting: display, sound, network, input, power, users, appearance, privacy, defaults, date, region | P0 |
| SY02 | `files` | Files | File manager: browse, search, preview, tags, ZFS snapshots browser, remote locations, bulk rename | P0 |
| SY03 | `terminal` | Terminal | Terminal emulator: tabs, splits, profiles, multiplexing, capability-scoped shells, session recording | P0 |
| SY04 | `system-monitor` | System Monitor | Processes, capability domains, CPU, memory, disk, network, GPU, sensors, services, history | P0 |
| SY05 | `software` | Software | Install, update and remove packages and apps; signed updates; boot environments and rollback; reproducibility and SBOM view | P0 |

## admin — administration

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| AD01 | `installer` | Installer and First Run | Live installer (disk, ZFS, encryption, verified boot, users) and the first-run welcome and tour | P0 |
| AD02 | `disks` | Disks | Disks, partitions, ZFS pools and datasets, GELI encryption, SMART, benchmarks, image write and restore | P1 |
| AD03 | `backup` | Backup | Snapshot schedules, ZFS send and receive, encrypted off-site backups, restore browser | P1 |
| AD04 | `logs` | Logs | System logs, audit trail, capability grants and denials, crash reports, boot analysis | P1 |
| AD05 | `permissions` | Permissions | Every app's capabilities, the handle lineage tree, grants, revocation, per-resource history | P0 |

## security — security and privacy

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| SE01 | `security-center` | Security Centre | Security posture dashboard, integrity and verified-boot state, intrusion detection (new sensors); reuses SENTINEL's statistics, provenance and export code | P0 |
| SE02 | `firewall` | Firewall | pf rules, per-app network policy, live connections, blocklists, DNS policy | P0 |
| SE03 | `anonymity` | Anonymity | Network routes per app: clear, VPN, onion, and the AEGIS mix network for messages; identity separation | P1 |
| SE04 | `passwords` | Passwords and Keys | Password manager, passkeys, TOTP, SSH / OpenPGP / age / signing keys, certificates, hardware tokens | P0 |
| SE05 | `vault` | Vault | Encrypted containers and folders, file encryption and signing, secure erase, metadata cleaner, hash verify | P1 |

## virt — virtualisation, compatibility, hardware

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| VM01 | `virtual-machines` | Virtual Machines | Create and run VMs on BIFROST / bhyve: console, snapshots, devices, disposable VMs | P1 |
| VM02 | `jails` | Jails and Containers | Jails and Linux containers as capability domains: templates, networking, resource limits | P2 |
| VM03 | `compatibility` | Compatibility | Run Windows, Linux and Android apps through the personalities: prefixes, per-app sandboxes, launchers | P2 |
| VM04 | `remote-desktop` | Remote Desktop | Client and server for remote sessions (RDP, VNC, SSH), file and clipboard policy | P2 |
| VM05 | `devices` | Devices | Device tree, drivers and UDA descriptors, firmware, USB policy, Bluetooth, IOMMU groups | P1 |

## office — productivity

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| OF01 | `text-editor` | Text Editor | Fast plain-text and code editor for everyday editing | P0 |
| OF02 | `notes` | Notes | Notebooks of rich Markdown notes, links, tags, attachments, encrypted notebooks | P1 |
| OF03 | `document-viewer` | Document Viewer | PDF, EPUB, DjVu, comics, XPS: read, annotate, fill forms, sign, redact | P1 |
| OF04 | `writer` | Writer | Word processor: styles, page layout, tables, references, track changes, ODF and DOCX | P1 |
| OF05 | `sheets` | Sheets | Spreadsheet: formulas, charts, pivot tables, large data, ODS, XLSX, CSV | P1 |
| OF06 | `slides` | Slides | Presentations: masters, animations, presenter view, ODP and PPTX | P2 |
| OF07 | `calculator` | Calculator | Basic, scientific, programmer, units, currency (offline), graphing; MathScript engine | P1 |
| OF08 | `draw` | Draw | Vector drawing and diagrams: shapes, connectors, layers, SVG | P2 |
| OF09 | `data-studio` | Data Studio | Database browser and query tool for SQLite and network databases, CSV and Parquet explorer | P2 |

## comms — communication

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| CO01 | `mail` | Mail | Email: IMAP, JMAP, SMTP, OpenPGP and S/MIME, filters, offline, search | P1 |
| CO02 | `calendar` | Calendar | Calendars: CalDAV, iCalendar, scheduling, reminders, time zones | P1 |
| CO03 | `contacts` | Contacts | Address book: CardDAV, vCard, keys per contact, merge | P1 |
| CO04 | `tasks` | Tasks | To-dos and projects: lists, boards, recurring, CalDAV VTODO | P2 |
| CO05 | `messenger` | Messenger | End-to-end encrypted chat and calls: Matrix, XMPP, IRC; anonymous transport option | P1 |

## internet

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| IN01 | `browser` | Web Browser | Hardened browser with no JIT: per-site processes, tracker blocking, containers, reader mode | P1 |
| IN02 | `feeds` | Feeds | RSS, Atom and JSON feeds, podcasts, read later, offline articles | P2 |
| IN03 | `transfers` | Transfers | Downloads and file transfer: HTTP, SFTP, FTP, BitTorrent, WebDAV, rsync, local device send | P2 |
| IN04 | `maps` | Maps | Offline vector maps, search, routing, GPX tracks; no location unless granted | P2 |
| IN05 | `weather` | Weather | Forecasts from open sources with privacy-preserving location | P3 |

## media — playback and capture

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| ME01 | `photos` | Photos | Image viewer and photo library: albums, RAW, metadata, quick edits, local face and object search | P1 |
| ME02 | `music` | Music | Music library and player: gapless, playlists, tags, internet radio, podcasts hand-off | P1 |
| ME03 | `video` | Video | Video player: subtitles, tracks, streaming, casting to local devices | P1 |
| ME04 | `camera` | Camera | Photo and video capture, document scan, QR codes | P2 |
| ME05 | `screen-capture` | Screen Capture | Screenshots, annotations, screen recording, GIF, OCR | P1 |
| ME06 | `sound-recorder` | Sound Recorder | Voice and audio recording, trimming, local transcription | P2 |

## create — creative tools

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| CR01 | `image-editor` | Image Editor | Raster editor: layers, masks, brushes, filters, colour management | P2 |
| CR02 | `video-editor` | Video Editor | Non-linear video editor with effects and compositing; NeuralVFX supplies generated effects | P2 |
| CR03 | `audio-editor` | Audio Editor | Multitrack audio editing, effects, MIDI, mastering | P3 |
| CR04 | `studio` | Studio | The FUSE editor on its Qt-free shell, shipped as the one native app: 2D and 3D worlds, simulation, games | P2 |

## dev — development

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| DV01 | `ide` | IDE | ForgeScript and C++23 IDE: language servers, debugger, build, tests, safe-subset and PRISM gates inline | P1 |
| DV02 | `ui-designer` | UI Designer | Visual designer for `.fuseui` documents and style sheets | P2 |
| DV03 | `version-control` | Version Control | Git client: history, staging, diff, merge, signing | P2 |
| DV04 | `hex-editor` | Hex Editor | Binary editor: structure templates, diff, entropy, disassembly hand-off | P2 |
| DV05 | `packet-analyser` | Packet Analyser | Capture and dissect network traffic; flows, filters, export | P2 |
| DV06 | `proof-workbench` | Proof Workbench | Front end for PRISM: jobs, obligations, counterexamples, proof tiers, reports | P1 |
| DV07 | `reverse-engineering` | Reverse Engineering | Decompiler workbench; front end for the RetDec fork (replaces its Qt GUI) | P2 |

## ai — AI and science

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| AI01 | `assistant` | Assistant | Local assistant on FORGE models: chat, documents, tools with capability prompts, no cloud; CyphaLM as an optional mode | P1 |
| AI02 | `model-manager` | Model Manager | Import, verify, sign, benchmark and serve `.fpk` models with FORGE | P1 |
| AI03 | `forecast` | Forecast | Time-series forecasting studio: classical methods plus Cypha regressors and uncertainty | P2 |
| AI04 | `notebook` | Notebook | Computational notebook for MathScript and ForgeScript: cells, plots, symbolic maths | P2 |
| AI05 | — | Kodai | Unverified: private repo, not read. No spec until its purpose is known. | — |

## utils — utilities

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| UT01 | `archives` | Archives | Create, browse and extract archives; encrypted archives; safe extraction | P1 |
| UT02 | `clocks` | Clocks | World clock, alarms, timers, stopwatch | P2 |
| UT03 | `fonts` | Fonts and Characters | Font manager and viewer, character map, glyph inspector | P2 |
| UT04 | `disk-usage` | Disk Usage | Where the space went: treemap, duplicates, snapshot space, cleanup | P2 |
| UT05 | `help` | Help | Handbook, man pages, app help, offline search | P1 |
| UT06 | `print-scan` | Printers and Scanners | Print queues, printer setup, scanning with OCR | P2 |

## access — accessibility and input

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| AX01 | `screen-reader` | Screen Reader | Speech and braille output for the whole desktop; local voices | P1 |
| AX02 | `magnifier` | Magnifier | Zoom, lens, colour filters, pointer and focus highlighting | P1 |
| AX03 | `on-screen-keyboard` | On-Screen Keyboard | Touch and pointer keyboard, word prediction, switch access scanning | P2 |
| AX04 | `input-methods` | Input Methods and Emoji | Input method framework, layouts, compose, emoji and symbol picker | P1 |
| AX05 | `clipboard` | Clipboard | Clipboard history with sensitivity rules, pins, snippets | P2 |
| AX06 | `voice` | Voice | Local dictation and voice control | P3 |

## games

| ID | Slug | Name | What it is | Pri |
|---|---|---|---|---|
| GM01 | `games` | Games | A small pack built on FUSE: solitaire, mines, sudoku, chess, tiles, a game hub | P3 |

## Other repos on the profile

| Repo | In the suite | Reason |
|---|---|---|
| FUSE, FORGE, PRISM, MathScript, Cypha, RetDec-Decompiler, SENTINEL, ANONYMOUS, NeuralVFX | yes | named as the distro's components |
| Kodai | undecided | private; purpose Unverified |
| Hypercube-Gated-Linear-Network, CellAI, TRACE, HPv40-Compression-Algorithm, CompressionAlgorithm, CameraDetector, RallyTek, Ideas, Music, Mnemonic-Meditation, Extreme-Sports-Drink, NetworkHeaders, Seek, ToteBet, assets, odin-loki.github.io | not planned here | not named as distro components; not read |
