<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# App platform

By Odin Loch

How every app in `Apps/` is built, packaged, confined and connected. Every app spec refers to this file by section number instead of repeating it.

Labels used in this folder: **Verified** = read in a repo at the cited path. **Proposed** = design in this plan, nothing built. **Unverified** = could not be read.

## 1. Ground truth (Verified, 12 Nov 2026)

Read from shallow clones of `odin-loki/FUSE` and `odin-loki/FORGE` at `main`, and from this repo.

| # | Fact | Source |
|---|---|---|
| V1 | FUSE has its own GUI framework, FUSE UI: a retained core styled with a CSS subset, a backend-neutral draw list, a CPU rasteriser and a Vulkan compute backend. | `FUSE:docs/ui/README.md` |
| V2 | FUSE UI rules: handles not pointers, `std::expected`, one owner thread (others `post()`), no steady-state allocation. | `FUSE:docs/ui/README.md` |
| V3 | Widgets in tree: button, toolButton, checkBox, radioButton, toggleSwitch, slider, dial, spinBox, lineEdit, comboBox, label, link, image, separator, progressBar, panel, scrollArea, scrollBar, splitter, tabBar, tabWidget, dockArea, dockWidget, menuBar, popupMenu, dialog, tooltip, listView, modelListView, treeView, tableView, gridView, propertyGrid, colorPicker, calendar, dateTimeEdit, chart, markdownView, fontPicker, fileDialog, nodeGraph, curveEditor, timeline, viewportWidget, codeEdit, videoWidget (frame-source stub). | `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp`, `FUSE:docs/ui/README.md` |
| V4 | UI documents are `.fuseui` JSON trees with `{expr}` and `{=two-way}` bindings. Script binding today is Lua 5.2 only. | `FUSE:docs/ui/README.md` |
| V5 | The app shell `fuse_app` has `Application` / `AppWindow`, an on-demand loop, settings, command line and single instance. Native windows are X11 and Win32 with a Vulkan swapchain presenter. No Wayland backend in the UI app shell. | `FUSE:docs/ui/README.md` |
| V6 | Text: MINIMAL profile (stb_truetype, bundled Open Sans) by default. FULL profile (HarfBuzz, FreeType, SheenBidi, libunibreak) fetches sources at configure time. | `FUSE:docs/ui/README.md` |
| V7 | Accessibility bridge exists (snapshot diff, action queue). The AccessKit adapter needs a Rust toolchain. | `FUSE:docs/ui/README.md` |
| V8 | `fuse_ui_sql::SqliteModel` gives an item model over a SQLite query. | `FUSE:docs/ui/README.md` |
| V9 | The FUSE editor still has a Qt 6 shell; a Qt-free shell (`-DFUSE_EDITOR_SHELL=fuse`) ports all ten panels. | `FUSE:README.md`, `FUSE:docs/ui/README.md` |
| V10 | ForgeScript is `forge script`: an interpreted, memory-safe subset of C++23 with a bytecode VM and a tree-walker. Raw pointers, `new`/`delete`, casts and pointer arithmetic are rejected by the parser. | `FORGE:docs/scripting.md` |
| V11 | ForgeScript memory: value types, atomic refcounts, a cycle collector, bounds checks, `BorrowError` on use-after-move and on mutation during iteration. | `FORGE:docs/scripting.md` |
| V12 | ForgeScript capabilities: `fs`, `net`, `proc`, `env`, `reflect`. They only narrow. A security policy (FSS) makes file, network, process and env access path-scoped and default-deny. | `FORGE:docs/scripting.md` |
| V13 | OS sandbox mapping exists for Capsicum on FreeBSD; only the Linux mechanism has an automated test. | `FORGE:docs/scripting.md` |
| V14 | Scripts and plugins can be signed: Ed25519 over BLAKE3, detached `.sig`, bound to the file name; `integrity.scripts = required` refuses unsigned code. | `FORGE:docs/scripting.md` |
| V15 | Embedding: `forge::script::engine` with `define(name, min, max, fn)` host bindings. Values cross as JSON text; script callbacks arrive as `{"$callback": id}`. | `FORGE:docs/scripting.md` |
| V16 | `engine::options::jit_threshold` is a call count after which a function's AST is compiled to **bytecode**, not native code; the VM is a switch loop over `Instr` records and `script/src` maps no executable memory. Executable mappings can still arrive through `dlopen` (native plugins, GPU and OS libraries). Read in source, not run. | `FORGE:script/include/forge/script/script.hpp:23`, `FORGE:script/src/runtime.cpp:729-730`, `FORGE:script/src/vm.cpp:220-229,329-337` |
| V17 | ForgeScript has no script bindings for models, GPU or serving yet. | `FORGE:docs/scripting.md` |
| V18 | FORGE runs models from `.fpk` files, has an OpenAI-compatible server `forge-serve`, and an OS layer with sandboxing, keystore, IPC, services and text/locale. | `FORGE:README.md` |
| V19 | PBSD's native model is handles with rights and lineage: rights only narrow (SI-1), no ambient authority (SI-2), W^X everywhere (SI-3), revocation is total (SI-6). | `docs/security/INVARIANTS.md`, `docs/specs/HANDLES.md` |
| V20 | The current repo plan (P5-02) ships Plasma 6 / KWin on Wayland. This folder plans the Qt-free suite that replaces it. | `docs/plan/PBSD_PLAN.md` |

Unverified: `distro-build-plan-v4.md` (on the author's PC, not in any repo read here); repo `Kodai` (private).

## 2. The three languages

| Layer | Language | What lives here | Rules |
|---|---|---|---|
| App logic | ForgeScript | All app behaviour, view models, document logic, glue | Interpreted only. Signed. Runs under an FSS policy generated from the manifest. |
| UI | FUSE UI | `.fuseui` documents, CSS, widgets, rendering | No Qt. One UI thread. Zero steady-state allocation. |
| Native services | ISO memory-safe C++23 | Codecs, parsers for hostile data, crypto, indexers, engines, daemons | MathScript's Clang plugin enforces the subset; PRISM gates the merge. |

An app is ForgeScript first. Native code is allowed only as a **helper service** in its own process when one of these holds: (a) it parses untrusted bytes, (b) it needs more speed than the interpreter gives, (c) it holds a secret, or (d) it wraps an existing engine (FORGE, Cypha, RetDec, PRISM, NeuralVFX, SENTINEL, AEGIS). Rust code (ANONYMOUS/AEGIS) is treated as a native helper behind the same IPC boundary.

## 3. App bundle

```
<id>.app/
  manifest.json          identity, entry points, capabilities, intents, MIME types, helpers
  main.fs                ForgeScript entry
  src/*.fs               modules
  ui/*.fuseui            windows, dialogs, panels
  style/*.css            app styles on top of the system tokens
  locale/<lang>.tbl      key=value string tables
  assets/                icons (SVG subset or pre-rasterised), sounds, templates
  helpers/<name>         signed native helper binaries (optional)
  help/*.md              in-app help, rendered by markdownView
  SIGNATURES             BLAKE3 tree hash + Ed25519 signature over every file
```

Bundles are read-only, installed on a verified dataset, and never write into themselves. `manifest.json` schema is in section 12.

## 4. Process model

```
compositor ── shell ── app host (one per app instance)
                          ├─ ForgeScript engine  (app logic, FSS policy from the manifest)
                          ├─ FUSE UI context     (UI thread, draw list → compositor surface)
                          └─ IPC endpoint        (session bus, portals, own helpers)
helper services (one process each, Capsicum capability mode, only the handles they were passed)
system services (settings store, notifications, search index, keyring, portals, ...)
```

- **App host**: one C++23 binary shared by every app. It verifies the bundle signature, builds the FSS policy from the manifest, enters capability mode, creates the engine and the UI context, and exposes the bindings in section 5. Apps do not ship their own host.
- **No ambient authority**: the host starts with only the handles the manifest allows. Anything else arrives later as a handle from a portal after the user chose it.
- **Crash containment**: an app, a helper or a system service can crash and restart without taking the session down. Windows of a restarted app reattach by session token.
- **No JIT, no W+X**: the host maps no writable-executable memory. The interpreter compiles to bytecode only (V16); what remains is to enforce and test it on HardenedBSD and to close the `dlopen` paths (gap G-05).

## 5. Script-to-UI binding (Proposed)

FUSE UI is scripted from Lua today (V4). The suite needs a ForgeScript binding with the same surface:

- `ui::load(path, model)`, `ui::find(id)`, `ui::set(node, prop, value)`, `ui::get`, `ui::connect(node, signal, fn)`, `ui::post(fn)`.
- `ui::model<T>` observable view models; `.fuseui` bindings `{expr}` / `{=two-way}` resolve against them.
- `ui::ItemModel` for list, tree, table and grid views, backed by script containers or by `SqliteModel`.
- `ui::action(id, label, shortcut, fn)` so menus, toolbars, the command palette and shortcut settings share one action table.
- `ui::dialog`, `ui::menu`, `ui::toast`, `ui::drag`, `ui::drop`, `ui::clipboard`.

V15's JSON crossing is too slow for per-frame paths, so the binding adds typed handles and batched property updates (gap G-02).

## 6. Capabilities and confinement

Manifest capabilities map onto three enforcement layers at once: the ForgeScript `Caps` flags and FSS path scopes (V12), Capsicum rights on every descriptor, and PBSD handle rights (V19).

| Manifest capability | Grants | Default |
|---|---|---|
| `fs.app` | Own data, config and cache directories | always |
| `fs.portal` | Files and folders the user picks in the file portal; grants can be remembered per app | on |
| `fs.path:<path>:<r\|rw>` | A fixed path; shown to the user at install | off |
| `net.none` | No sockets except the session bus | default |
| `net.host:<host>:<port>` | Named endpoints only | off |
| `net.user` | Endpoints the user types (browser, mail, chat); each first contact is logged | off |
| `net.route:<clear\|vpn\|tor>` | Which route the app's sockets may use | `clear` |
| `proc.helper:<name>` | Start a helper shipped in the bundle | off |
| `proc.exec` | Run other programs (Terminal, IDE) | off |
| `dev.camera`, `dev.mic`, `dev.screen`, `dev.location`, `dev.usb:<class>`, `dev.print`, `dev.gpu.compute` | Device portals; a visible indicator is on while in use | off, prompt on first use |
| `svc.<name>` | Talk to a system service (section 8) | per service |
| `ai.local` | Use on-device models through the model service | off |
| `bg.run`, `bg.autostart` | Keep running without a window; start at login | off |

Rules:

1. Default deny. An app with an empty capability list can draw windows and keep its own data, nothing else.
2. Capabilities only narrow at run time. A helper never gets more than its app.
3. Prompts are per capability and per resource, are shown by the shell (not the app), and say what, why and for how long (once, this session, always).
4. Every grant is visible and revocable in the Permissions app; revocation is immediate (SI-6).
5. No telemetry, no accounts, no network at build time, and no network at run time unless a `net.*` capability is declared, granted and shown.

## 7. IPC

One local message bus per session (Unix-domain sockets, peer credentials checked, capability-mode friendly). Messages are length-prefixed, schema-described records; schemas are versioned files under each service's `schema/` directory and are compiled into both ForgeScript modules and C++23 headers. Large payloads move as passed descriptors (files, shared-memory handles), never as copies. Every interface states its version; a mismatch is refused, not guessed.

## 8. System services apps may use

| Service | Purpose | Notes |
|---|---|---|
| `svc.settings` | Typed, schema-checked settings store with change signals | per-app and system namespaces |
| `svc.notify` | Notifications, progress, badges, do-not-disturb | actions route back to the app |
| `svc.portal.file` | Open, save, folder pickers; recent files; returns handles | the only way to reach user files |
| `svc.portal.print`, `.screen`, `.camera`, `.mic`, `.location`, `.usb` | Device portals with indicators | |
| `svc.intent` | Open-with, share, "new document of type", URL schemes | section 9 |
| `svc.search` | System-wide index and query; apps contribute documents | local only |
| `svc.keyring` | Secrets, keys, passkeys; unlocks with the session | apps see only their own items |
| `svc.clipboard` | History-aware clipboard with sensitivity flags | secrets expire |
| `svc.thumbnail` | Sandboxed thumbnailers per MIME type | |
| `svc.media` | Media session (now playing, transport keys), audio routing | |
| `svc.power` | Inhibit sleep, battery state, performance profile | |
| `svc.accounts` | Local store of mail, calendar, chat and sync endpoints the user added | no vendor account |
| `svc.model` | On-device models through FORGE; queueing, memory budget | `ai.local` |
| `svc.update` | Signed package and app updates, boot environments | |
| `svc.backup` | Snapshot, include and exclude rules per app | |
| `svc.audit` | Append-only log of grants, denials, network first-contacts | read by Logs and Security Centre |
| `svc.undo-trash` | Trash, file operation journal, undo across apps | |
| `svc.a11y` | Accessibility tree and actions for assistive tools | |

## 9. Intents and data exchange

- **MIME registry**: each manifest lists MIME types it can view, edit, import and export.
- **Intents**: `open`, `edit`, `view`, `share`, `compose`, `print`, `scan`, `pick`, `new`, `search`, plus URL schemes. The shell resolves an intent to an app and passes handles, not paths.
- **Drag and drop / clipboard**: typed offers with at least one open format (text, PNG, SVG, CSV, Markdown, PDF) beside the native one.
- **Open formats first**: every app's default save format is open and documented; proprietary formats are import and export only.

## 10. Data layout

| Kind | Location | Backed up | Notes |
|---|---|---|---|
| Config | `~/.config/<id>/` | yes | through `svc.settings`; human-readable |
| Data | `~/.local/share/<id>/` | yes | SQLite and files; schema version in every store |
| Cache | `~/.cache/<id>/` | no | may be deleted at any time |
| Runtime | `$XDG_RUNTIME_DIR/<id>/` | no | sockets, locks |
| Documents | wherever the user saved them | yes | reached by portal handles |

Stores are crash-safe (journal or copy-on-write), versioned, and migrate forward only with a backup of the previous version. ZFS snapshots give per-app rollback.

## 11. Quality bars for every app

- **Start**: first frame under 150 ms cold for small apps, under 400 ms for large ones (FUSE's own gate is 150 ms for `hello`, V1).
- **Idle**: zero frames and zero wake-ups when nothing changes.
- **Keyboard**: every function reachable without a pointer; all shortcuts rebindable.
- **Accessibility**: full a11y tree, names and roles on every control, 200% scaling, high contrast, reduced motion.
- **Languages**: all strings in tables; right-to-left and complex scripts (needs FULL text, gap G-03).
- **Offline**: every function that can work offline does.
- **Recovery**: autosave and session restore; no data loss on crash or power cut.
- **Tests**: ForgeScript unit tests, `.fuseui` golden renders, scripted UI tests through the a11y tree, fuzzers on every parser.
- **Proof tier**: stated per module in each spec (T1 proof of correctness, T2 proved absence of runtime errors, T3 safe subset plus sanitizers and fuzzing, T4 not covered). A tier is a target until PRISM has run and the output is recorded.

## 12. Manifest schema (draft)

```json
{
  "id": "org.pbsd.files",
  "name": "Files",
  "version": "0.1.0",
  "category": "system",
  "entry": "main.fs",
  "single_instance": true,
  "capabilities": ["fs.app", "fs.portal", "svc.notify", "svc.search", "svc.thumbnail"],
  "helpers": [{"name": "archive-helper", "language": "c++23", "capabilities": []}],
  "mime": {"view": [], "edit": [], "import": [], "export": []},
  "intents": {"handles": ["open"], "emits": ["share", "print"]},
  "url_schemes": [],
  "services_provided": [],
  "background": {"autostart": false, "run": false},
  "depends_on": ["org.pbsd.platform >= 0.1"],
  "backing_repos": [],
  "priority": "P0"
}
```

## 13. Platform gaps

Work the platform needs before the apps can be built. IDs are referenced from the app specs.

| ID | Gap | Evidence | Needed for |
|---|---|---|---|
| G-01 | Wayland backend for `fuse_app` (xdg-shell, layer-shell, fractional scale, text-input, data-device) | V5 | every app, the shell |
| G-02 | ForgeScript binding for FUSE UI with typed handles and view models | V4, V15 | every app |
| G-03 | FULL text profile vendored for offline builds; text shaping for all scripts | V6 | all text |
| G-04 | Accessibility adapter that does not need Rust at build time, or a pinned vendored AccessKit | V7 | a11y tools |
| G-05 | A run-time assertion and a HardenedBSD test that the script host never maps executable memory (the interpreter itself emits bytecode only, V16); native plugins and other `dlopen` paths closed for apps | V16 | no-JIT rule |
| G-06 | Capsicum sandbox path tested on HardenedBSD | V13 | confinement |
| G-07 | App host, bundle format, signature check, manifest-to-policy compiler | section 3, 4 | every app |
| G-08 | Session bus and schema compiler | section 7 | every app |
| G-09 | System services of section 8 | section 8 | most apps |
| G-10 | Rich text widget (styled runs, inline objects, tables) | not in V3 | Writer, Mail, Notes, Messenger |
| G-11 | Real video and audio pipeline behind `videoWidget` | V3 (stub) | Video, Camera, Music, editors |
| G-12 | 2D canvas widget with tiles, zoom, layers and pen input | not in V3 | Image Editor, Draw, Maps, PDF |
| G-13 | Terminal grid widget | not in V3 | Terminal, IDE |
| G-14 | Web content engine without JIT | none exists | Browser, Mail HTML, Help |
| G-15 | Model, GPU and serving bindings for ForgeScript | V17 | Assistant, AI features |
| G-16 | Print pipeline (PDF as the spool format) and scan pipeline | none read | Printers, all documents |
| G-17 | Spell check, hyphenation and dictionaries as a service | none read | all text apps |
| G-18 | FUSE build and test status on HardenedBSD | Unverified | everything |
