<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Decisions

By Odin Loch

Every open choice raised while planning the suite, and what was chosen. The instruction was: pick all of them, choosing **functionality and simplicity**. The test applied to each: which option gives the user the most working function with the fewest new parts to build and maintain.

Where a spec or build plan says something different from this file, **this file wins**; the "Affects" column names what must be edited to match. Any decision can be reversed by editing this file and the specs it names.

Nothing here has been built or tested. A decision is a plan choice, not a result.

## 1. What the backing repos really are

The catalogue was first written from the repo names. Reading the repos changed four roles. The specs follow the repos.

| ID | Repo | What it is (Verified, cited in the spec) | Decision | Affects |
|---|---|---|---|---|
| D-01 | Cypha (CyphaLM) | A byte-level trie model that answers stored questions only | Assistant chats with FORGE text-generation models through the model service. CyphaLM is an optional later completion mode. | AI01 |
| D-02 | Cypha (forecast module) | Conflict-event forecasting; its own docs call it a scaffold under repair | Forecast gets new classical time-series code in C++23. Cypha supplies regressors, uncertainty and anomaly scores. The event module is a later, separate workspace. | AI03 |
| D-03 | Cypha (`cypha_rest`) | Loopback HTTP server with no authentication that takes a server-side path from the request | Not shipped. The Forecast helper links the Cypha core in-process. | AI03 |
| D-04 | NeuralVFX | CPU neural effects for games (fire, smoke, water, weather). No timeline, codecs, compositing or grading | Video Editor uses it for generated effect elements only. The editing engine is new work. | CR02 |
| D-05 | SENTINEL | A crime-analytics desktop program on Qt 6. No host security code | Security Centre reuses its statistics, provenance and export code after Qt is removed. Host sensors (boot, integrity, hardening, intrusion) are new work. | SE01 |
| D-06 | ANONYMOUS / AEGIS | A message mix network for a permissioned consortium. No tunnel, no onion routing, no client receive path | Anonymity offers four routes: clear, VPN (WireGuard from the base tree), onion (D-22), and AEGIS as a message-only route. | SE03, CO05 |
| D-07 | RetDec fork | Decompiler whose Qt GUI drives it as a subprocess; no library API | Reverse Engineering first gets a library API with per-function re-decompile, then the new front end. | DV07 |
| D-08 | MathScript | A computer algebra system with LaTeX and MathML output. Not a typesetter. No loops, conditionals or user functions found in its guide | Used as the maths engine for Calculator, Sheets and Notebook. Maths typesetting is separate FUSE work. Notebook cells that need control flow use ForgeScript. | OF07, AI04, OF04 |

## 2. Platform

| ID | Question | Decision | Why | Affects |
|---|---|---|---|---|
| D-10 | Where the app host source lives | In this repo, linking a FORGE runtime library and the FUSE binding. FORGE keeps a headless host for tests. | FORGE must not depend on FUSE; one host for all apps. | FORGE plan Q1 |
| D-11 | When the host enters capability mode | After it has verified the bundle. Starting already confined is a later hardening step. | No statically linked host or launcher needed for the first milestone. | FORGE plan Q2 (differs from its recommendation) |
| D-12 | Default-deny | A new `app` profile is default-deny now. FORGE's own default flips at its next minor release. | Does not break existing FORGE users. | FORGE plan Q3 |
| D-13 | Python at build time | Commit the generator outputs for the OS build; port generators to C++ over time. | Offline build with no Python needed. | FORGE plan Q4 |
| D-14 | Bundle hashing at launch | Trust the verified dataset; full check at install and update, and whenever the dataset is not in a verified state. | Fast start. | FORGE plan Q5 |
| D-15 | TLS | rustls in a per-app helper with no other capabilities. | Memory-safe; Rust is in the tool set anyway (D-21). | FORGE plan Q6 |
| D-16 | Post-quantum | From the TLS library where it has ML-KEM and ML-DSA; otherwise one dedicated library. | One source. | FORGE plan Q7 |
| D-17 | SQLite | In the host for an app's own stores; in a helper for database files from elsewhere. | Fast for the common case, contained for hostile files. | FORGE plan Q8, OF09 |
| D-18 | `finally` in ForgeScript | No. Scope guards only. | Scripts stay valid C++23. | FORGE plan Q9 |
| D-19 | How apps are distributed | Bundles inside OS packages now; a separate signed archive for third parties later. | One install path to build first. | FORGE plan Q10, SY05 |
| D-20 | Key revocation and a running app | Quit at once if the key is marked compromised; finish the session if it only expired. | | FORGE plan Q12 |
| D-21 | Rust in the offline build | Yes, vendored. | AEGIS needs it; it also serves TLS, the onion route and chat encryption. | FORGE plan Q14 |
| D-22 | Onion route | Arti (Rust) in a helper. | Memory-safe; no second language added. | SE03 |
| D-23 | How an app's sockets are bound to a route | The host sets a routing table for the app at launch, and the kernel labels sockets with the app so pf can match them. It fails closed. | Uses existing routing and pf; no per-socket proxy. | SE02, SE03, IN01, IN03, FORGE-WP-21 |
| D-24 | GPU for on-device models | Allowed in an isolated worker process when a supported GPU is present, shown in Permissions. CPU is the fallback. | Models are slow without it. The exemption is confined to one process. | FORGE plan Q13 (differs from its recommendation) |
| D-25 | `forge-serve` on the desktop | Not installed by default. The model service is the only model interface for apps. Model Manager can install serving as an add-on. | No listening socket by default. | FORGE plan Q15, AI02 |
| D-26 | Which models ship | Decided per class when the model work reaches it; licence and signer recorded in the model library. | | FORGE plan Q11 |
| D-27 | Native apps | Allowed only as a named exception in the manifest (`entry_kind: native`), confined, signed, and gated like FUSE code. Studio is the only one. | See D-40. | PLATFORM.md section 2 |
| D-28 | Imported C or C++ engines that are not in the safe subset | Allowed inside helpers with no capabilities, one job per process, when writing our own would cost years. Each is listed in section 6. | Function first; containment replaces proof (tier T4). | PLATFORM.md section 2 |
| D-29 | Audio processing code | One platform audio engine inside the host, driven from ForgeScript. Apps ship no native in-process code. | Real-time audio cannot cross a process boundary per sample; one shared engine keeps the rule simple. | ME02, ME06, CR02, CR03 |
| D-30 | Services the specs added | Adopted. Listed in PLATFORM.md section 14. | The apps cannot work without them. | PLATFORM.md |
| D-31 | Manifest fields the specs added | Adopted. Listed in PLATFORM.md section 14. | | PLATFORM.md |
| D-32 | Shared library code (XML parser, HTML sanitiser, archive helper, image codecs, QR, PDF engine) | One copy each, as platform helpers under `Apps/00-foundation/shared/`, not per app. | Fewer parsers to secure. | ROADMAP.md section 4 |
| D-33 | One SSH implementation | OpenSSH from the base tree, run in a sandboxed helper; keys stay in the Passwords and Keys agent. | Already in the tree and trusted. | SY03, IN03 (replaces its libssh2 choice), VM04, DV03, SY02 |
| D-34 | Trust scale for people and keys | unknown, seen, introduced, verified, changed, revoked. Public keys live in Passwords and Keys; Contacts stores references. | One scale across Mail, Messenger and Contacts. | SE04, CO01, CO03, CO05 |
| D-35 | Codecs | Royalty-free codecs by default. Patent-encumbered ones (H.264, HEVC, AAC, AC-3, DTS) as one optional add-on. | Works out of the box; the user chooses the rest. | ME01–ME06, CR02 |

## 3. FUSE

| ID | Question | Decision | Affects |
|---|---|---|---|
| D-40 | Studio front end | Ship FUSE's existing Qt-free C++ editor shell as the app (D-27). ForgeScript is the project scripting and extension language. | CR04 (revised), FUSE plan Q-13 |
| D-41 | Binding interface | FORGE's FFI header is the single source for handles and batches; FUSE mirrors it. FUSE evaluates built-in `.fuseui` expressions; FORGE the rest. | FUSE plan Q-01 |
| D-42 | Wayland client | Vendor the reference C client library now, behind a seam. | FUSE plan Q-02 |
| D-43 | Window decorations | Apps draw the header bar. The compositor draws only the security border. | FUSE plan Q-03, SH05 |
| D-44 | Default presenter | GPU when a hardware driver passes the mapping audit, else the CPU rasteriser. CPU everywhere until that audit has run. | FUSE plan Q-04 |
| D-45 | Software rendering | The CPU rasteriser only. No software Vulkan. | FUSE plan |
| D-46 | Embedding another process's surface | Buffers handed to the embedder and drawn by FUSE. | FUSE plan Q-05 |
| D-47 | Missing compositor protocols | Use existing public protocols where they exist; define our own only for the trusted surface and thumbnails. | FUSE plan Q-06, GAPS.md |
| D-48 | Hostile fonts and colour profiles | Installed fonts in process after a sanitiser; embedded fonts and profiles in helpers. | FUSE plan Q-07 |
| D-49 | Huge mapped files that may change | A ZFS snapshot or private copy handle from the host. | FUSE plan Q-08 |
| D-50 | Engine shaders | Ship precompiled shaders. | FUSE plan Q-09 |
| D-51 | Subpixel text | Greyscale only. | FUSE plan Q-10 |
| D-52 | Fonts | Keep the bundled UI face; add a monospace face and wider script coverage. | FUSE plan Q-11 |
| D-53 | Accessibility adapter | Native adapter on the accessibility service. No AccessKit. | FUSE plan Q-12 |
| D-54 | Opaque `void*` tables in FUSE UI | Migrate to typed inline functions and handles, in stages. | FUSE plan Q-14 |

## 4. Shell and compositor

| ID | Question | Decision | Affects |
|---|---|---|---|
| D-60 | `Super+1..9` | Workspaces. Task entries use `Super+Alt+1..9`. | SH01, SH05 |
| D-61 | Window policy | Built into the compositor in C++, with rules as data from the settings store. No separate policy service. | SH05 (replaces its ForgeScript policy service) |
| D-62 | Clipboard data-control protocol | Bound to the Clipboard service only; reading needs a paste gesture. | SH04, AX05 |
| D-63 | Shell build order | Login and Lock, Portals, Window Management, Desktop Shell, Notifications. | ROADMAP.md |
| D-64 | Security labels on windows | One vocabulary owned by Window Management: native, sandboxed, disposable, vm, foreign, elevated. | SH01–SH05, AD05, VM01, VM03 |
| D-65 | Shortcuts in Terminal | `Ctrl+letter` goes to the shell. Terminal's own shortcuts use `Ctrl+Shift`. | SY03, DESIGN_SYSTEM.md section 6 |
| D-67 | `Ctrl+P` in the IDE | The reserved keys stand: `Ctrl+P` is Print everywhere. Go to File in the IDE is `Ctrl+Shift+O`, and is also in the command palette. | DV01 (replaces its `Ctrl+P` choice) |
| D-66 | Home folder access for Files and Terminal | Granted read-write to the home folder, shown at install and in Permissions. Scoped shells remain available. | SY02, SY03, AD05 |

## 5. Apps

| ID | Question | Decision | Affects |
|---|---|---|---|
| D-70 | Web engine | Two tracks. An own script-free renderer for Mail, Feeds, reader mode and archives. For the general web, import WebKit with its JavaScript engine in interpreter-only mode, drawing into a FUSE surface; Chromium in jitless mode is the fallback. The first work item is a spike that builds and measures it on HardenedBSD. | IN01 |
| D-71 | Browser extensions | None. Signed, data-only add-ons (filter lists, search engines, themes). | IN01 |
| D-72 | HTML in Mail | The restricted renderer, not the web engine. | CO01 |
| D-73 | Help | No web engine. mandoc emits a document tree that the app draws. | UT05 |
| D-74 | Documents: PDF, PostScript, DjVu | MuPDF, Ghostscript and DjVuLibre in helpers (D-28). | OF03, UT06 |
| D-75 | Office formats | Own code, ODF first, then DOCX, XLSX and PPTX. | OF04, OF05, OF06 |
| D-76 | Image Editor native format | OpenRaster with documented extensions. | CR01 |
| D-77 | Audio plugins | LV2 and CLAP, one per helper, no plugin windows. | CR03 |
| D-78 | Remote Desktop protocol code | Import FreeRDP in a per-session helper. | VM04 (replaces own-code RDP as the first choice) |
| D-79 | Messenger encryption and calls | Import vetted Rust libraries in helpers. Text first; calls in a later phase. | CO05 |
| D-80 | Linux containers and PBSD handles | Follow the OS plan: containers are capability domains. The older compatibility note is out of date. | VM02, VM03 |
| D-81 | Guests | Any operating system, not only Windows. | VM01 |
| D-82 | Translating foreign binaries | Allowed only inside the Compatibility domain, as a labelled exception to the no-JIT rule. Never in a native app. | VM03 |
| D-83 | Virtual networks | One layer shared by Virtual Machines and Jails. | VM01, VM02 |
| D-84 | Tracing in System Monitor | DTrace only. | SY04 |
| D-85 | Braille | liblouis in a helper. | AX01 |
| D-86 | First speech voice | eSpeak NG in a helper. Neural voices later. | AX01 |
| D-87 | Keyboard layouts | libxkbcommon in a helper. | AX04 |
| D-88 | Voice and security prompts | Voice can deny a prompt. It can never approve one. No spoken second factor. | AX06, SH04 |
| D-89 | Installer wireframes | Seven first-run pages are described in one line each rather than drawn. Accepted. | AD01 |
| D-91 | Git | Own safe-subset reader for open and browse, so opening a repository never runs Git. Writes and transport use the `git` binary in an exec-limited sandbox at first, replaced step by step. No libgit2. | DV03 |
| D-92 | Languages defined in the specs | The binary template language (Hex Editor) and the packet filter language (Packet Analyser) are accepted as specified. | DV04, DV05 |
| D-90 | Kodai | No spec until the repo can be read. | CATALOGUE.md |

## 6. Imported third-party code (all in helpers with no capabilities unless stated)

Licences are as the specs state them and must be confirmed before import.

| Library | Used for | Licence (to confirm) | Decision |
|---|---|---|---|
| WebKit (JavaScript engine interpreter-only) | General web | LGPL-2.1 / BSD | D-70 |
| MuPDF, Ghostscript | PDF, PostScript | AGPL-3.0-or-later | D-74 |
| DjVuLibre | DjVu | GPL-2.0-or-later | D-74 |
| libarchive | Archives | BSD (in the base tree) | UT01 |
| mandoc | Manual pages | ISC-style (in the base tree) | D-73 |
| OpenSSH | SSH | BSD (in the base tree) | D-33 |
| WireGuard | VPN | in the base tree | D-06 |
| rustls | TLS | Apache-2.0 / MIT / ISC | D-15 |
| Arti | Onion route | Apache-2.0 / MIT | D-22 |
| FreeRDP | Remote Desktop | Apache-2.0 | D-78 |
| libavcodec (cut down) | Optional codec add-on | LGPL-2.1-or-later | D-35 |
| liblouis | Braille | LGPL-2.1-or-later | D-85 |
| eSpeak NG | Speech | GPL-3.0-or-later | D-86 |
| libxkbcommon | Keyboard layouts | MIT | D-87 |
| Wayland client library | Windowing | MIT | D-42 |
| HarfBuzz, FreeType, SheenBidi, libunibreak | Text | permissive | FUSE plan |
| SQLite | Stores | public domain | D-17 |
| Wine, DXVK, vkd3d, AOSP parts | Compatibility domain only | LGPL / zlib / Apache-2.0 | VM03 |
| LV2 and CLAP hosts | Audio plugins | ISC / MIT | D-77 |

## 7. Still open

These could not be decided from the repos.

| ID | Question | Needed from |
|---|---|---|
| O-01 | Does `distro-build-plan-v4.md` name apps or choices that differ from this plan? | The file itself |
| O-02 | What is Kodai? | Access to the private repo |
| O-03 | Does FUSE build and pass its gates on HardenedBSD with clang and libc++? | A build on HardenedBSD |
| O-04 | Does the chosen web engine build and run acceptably without JIT on HardenedBSD? | The spike in IN01 |
| O-05 | Can PRISM check ForgeScript, or only C++? | PRISM's maintainer |
| O-06 | Licences of lexicons, voices, map data, weather sources and trained models | Per item, before import |
