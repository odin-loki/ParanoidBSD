<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# IDE

By Odin Loch

| | |
|---|---|
| ID | `DV01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.ide` |
| Category | dev |
| Priority | P1 |
| Backing repos | FORGE (ForgeScript engine, static checker, signing), FUSE (`codeEdit`, docking, `.fuseui`), MathScript (Clang plugin for the safe subset), PRISM (proof gates), ParanoidBSD (`pbsd/analyser`, kernel tree) |
| Status | Planned. Nothing built. |

## 1. Purpose

The development environment for the distro's three languages: ForgeScript, memory-safe C++23 and FUSE UI (`.fuseui` plus CSS). App authors, system developers and PBSD kernel and driver developers use it to edit, check, build, test, debug, profile, package and sign their work. It replaces KDevelop and Kate projects from the Plasma plan (PLATFORM.md V20).

The IDE treats every project as untrusted input. Opening a folder grants nothing. Builds, language servers, tests and debug targets each run in a capability-scoped sandbox with no network by default.

## 2. Scope

In scope:

- Project model, workspace trust, editor (shared core with OF01), language intelligence for ForgeScript, C++23, `.fuseui`, CSS, CMake, JSON and Markdown.
- Build and task system (CMake and Ninja for native code, the bundle builder for apps), test explorer, sanitizers, fuzzers, coverage.
- Debuggers: ForgeScript VM debugger, native debugging through a debug adapter, kernel debugging over a remote stub.
- Safe-subset diagnostics and PRISM gate results inline.
- App-bundle packaging and signing wizard, manifest editor with capability linting, capability simulator.
- Terminal panel, version-control panel, docs panel, local AI assistance, signed ForgeScript extensions, remote development over SSH, PBSD kernel and driver workflow.

Out of scope:

| Item | Owner |
|---|---|
| Quick single-file editing without a project | OF01 Text Editor |
| Visual canvas editing of `.fuseui` (the IDE edits source and shows a live preview; "Open in Designer" hands over) | DV02 UI Designer |
| Full Git client: history graph, interactive rebase, merge tool, forge integration | DV03 Version Control |
| Binary editing | DV04 Hex Editor |
| Packet capture | DV05 Packet Analyser |
| Proof job management, obligations, counterexample exploration, reports | DV06 Proof Workbench |
| Decompilation | DV07 Reverse Engineering |
| Terminal emulation itself (the IDE embeds the terminal grid widget and session service) | SY03 Terminal |
| Key generation and storage | SE04 Passwords and Keys |
| VM lifecycle beyond the kernel test VM launched by a task | VM01 Virtual Machines |
| Help content and indexing | UT05 Help |
| Installing toolchains and SDK packages | SY05 Software |
| Notebook-style exploration | AI04 Notebook |

## 3. Users and top tasks

Personas:

- **App author**: writes ForgeScript and `.fuseui`, needs fast check-run-debug and a way to prove the app works with the capabilities it declares.
- **Native helper author**: writes safe-subset C++23 helpers and services, needs the subset diagnostics, sanitizers, fuzzers and proof tier status beside the code.
- **Kernel and driver developer**: works in `pbsd/`, boots test kernels in a VM, reads the serial console, attaches a debugger.
- **Reviewer or auditor**: opens an unknown project read-only to inspect it without running anything from it.
- **Remote developer**: edits and builds on another machine over SSH.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a folder and read code with highlighting, with nothing executed | 1 |
| T2 | Create a new app from a template and see its first window | 3 |
| T3 | Jump to the definition of the symbol under the caret | 1 (`F12`) |
| T4 | Run the static checker and fix the first error | 2 (`F8`, quick fix) |
| T5 | Build and run the project | 1 (`F5` or `Ctrl+F5`) |
| T6 | Set a breakpoint and inspect a variable | 2 (`F9` on the line, hover) |
| T7 | Run the tests of the file being edited | 1 (`Ctrl+;` then `F`) |
| T8 | See why a file fails the safe subset and open the rule | 2 |
| T9 | See the proof tier of the current function and open the failing obligation in DV06 | 2 |
| T10 | Run the app with a reduced set of capabilities and see what was denied | 3 |
| T11 | Package and sign an app bundle | 4 (wizard) |
| T12 | Rename a symbol across the project with a preview | 3 |
| T13 | Search text across the project | 2 (`Ctrl+Shift+F`, type) |
| T14 | Boot a test kernel in a VM and attach the debugger | 2 (pick the launch configuration, `F5`) |
| T15 | Ask the local model to explain a diagnostic | 2 |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Projects and workspaces

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F001 | Open folder | `Ctrl+O` then "Folder" opens the folder portal (SH04). The IDE gets a directory handle. Nothing in the folder runs or is parsed by a native tool until trust is given (4.2). | M | 1 |
| DV01-F002 | Workspace file | `<name>.pbsdws` (JSON): list of folders (by portal grant id, not path), per-folder settings, launch and task configurations. Stored where the user chooses. | M | 1 |
| DV01-F003 | Multi-root workspace | Several folders in one window, each with its own trust level, build directory and toolchain. | S | 2 |
| DV01-F004 | Project detection | Read-only detection of kind by marker files: `manifest.json` (app bundle), `CMakeLists.txt` (native), `sys/conf` or `pbsd/kernel` (kernel tree), `forge-package.json` (ForgeScript package). Detection only reads names; it parses nothing. | M | 1 |
| DV01-F005 | New project wizard | Templates: App (ForgeScript + FUSE UI), App with native helper, ForgeScript library, C++23 service, kernel module, UDA driver descriptor, IDE extension. Each creates a signed-ready layout matching PLATFORM.md section 3. | M | 1 |
| DV01-F006 | Recent workspaces | Last 30, with trust badge and "missing" flag. Reopen uses a remembered portal grant only when the user chose "always". | M | 1 |
| DV01-F007 | Project tree | Folder tree with file icons, modified and diagnostic badges, version-control status letters, filter box, "show ignored" toggle. | M | 1 |
| DV01-F008 | File operations in the tree | New file or folder, rename (`F2`), move by drag, delete to trash (`svc.undo-trash`), duplicate, copy relative path, reveal in Files (SY02). | M | 1 |
| DV01-F010 | Exclusions | Globs excluded from the tree, search and indexing; defaults: build directories, `.git`, caches. `.gitignore` honoured. | M | 1 |
| DV01-F011 | Session restore | Open tabs, splits, folds, breakpoints, watch expressions, terminal layout and panel layout restored per workspace. | M | 1 |
| DV01-F012 | Unsaved buffer persistence | Shared with OF01: unsaved text survives crash and power cut. | M | 1 |
| DV01-F013 | Project settings layers | Default, user, workspace, folder. Workspace and folder settings from an untrusted project are shown but not applied if they could run a program (4.2). | M | 1 |
| DV01-F014 | `.editorconfig` | Indent, charset, line ending, final newline, max line length, trim whitespace. Parsed in ForgeScript. | S | 1 |
| DV01-F015 | Snapshot of the workspace | "Checkpoint" takes a ZFS snapshot of the project dataset through `svc.backup` before a risky refactor; "Roll back" lists checkpoints. | S | 2 |
| DV01-F016 | Open single file | Files outside any workspace open in a "loose files" group with highlighting only. | M | 1 |

### 4.2 Workspace trust and sandboxes

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F020 | Trust levels | **Restricted** (default): view, edit, in-process highlighting, text search. **Analyse**: language servers and the static checker run in read-only sandboxes. **Build**: build, test, debug and tasks run in the build sandbox. Each level is per folder and revocable. | M | 1 |
| DV01-F021 | Trust banner | A bar under the tabs states the level and what is off, with "Allow analysis" and "Allow build" buttons. No modal on open. | M | 1 |
| DV01-F022 | Nothing auto-runs | No project-supplied task, hook, pre-build step, CMake file, formatter config or extension runs because a folder was opened. The first execution always follows an explicit user action. | M | 1 |
| DV01-F023 | Build sandbox | Every build, test, run and task executes through the system launcher (`proc.exec`, FORGE-WP-27.5) in a new capability domain with: project directory `rw` (or `r` plus a separate build directory `rw`), toolchain packages `r`, a private temporary directory, no network, no home directory, no session bus except a status channel. | M | 1 |
| DV01-F024 | Sandbox profile editor | Per configuration: extra read-only paths (picked through the portal), extra writable output folder, environment variables (explicit list, nothing inherited), CPU, memory and time limits, network policy. | M | 2 |
| DV01-F025 | Network off by default | A task that needs the network must name hosts (`net.host`) in its configuration. The user sees a shell prompt on first use. The header `[net]` badge lights while any sandbox has a socket. | M | 1 |
| DV01-F026 | Dependency fetch as a separate step | "Fetch dependencies" is its own task with network to named hosts and write access only to a vendor directory. The build itself stays offline. Fetched archives are hashed and recorded in a lock file. | S | 2 |
| DV01-F027 | Language server sandbox | Each server gets the project `r`, its index cache `rw`, no network, no exec, a memory cap (default 4 GiB) and a CPU priority below the UI. | M | 1 |
| DV01-F028 | Debug target sandbox | The debuggee runs with the capabilities of its launch configuration, not the IDE's. For an app bundle that is the manifest's capability set or a simulated subset (4.12). | M | 2 |
| DV01-F029 | Sandbox report | After each run: handles used, denials (path, capability, time), peak memory, wall and CPU time. Denials link to the line of the task or manifest that would allow them. | S | 2 |
| DV01-F030 | Trust store | Per-folder trust decisions keyed by the folder's portal grant and the BLAKE3 of its top-level marker files; a changed `CMakeLists.txt` or task file from version control drops "Build" back to "Analyse" until reviewed. | S | 2 |
| DV01-F031 | Review-before-run | When a pull or checkout changes files that define what runs (CMake files, task files, launch files, manifest helpers), the IDE shows the diff and asks before the next build. | S | 2 |
| DV01-F032 | Disposable workspace | "Open in a disposable sandbox" clones the folder to a throw-away dataset; all writes are discarded on close unless exported. For auditing unknown code. | S | 3 |
| DV01-F033 | Audit trail | Every sandbox start, grant and denial is written to `svc.audit` with the workspace id. | M | 2 |

### 4.3 Editor (shared core with OF01)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F040 | Shared editor core | The same code-editor engine as OF01 (FUSE-WP-37): piece table, multi-cursor, column mode, folding, minimap, soft wrap, encodings, huge files. The IDE adds no second text engine. All OF01 section 4.2 to 4.7 editing features apply. | M | 1 |
| DV01-F041 | Semantic highlighting | Token classes from the language server overlay the grammar colours: type, function, parameter, local, member, macro, namespace, moved-from variable (struck through). | S | 2 |
| DV01-F042 | Inline diagnostics | Squiggles by severity, gutter icons, end-of-line message (optional), hover card with the full message, rule id, source tool and quick fixes. | M | 1 |
| DV01-F043 | Completion | Popup with kind icon, type, documentation pane; fuzzy match; commit characters; snippet completions; never blocks typing (results older than the current keystroke are dropped). | M | 1 |
| DV01-F044 | Signature help | Parameter list with the active parameter in bold; overloads with `Alt+Up/Down`. | M | 1 |
| DV01-F045 | Hover | Type, declaration, documentation comment, value of constants; for ForgeScript also the capability a built-in needs. | M | 1 |
| DV01-F047 | Code lens | Above functions and tests: references count, "Run test", "Debug test", proof tier chip (4.9). | S | 2 |
| DV01-F049 | Breadcrumbs | File path and enclosing symbols above the editor; each part opens a sibling list. | S | 1 |
| DV01-F051 | Format | Format document (`Ctrl+Shift+I`), selection, on save (changed lines only, option). ForgeScript formatter from FORGE-WP-37; C++ through `clang-format` in the tool sandbox. | M | 2 |
| DV01-F053 | Snippets | Per-language snippets with tab stops and placeholders; shared format with OF01. | S | 2 |
| DV01-F054 | Bidirectional and invisible character warnings | As OF01-F063, always on for source files; a diagnostic of severity warning with a quick fix. | M | 1 |
| DV01-F055 | Diff gutter | Change bars against the last commit (from the DV03 service) with inline revert of a hunk. | S | 2 |

### 4.4 ForgeScript support

Facts in the Detail cells marked (V) are Verified in `FORGE:docs/scripting.md`; the IDE feature on top is Proposed.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F060 | Static checker on type | Runs the checker in the ForgeScript language server on every pause (300 ms). (V) `forge script --check --json` emits `[{file,line,col,severity,id,message}]`; the server links the same library (`forge/script/check.hpp`) instead of spawning the CLI. | M | 1 |
| DV01-F061 | Checker rule catalogue | (V) Rule ids: `syntax`, `use-after-move`, `unique-copy`, `iter-mutation`, `capability`, `limit-option`, `pool-task`, `pool-json`, `thread-send`, `thread-capture`, `channel-send`, `lock-order`, `unreachable`, `unused-variable`, `shadow`, `missing-return`, `arg-count`, `catch-order`, `infinite-loop`, `format`, `deprecated`, `type`. Each has a help page, severity override and on/off switch per project. | M | 1 |
| DV01-F064 | Capability diagnostics against the manifest | The checker's `capability` rule is fed the app's manifest: a call to a `net` built-in in an app with `net.none` is an error at the call site with a fix "Add capability to manifest" or "Remove call". Proposed; needs FORGE-WP-15. | M | 2 |
| DV01-F065 | Module resolution | Imports resolve with the engine's rules ((V) `--module-dir`), restricted to the bundle and package store as in the app profile (FORGE-WP-14.3). Unresolved imports are errors with the searched locations listed. | M | 1 |
| DV01-F066 | Type of expression | Hover and `Ctrl+Shift+T` use (V) `engine::type_of(expr)` / REPL `:type` in a paused or REPL context; static contexts use the checker's inference where FORGE-WP-15 provides it. | S | 2 |
| DV01-F067 | Bytecode view | "Show Bytecode" opens (V) `engine::disassemble(src)` / `:dis` output beside the source, with source-line to instruction mapping and the verifier's stack depth. Read-only. | S | 2 |
| DV01-F068 | REPL panel | A dock panel hosting an engine in a sandbox: (V) multi-line continuation, value echo, `:type`, `:dis`, `:mem`, `:gc`, `:reset`, `:vm on|off`. The IDE adds what the CLI lacks ((V) "no line editing, history or tab completion"): history per workspace, completion from the language server, syntax colours. | M | 2 |
| DV01-F069 | REPL scope | "Load file into REPL" and "Send selection to REPL" (`Ctrl+Enter`). The REPL runs with a capability set chosen in its toolbar (default none); (V) the REPL does not run static checks, so the panel runs the checker on each input itself and shows warnings. | S | 2 |
| DV01-F070 | REPL under signing policy | (V) With `integrity.scripts = required` the REPL and `-e` are refused. The panel therefore always runs under a development policy that is separate from the installed-app policy, and says so in its header. | M | 2 |
| DV01-F071 | Run configurations | Per script or app: arguments, engine options ((V) `--timeout`, `--max-steps`, `--heap`, `--tree`), capability switches, policy file, working directory handle. | M | 1 |
| DV01-F072 | Debug detectors | Toggle "Race, lock-order and stall detectors" maps to (V) `--debug`: `RaceError`, `DeadlockError` (lock-order cycle; stall after `deadlock_ms`, default 1000). Each error opens the two stacks involved side by side. | M | 2 |
| DV01-F074 | Error navigation | Uncaught errors show class, message, `line` and `trace` ((V) fields of `Error`); each frame is a link. The error class hierarchy ((V) `OSError`, `SecurityError`, ...) is browsable in docs. | M | 1 |
| DV01-F077 | Test discovery and run | ForgeScript tests through the framework of FORGE-WP-35 (Proposed; today only (V) `assert` and golden `.out` files exist). Golden tests: run, diff against `<name>.out`, "Accept new output" with review. | M | 2 |
| DV01-F078 | Signing | "Sign file" and "Sign bundle" call the signing tool ((V) `forge sign job.fs --key k.key`, Ed25519 over BLAKE3, detached `.sig`, bound to the file name). The key never enters the IDE: signing is done by SE04's key service; the IDE passes the digest and file name. | M | 2 |
| DV01-F079 | Signature status | Gutter and tree badges: signed and valid, signed and stale (bytes changed since signing), unsigned. (V) Renaming a signed file breaks its signature; rename in the tree offers "Re-sign". | S | 2 |
| DV01-F080 | Emit to shell | "Emit As" lists (V) `--emit-targets`; the result opens read-only in a split. Constructs outside the (V) portable subset are underlined in the source with the emitter's message before emitting. | S | 3 |
| DV01-F083 | No-native-code guarantee view | A diagnostics page states the engine build facts: bytecode only, no machine-code JIT ((V) `scripting.md` "There is no JIT to machine code"; verified again in `FORGE:script/src/runtime.cpp:729-730`, `script/src/vm.cpp:220-229`), plugins compiled out (FORGE-WP-02.3). | S | 2 |
| DV01-F084 | Plugin import warning | (V) Native plugin `import "x.so"` is flagged as an error in app projects: plugins are not available in the app profile. | M | 1 |
| DV01-F085 | Host binding stubs | Bindings of PLATFORM.md section 5 (`ui::`, services) are known to the server from schema files, so completion and arity checks work without the host running. | M | 2 |

### 4.5 C++23 (safe subset) support

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F090 | Language server | A clangd-style server (clangd from LLVM, Apache-2.0 with LLVM exception, shipped as a system package) runs in the analysis sandbox, driven by `compile_commands.json`. | M | 1 |
| DV01-F091 | Compilation database | Produced by the CMake configure step into the build directory; the IDE never guesses flags. Without one, C++ files get grammar highlighting only and a banner "Configure to enable analysis". | M | 1 |
| DV01-F092 | Safe-subset diagnostics inline | The MathScript Clang plugin (Verified to exist: `MATHSCRIPT:README.md` "A Clang plugin can enforce twenty compile-time rules (no raw `new`, no `throw`, no C-style casts, ...)") is loaded by the build compiler; its diagnostics are parsed from the compiler's structured output and shown inline with rule id, rule text and a link to the rule's help page. Loading it into the language server for on-type results is Proposed. | M | 1 |
| DV01-F093 | PBSD analyser diagnostics | For kernel code the PBSD plugin `pbsd-ownership` (Verified: `docs/specs/ANALYSER.md`, `pbsd/analyser/ownership_check.cpp`, `isr_context_check.cpp`) is shown the same way: use-after-move of `UniqueHandle`, double release, rights widening, sleep in ISR context. | M | 2 |
| DV01-F094 | Subset profile per target | Each target declares its profile: `app-helper`, `service`, `kernel`. The profile picks the plugin set and flags. A file outside any profile shows "not gated". | M | 2 |
| DV01-F097 | CMake integration | Configure, build, clean, rebuild; presets (`CMakePresets.json`); target list; cache variable editor; Ninja as the generator. CMake runs only at "Build" trust. | M | 1 |
| DV01-F099 | Build output parsing | Compiler, linker and Ninja messages become diagnostics with file and line; the raw log stays available. | M | 1 |
| DV01-F100 | Sanitizer runs | Build variants ASan, UBSan, TSan, MSan as one-click configurations; reports are parsed into a "Sanitizer" diagnostics group with stack frames as links. Note: HardenedBSD's hardening (ASLR, W^X) can conflict with some sanitizers; each variant states what it relaxes inside its own sandbox. | M | 2 |
| DV01-F101 | Fuzzers | libFuzzer targets discovered by naming convention or CMake property; run with time and memory limits in a sandbox with a private corpus directory; crashes are minimised, saved and listed; "Debug this input" starts the debugger on it. | M | 2 |
| DV01-F103 | Coverage | Source-based coverage (`llvm-cov`): per-line and per-branch gutter colours, per-file and per-target table, diff coverage against the base branch, export as lcov and HTML-free Markdown summary. | M | 2 |
| DV01-F106 | Toolchain selection | Toolchains are read-only packages installed by SY05; the IDE lists them with version and hash; a project pins one. No toolchain is downloaded by the IDE. | M | 1 |
| DV01-F108 | Helper SDK awareness | Knows the helper skeleton of FORGE-WP-28.1 (`helper_main(channel&, grants&)`): new-helper template, schema-to-header regeneration task, check that the helper requests no capability its app lacks. | S | 2 |

### 4.6 FUSE UI and CSS support

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F110 | `.fuseui` language support | JSON grammar plus schema: completion of widget classes (PLATFORM.md V3), properties, signal names under `"on"`, ids; validation of `{expr}` and `{=two-way}` bindings with the same expression rules as `FUSE:Source/FUSE/UI/Doc/include/fuse/ui/doc/document.hpp` (numbers, dotted names, arithmetic, comparisons, `&& || !`). | M | 1 |
| DV01-F111 | Binding checks against the view model | A binding to a name the view-model module does not define is a warning; a two-way binding to a read-only name is an error. | M | 2 |
| DV01-F112 | Action checks | Every `"on": {"clicked": "save"}` action name must match a `ui::action` or handler in the app's ForgeScript; go-to-definition works from the document to the script. | M | 2 |
| DV01-F113 | CSS support | Completion and validation limited to the FUSE CSS subset (selectors, pseudo-classes and properties listed in `FUSE:docs/ui/README.md` "Styling"); unknown properties are warnings; colour swatches; token (`var(--name)`) completion from DESIGN_SYSTEM.md. | M | 1 |
| DV01-F114 | Hard-coded value lint | A colour, size or radius literal where a design token exists gets a hint "Use `var(--accent)`". Shares rules with DV02 design linting. | S | 2 |
| DV01-F115 | Live preview panel | The document is rendered by a preview helper process using FUSE UI itself; the preview updates on each valid edit using hot reload (Verified: `Document::reload` keeps nodes with an unchanged id and class). On a JSON error the last good render stays with an error strip. | M | 1 |
| DV01-F116 | Preview controls | Theme (dark, light, high contrast), density, scale 100 to 300%, width presets for the three breakpoints, locale and right-to-left switch, mock view-model values from a JSON file. | S | 2 |
| DV01-F117 | Select in preview | Click a widget in the preview to move the caret to its node; caret movement outlines the node in the preview. | S | 2 |
| DV01-F118 | Open in Designer | Hands the document handle to DV02 by `edit` intent; DV02's saves reload in the IDE (round-trip guarantee is DV02's). | M | 2 |
| DV01-F120 | Locale table support | `locale/<lang>.tbl` key=value: missing keys per language, unused keys, keys used in `.fuseui` and scripts but not defined; "Extract string to table" refactor. | S | 2 |

### 4.7 Navigation, search and refactoring

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F130 | Go to definition, declaration, type, implementation | `F12`, `Ctrl+F12`, `Ctrl+Shift+F12`, `Ctrl+Alt+F12`; works across ForgeScript, C++, `.fuseui` action names and CSS classes. | M | 1 |
| DV01-F131 | Find references | `Shift+F12`; results grouped by file with read, write and call kinds. | M | 1 |
| DV01-F132 | Go to file | `Ctrl+P`: fuzzy file name match over the workspace. (Print is on `Ctrl+Shift+P` in this app's file menu; the command palette stays on `Ctrl+K`.) | M | 1 |
| DV01-F133 | Go to symbol | In file `Ctrl+Shift+O`; in workspace `Ctrl+T`. | M | 1 |
| DV01-F134 | Outline | Sidebar tree of the current file's symbols with kind icons, sort by position or name, filter. | M | 1 |
| DV01-F135 | Call hierarchy | Incoming and outgoing calls as a lazy tree (`Ctrl+Alt+Shift+H` on a function). | S | 2 |
| DV01-F137 | Navigation history | `Alt+Left/Right`; "Last edit location" `Ctrl+Shift+Backspace`. | M | 1 |
| DV01-F139 | Search in workspace | `Ctrl+Shift+F`: text or regex, case, whole word, include and exclude globs, scope (workspace, folder, open files, changed files). Runs in the search helper over the granted handles only. Streaming results. | M | 1 |
| DV01-F140 | Replace in workspace | Preview as diff rows with check boxes; atomic per file; one cross-file undo. | M | 2 |
| DV01-F142 | Rename symbol | `F2` on a symbol: preview of all edits across files and languages (script, `.fuseui` ids and action names, locale keys); applied as one undo step. | M | 2 |
| DV01-F144 | Code actions | `Ctrl+.` lists quick fixes and refactors at the caret: add include or import, add missing `return`, replace a moved-from use, wrap in `forge::shared`, add capability to manifest. | M | 2 |
| DV01-F147 | Symbol index | Persistent per-workspace index in the cache directory; rebuilt in the background at low priority; progress in the status bar. | M | 2 |

### 4.8 Build, tasks and tests

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F150 | Task model | A task is declarative: program (from a toolchain package or the bundle tools), arguments, working directory handle, environment list, sandbox profile, problem matcher, dependencies. No shell string is evaluated. Stored in the workspace file. | M | 1 |
| DV01-F151 | Built-in tasks | Check, Configure, Build, Clean, Test, Run, Package, Sign, Format, Fetch dependencies, Render goldens, Run PRISM gate. | M | 1 |
| DV01-F152 | Pipelines | A task may be a ForgeScript pipeline (Verified facility: `forge::pipeline` with jobs, inputs, outputs, retries, parallel waves). The IDE shows the DAG, per-job status and skipped jobs. | S | 2 |
| DV01-F153 | Build bar | Configuration, target and launch selectors in the toolbar; `Ctrl+B` build, `Ctrl+Shift+B` pick task, `Ctrl+Break` cancel. | M | 1 |
| DV01-F154 | Build queue | Jobs run one at a time per workspace by default; progress in the status bar and `svc.notify`; a finished build raises a notification only when the window is not focused. | M | 1 |
| DV01-F155 | Output panel | One channel per task; ANSI colours; links for `file:line:col`; search; "Copy as Markdown"; kept for the last 20 runs. | M | 1 |
| DV01-F157 | Test explorer | Tree of suites and cases from all frameworks: ForgeScript tests, ForgeScript golden files, CTest, `.fuseui` golden renders, scripted UI tests, fuzz regression inputs. Filter by status, tag, text. | M | 2 |
| DV01-F158 | Run and debug tests | Run all, failed, changed since last commit, at caret, in file; debug one test; repeat until failure (N times). | M | 2 |
| DV01-F159 | Test results | Pass, fail, skipped, duration, captured output, expected and actual diff, history sparkline of the last 20 runs, flaky flag. | M | 2 |
| DV01-F161 | Coverage from tests | "Run with coverage" fills the coverage view (F103) for C++ and the ForgeScript coverage of FORGE-WP-35. | S | 2 |
| DV01-F162 | Problem matchers | Declarative regex matchers for tool output; shipped for clang, Ninja, CMake, the ForgeScript checker, PRISM, sanitizers. | M | 1 |
| DV01-F163 | Launch configurations | Named run and debug set-ups: target, arguments, environment list, sandbox profile, capability set, pre-launch task, debugger options. | M | 1 |

### 4.9 Proof and gate status (hand-off to DV06)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F170 | Gate results inline | PRISM findings are read from its report files (Verified outputs: `report.json`, `report.sarif` SARIF 2.1.0, `stages.jsonl`, `taxonomy.json`, per `PRISM:README.md`) and shown as diagnostics with source "PRISM", stage name and severity. | M | 2 |
| DV01-F171 | Tier chips | Each module and function with a declared target tier (T1 to T4, PLATFORM.md section 11) shows a chip: target, last recorded result, age. A chip never says "proved" unless a recorded PRISM output for the current file hash exists. | M | 2 |
| DV01-F172 | Stale results | When the file hash differs from the hash in the report, findings are greyed and labelled stale. | M | 2 |
| DV01-F173 | Run gate | "Run PRISM gate" on file, target or project starts a job through DV06's job interface; progress streams from `stages.jsonl`. | M | 2 |
| DV01-F174 | Open in Proof Workbench | Any finding, obligation or counterexample opens in DV06 at that item (`open` intent with a `prism-item` reference). The IDE shows the counterexample's input values as a read-only card and offers "Debug with these inputs" when DV06 supplies a runnable case. | M | 2 |
| DV01-F176 | Merge gate summary | Before commit: one row each for checker, subset plugin, tests, sanitizers, PRISM, signature; green only from recorded results. | S | 2 |

### 4.10 Debugging

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F180 | Debug adapter host | All debuggers speak the Debug Adapter Protocol to the IDE; each adapter is a separate sandboxed process. | M | 2 |
| DV01-F181 | ForgeScript VM debugger | Uses the engine debug interface of FORGE-WP-36 (Proposed; none exists today). Breakpoints by line map to bytecode offsets through the line table; stepping in, over, out; pause through `engine::interrupt()` (Verified to exist). | M | 2 |
| DV01-F182 | ForgeScript data views | Locals, globals, closures, containers (paged), structs, refcount and owner thread of heap objects, moved-from and frozen flags, borrow state of containers under iteration. | M | 2 |
| DV01-F183 | ForgeScript tasks view | Threads and tasks (`std::async`, `forge::spawn`), channels with queue depth, `forge::shared` lock holders and waiters. | S | 2 |
| DV01-F184 | Exception breakpoints | Break on throw or on uncaught, filtered by error class (for example only `SecurityError` and subclasses). | M | 2 |
| DV01-F185 | Capability breakpoints | Break when a built-in is denied (`PermissionError`, `SandboxViolationError`, `PolicyViolation`) or when a named capability is first used. | S | 2 |
| DV01-F186 | Native debugging | LLDB (Apache-2.0 with LLVM exception, system package) through its debug adapter. Launch only by default; attach needs the system debug broker (section 13, DV01-GAP-2) because HardenedBSD restricts process tracing. | M | 2 |
| DV01-F188 | Breakpoints | Line, conditional, hit count, log point, function, data (watchpoint, native only), one-shot; a breakpoints panel with enable, disable and groups. | M | 2 |
| DV01-F189 | Stepping | `F10` over, `F11` into, `Shift+F11` out, `Ctrl+F10` run to caret, "Set next statement" where the adapter allows. | M | 2 |
| DV01-F190 | Watches and evaluate | Watch panel, hover evaluation, debug console. Evaluation in ForgeScript uses the tree-walker in the paused frame with side effects off by default. | M | 2 |
| DV01-F191 | Call stack | Frames per thread, async task ancestry for ForgeScript, frame filters to hide engine internals. | M | 2 |

### 4.11 Profiling and memory tools

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F200 | ForgeScript profiler | Sampling and per-function counts from FORGE-WP-36; flame chart, top functions, call tree; per-line heat in the gutter. | S | 3 |
| DV01-F201 | ForgeScript heap view | Live objects by type, bytes, collections run (Verified counters: `gc.stats()`, `mem.stats()`, REPL `:mem`), cycles found by the collector, growth between two snapshots. | S | 3 |
| DV01-F202 | Native CPU profiler | Sampling through `pmcstat`/hwpmc where the debug broker allows; flame chart; source annotation. | S | 3 |

### 4.12 Packaging, signing and capability tools

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F210 | Manifest editor | Form view and source view of `manifest.json` (PLATFORM.md section 12), kept in sync: identity, entry, capabilities, helpers, MIME, intents, URL schemes, background, dependencies. | M | 1 |
| DV01-F211 | Capability linting | Errors: unknown capability, helper with a capability its app lacks, `bg.autostart` without `bg.run`. Warnings: capability declared but never used in code, capability used but not declared, `fs.path` where `fs.portal` would do, `net.user` where named hosts would do, `proc.exec` without a stated reason. | M | 1 |
| DV01-F213 | Capability simulator | "Run with these grants": a matrix of the manifest's capabilities with on, off, prompt-and-deny, prompt-and-allow, revoke-after-N-seconds. The app runs in a sandbox with exactly that set. | M | 2 |
| DV01-F214 | Denial log | During a simulated run every denied call is listed with script line, capability and resource; "did the app cope?" is judged by whether an error reached the top level. | M | 2 |
| DV01-F217 | Bundle builder | Assembles `<id>.app/` (PLATFORM.md section 3), validates layout, cooks `.fuseui`, compiles locale tables, builds helpers, writes `SIGNATURES` placeholders. | M | 2 |
| DV01-F218 | Packaging and signing wizard | Steps: validate, choose signing key (from SE04, never exported), review the capability summary, sign (`SIGNATURES`: BLAKE3 tree hash plus Ed25519), verify, export. The wizard refuses to sign with errors in the manifest lint. | M | 2 |
| DV01-F219 | Verify bundle | Checks every file against `SIGNATURES` and shows the first mismatch; uses the CLI of FORGE-WP-09.3. | M | 2 |
| DV01-F220 | SBOM and provenance | Writes a CycloneDX JSON SBOM (toolchain hash, dependencies, licences) and a provenance record (source commit, builder sandbox profile) beside the bundle for SY05 to display. | S | 3 |
| DV01-F221 | Install to a test profile | Installs the bundle for the current user in a "development" slot that SY05 marks as unsigned-by-distro; removable in one click. | S | 2 |

### 4.13 Panels: terminal, version control, docs, diagnostics

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F230 | Terminal panel | Embeds the terminal grid widget (G-13, FUSE-WP-39) and SY03's session service: tabs, splits, profiles. Shells start in the build sandbox of the workspace, not with the IDE's rights. | M | 2 |
| DV01-F232 | Version-control panel | Changed files, stage and unstage file or hunk, commit message, commit, branch switch, pull and push buttons. All operations go through DV03's repository service; the IDE holds no Git code. | M | 2 |
| DV01-F235 | Diagnostics panel | All diagnostics from all sources in one table: severity, message, source tool, rule id, file, line. Filter by source, severity, folder; group by file or rule; `F8`/`Shift+F8` next and previous. | M | 1 |
| DV01-F236 | Docs panel | Shows help for the symbol under the caret from UT05's index (`svc.search` scoped to docs): ForgeScript built-ins, FUSE widgets, platform services, C++ library reference if installed, man pages. `F1` on a symbol. | M | 2 |

### 4.14 Local AI assistance

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F240 | Opt-in | Off by default. Needs `ai.local` granted and a model installed through AI02. Runs through `svc.model` only; no network path exists in the feature. | M | 3 |
| DV01-F241 | Explain | Explain selection, explain diagnostic, explain a denial from the simulator. Output appears in a side panel as Markdown. | S | 3 |
| DV01-F242 | Suggest edit | Proposed changes are always shown as a diff the user applies or discards; nothing is written without that step. | S | 3 |
| DV01-F245 | Context control | The panel lists exactly which files and ranges are in the prompt; the user can remove any. Untrusted workspaces contribute text as data only; the model has no tools here. | M | 3 |
| DV01-F246 | Checked suggestions | Every suggested ForgeScript edit is run through the static checker before display; suggestions with errors are marked. | S | 3 |
| DV01-F247 | No retention | Prompts and replies are kept only in the session unless the user saves the conversation into the workspace. | M | 3 |

### 4.15 Extensions

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F250 | Extension format | A signed ForgeScript bundle with its own manifest: contributed commands, grammars, snippets, problem matchers, task types, tree views, `.fuseui` panels. No native code, no helpers. | S | 3 |
| DV01-F251 | Extension sandbox | Each extension runs in its own engine with an FSS policy from its manifest; default is no file, network or process access. It reaches documents only through an editor interface (read text, propose edits). | S | 3 |
| DV01-F252 | Extension permissions | Listed at install; a subset of the IDE's own capabilities at most (rule 2 of PLATFORM.md section 6). | S | 3 |
| DV01-F253 | Extension manager | Installed list, enable or disable per workspace, signature and signer shown, update through SY05. No extension marketplace service inside the IDE. | S | 3 |

### 4.16 Remote development

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F260 | SSH connection | Connect to a host the user types (`net.user`); host key checked against SE04's known hosts; keys and agent from `svc.keyring`; first contact logged. | S | 3 |
| DV01-F261 | Remote file system | The project tree, search and editing act on remote files through SFTP. Local cache of opened files in the cache directory, encrypted at rest with a workspace key. | S | 3 |
| DV01-F262 | Remote tasks | Build, test and run tasks execute on the remote host through the SSH session; output streams back; problem matchers map remote paths. | S | 3 |

### 4.17 PBSD kernel and driver workflow

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F270 | Kernel project kind | Detects the kernel tree; indexes with the kernel compile database; applies the `kernel` subset profile (F094). | S | 2 |
| DV01-F271 | Kernel build tasks | Configure and build kernel and modules for amd64, arm64, riscv64 in the build sandbox, out of tree. | S | 2 |
| DV01-F272 | Boot under an emulator | Launch type "Kernel in VM": starts QEMU (system package, GPL-2.0, run as a separate sandboxed program) or bhyve through VM01's service with the built kernel and a disposable disk image. | S | 2 |
| DV01-F273 | Serial console panel | The VM's serial port appears as a terminal tab; log saved per boot; panic text is recognised and its backtrace addresses are symbolised into links. | S | 2 |
| DV01-F274 | Kernel debugger | Attaches LLDB over the gdb-remote protocol to the emulator's debug stub; `kgdb` (GPL) is used instead only if the user installed it. Breakpoints, stepping, per-CPU threads, kernel module symbol loading. | S | 3 |
| DV01-F276 | UDA descriptor support | Schema, completion and validation for driver descriptors (`docs/specs/UDA_DESCRIPTOR.md`); hand-off to VM05 Devices to view a loaded descriptor. | S | 3 |

### 4.18 Workbench, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV01-F290 | Dock layout | Workbench pattern (DESIGN_SYSTEM.md section 4): docks left, right, bottom; tear-off windows; named layouts ("Edit", "Debug", "Review"); layout saved as dock text (Verified facility: `dockSaveLayout`). | M | 1 |
| DV01-F292 | Command palette | `Ctrl+K`: actions, files (`>` prefix off), symbols (`@`), lines (`:`), tasks (`task `), settings. | M | 1 |
| DV01-F293 | Keymap | All shortcuts rebindable; shipped presets: default, and two common editor-style presets named by their style ("modal", "chorded"). | M | 1 |
| DV01-F296 | Status bar | Trust level, branch, diagnostics counts, caret, language, encoding, line ending, indent, toolchain, build configuration, background jobs, sandbox count. | M | 1 |
| DV01-F297 | Background job list | Indexing, builds, fuzzers, gates: each with progress, cancel and resource use. | M | 1 |
| DV01-F298 | Crash recovery | A crashed language server, adapter or helper restarts with back-off (1, 5, 30 s) and a toast; the editor keeps working. | M | 1 |
| DV01-F299 | Screen-reader mode | Diagnostics, completions and debugger stops are announced through live regions; all trees and tables are in the a11y tree; "read line diagnostics" command. | M | 2 |

## 5. GUI

### 5.1 Window map

```
IDE
├─ Workbench window (one per workspace; more by tear-off)
│  ├─ Header bar: menu, workspace name, command palette, [net] badge, window buttons
│  ├─ Build bar: configuration, target, launch selectors, run/debug/stop
│  ├─ Trust banner (when below "Build")
│  ├─ Left dock: Project · Outline · Search · Tests · Version Control · Extensions
│  ├─ Centre: editor groups (tabs, splits), FUSE UI preview, manifest editor, bytecode view
│  ├─ Right dock: Inspector (docs, hover detail, proof tier) · Assistant · Debug variables
│  └─ Bottom dock: Diagnostics · Output · Terminal · REPL · Debug console · Test results · Serial console
├─ Dialogs
│  ├─ New project wizard
│  ├─ Trust and sandbox profile
│  ├─ Launch configuration editor
│  ├─ Packaging and signing wizard
│  ├─ Capability simulator
│  ├─ Rename preview / Replace preview
│  ├─ Toolchain picker
│  ├─ Remote connection
│  └─ Preferences (Editor, Languages, Build, Debug, AI, Extensions, Keymap)
├─ Popovers: completion, signature help, hover card, peek, code actions, breadcrumb lists, sandbox report
└─ Sheets (narrow windows): inspector, diagnostics
```

### 5.2 Main window

Shown at 1440×900, layout "Edit", trust "Build".

```
+------------------------------------------------------------------------------------------+
| [=] IDE — notes-app                 [ Search or run a command   Ctrl+K ]   [net] [▁][▢][✕] | (1)
+------------------------------------------------------------------------------------------+
| [Debug v] [notes-app v] [Run app v]  [▶ Run] [🐞 Debug] [■]  [Build] [Test]   Trust: Build | (2)
+----------------------+--------------------------------------------+----------------------+
| PROJECT        [⋯]   | main.fs ×  editor.fuseui ×  manifest.json   | INSPECTOR      [F4]  |
| [filter________]     +--------------------------------------------+ fs.write(path, text) |
| v notes-app          | src › main.fs › save_note()            (4) | Needs: fs capability |
|   v src              |  41  auto save_note(Note n) {              | Declared: fs.portal  |
|   > main.fs     M  2 |  42    auto p = ui::get(pathBox, "text");  | ------------------   |
|     model.fs         |  43    fs.write(p, n.body);                | PROOF TIER           |
|   v ui               |        ~~~~~~~~ capability: path API is    | target T3            |
|     editor.fuseui    |        not available in the app profile    | last run: none       |
|   v style            |  44    ui::toast("Saved");                 | [Open in Proof       |
|     app.css          |  45  }                                 (5) |  Workbench]          |
|   v locale           |                                            | ------------------   |
|     en.tbl           |                                       (6)▐ | OUTLINE              |
|   manifest.json      +--------------------------------------------+  Note                |
| (3)                  | DIAGNOSTICS  OUTPUT  TERMINAL  REPL  TESTS |  load_notes()        |
|                      | ✕ capability  main.fs:43  path API ...  (8)| > save_note()    (7) |
|                      | ⚠ unused-variable  model.fs:12  'tmp'      |  main()              |
+----------------------+--------------------------------------------+----------------------+
| Build ✓ 2.1 s · ✕1 ⚠1 · main ⎇ · Ln 43, Col 5 · ForgeScript · UTF-8 · LF · 1 sandbox  (9) |
+------------------------------------------------------------------------------------------+
```

1. Header bar (DESIGN_SYSTEM.md section 3). `[net]` lights only while a sandbox or the remote session holds a socket; click lists endpoints per sandbox.
2. Build bar: configuration, target and launch `comboBox`es, run, debug, stop `toolButton`s, trust chip (click opens the trust dialog).
3. Left dock (`dockWidget`, 260 px): project `treeView` with status letters and diagnostic counts; other tabs: Outline, Search, Tests, Version Control, Extensions.
4. Breadcrumb row (missing widget, DV01-GAP-4).
5. Editor: shared code editor (FUSE-WP-37) with squiggles, end-of-line messages, gutter (breakpoints, change bars, coverage).
6. Minimap with diagnostic and search marks.
7. Right dock (280 px): Inspector `panel` showing docs for the symbol, capability need versus manifest, proof tier chip; Outline `treeView`.
8. Bottom dock (220 px): `tabWidget` of Diagnostics (`tableView`), Output, Terminal, REPL, Tests.
9. Status bar: every chip is a button.

States: empty (no workspace: recent list and "Open Folder", "New Project"); loading (index progress in status bar, editor usable); error (toolchain missing banner with "Open Software"); offline (always; no state change); locked (session locked: debuggee paused if option set); read-only (Restricted trust plus read-only handle: lock in tab).

### 5.3 Other views and dialogs

#### Debug layout (1440×900)

```
+----------------------+--------------------------------------------+----------------------+
| CALL STACK           | main.fs ×                                  | VARIABLES            |
| v Main thread        |  41  auto save_note(Note n) {              | v Locals             |
| > save_note  main:43 | ●42    auto p = ui::get(pathBox, "text");  |   n   Note {…}       |
|   on_click   main:88 | ➤43    fs.write(p, n.body);            (1) |   p   "/tmp/a.md"    |
|   <host>             |  44    ui::toast("Saved");                 | v Globals            |
| v Task #3 (async)    |                                            |   notes vector[12]   |
|   index_notes model:7| [▶ Continue F5][⤼ F10][⤓ F11][⤒ S+F11][■]  | WATCH            (3) |
| BREAKPOINTS      (2) |                                        (4) |   n.body.size()  128 |
| [x] main.fs:42       +--------------------------------------------+ TASKS AND LOCKS      |
| [x] on PermissionErr | DEBUG CONSOLE                              |  #3 running          |
| [ ] model.fs:7 if i>3| > p.size()                                 |  ch(8) depth 2       |
|                      | 9                                      (5) |  shared<Index> free  |
+----------------------+--------------------------------------------+----------------------+
```

1. Current line marker and breakpoint dots in the gutter.
2. Breakpoints `listView` with check boxes; includes exception and capability breakpoints.
3. Variables and Watch as `treeView` with value column; changed values flash once (80 ms, off with reduced motion).
4. Floating debug toolbar, also in the build bar.
5. Debug console: evaluates in the paused frame; side effects need "Allow side effects" ticked.

States: no session (panels show "Start debugging with F5"); running (variables greyed); stopped; adapter crashed (banner with restart).

#### Trust and sandbox profile dialog (640×520)

```
+--------------------------------------------------------------+
| How far do you trust "notes-app"?                             |
|  ( ) Restricted   View and edit only. Nothing runs.           |
|  ( ) Analyse      Language servers read the project.          |
|  (•) Build        Build, test and debug in a sandbox.         |
| ------------------------------------------------------------ |
| Build sandbox                                                 |
|  Project folder      [read and write v]                       |
|  Build folder        [~/.cache/org.pbsd.ide/b/7f3a   ] [Pick] |
|  Extra read-only     [ + Add folder… ]                        |
|  Network             (•) None  ( ) Named hosts [__________]   |
|  Limits              CPU [8] cores  RAM [8 GiB]  Time [30 min]|
|  Environment         [ LANG=C.UTF-8                    ] [+]  |
| ------------------------------------------------------------ |
| Files that decide what runs: CMakeLists.txt, tasks (3)  [Review]|
|                                   [Cancel]  [Set trust: Build]|
+--------------------------------------------------------------+
```

`radioButton` group, `comboBox`, `lineEdit`, `spinBox`, `listView` for paths. "Review" opens a read-only diff of run-defining files since the last trusted hash. States: first open, changed-since-trusted (warn strip).

#### Capability simulator (900×600)

```
+------------------------------------------------------------------------------+
| Run "notes-app" with these grants                                             |
+--------------------------+----------+-----------------------------------------+
| Capability               | Mode     | Notes                                   |
| fs.app                   | Allow    | always                                  |
| fs.portal                | [Prompt: deny v]                                   |
| svc.notify               | [Allow v]                                          |
| svc.search               | [Deny v]                                           |
| net.host:sync.example:443| [Revoke after 10 s v]                              |
| ai.local                 | [Deny v]                                           |
+--------------------------+----------+-----------------------------------------+
| Presets: [All granted] [Nothing granted] [Install defaults] [Save preset…]    |
+------------------------------------------------------------------------------+
| DENIALS (live)                                                         (1)    |
| 12:01:03 fs.portal  open dialog denied     main.fs:61  handled               |
| 12:01:09 svc.search query denied           search.fs:14  UNCAUGHT  ✕         |
+------------------------------------------------------------------------------+
|                                          [Close]   [Run]   [Debug]           |
+------------------------------------------------------------------------------+
```

1. Live `tableView`; "UNCAUGHT" rows are failures. Double click goes to the line. States: idle, running, finished (summary: n denials, m uncaught).

#### Packaging and signing wizard (720×540), step 3 of 5

```
+----------------------------------------------------------------------+
| Package notes-app   1 Validate ✓  2 Key ✓  ▶3 Review  4 Sign  5 Export|
+----------------------------------------------------------------------+
| What users will be told                                               |
|  • Open and save files you pick                (fs.portal)            |
|  • Show notifications                          (svc.notify)           |
|  • Connect to sync.example on port 443         (net.host)   [net]     |
| Helpers: none        Background: no         Autostart: no             |
| Lint: 0 errors, 1 warning — ai.local declared but never used [Show]   |
| Files: 41 · 1.8 MiB · tree hash b3:9f2c…e1                            |
| Signing key: "Odin release 2026" (Ed25519, held by Passwords and Keys)|
+----------------------------------------------------------------------+
|                                  [Back]  [Cancel]  [Sign bundle]      |
+----------------------------------------------------------------------+
```

Wizard pattern. Signing prompts through SE04; the IDE sees only the signature. States: lint errors (Sign disabled with reason), key missing (link to SE04), verify failed (shows first mismatching file).

#### FUSE UI preview beside source

```
+-------------------------------------------+----------------------------------+
| editor.fuseui ×                           | PREVIEW  [dark v][compact v][100%]|
|  { "class": "panel", "id": "root",        | [640][1100][1440] [en v] [RTL ]  |
|    "children": [                          | +------------------------------+ |
|     { "class": "lineedit", "id": "title", | | [ Title_________________ ]   | |
|       "text": "{=note.title}" },          | | +--------------------------+ | |
|     { "class": "button", "text": "Save",  | | | body…                    | | |
|       "on": { "clicked": "save" } }       | | +--------------------------+ | |
|    ] }                                    | |                    [Save]    | |
|                                           | +------------------------------+ |
| ⚠ action "save" has no handler in main.fs | Mock model: note.json [Edit]     |
+-------------------------------------------+----------------------------------+
| [Open in Designer]                                  Last good render 12:04:10 |
+------------------------------------------------------------------------------+
```

Preview is a `viewportWidget` showing frames from the preview helper. States: rendering, error (last good render dimmed plus message), helper crashed (restart).

#### Other views without a wireframe

- **Test explorer**: left-dock `treeView` of suites and cases with status icons and counts; a detail pane with the failure message, expected and actual diff (FUSE-WP-38) or image compare for goldens, history and "Accept…".
- **Manifest editor**: section `listView` on the left (Identity, Capabilities, Helpers, MIME and intents, Background, Dependencies); a form of `checkBox`, `radioButton` and `lineEdit` rows on the right; each capability row shows where it is used and the lint result; a Form and Source switch.
- **Serial console**: a terminal grid tab (G-13) with Stop, Reboot, Attach debugger and Save log; panic backtrace frames are symbolised into links.

### 5.4 Menus

Shortcuts in brackets. `▸` opens a submenu.

| Menu | Items |
|---|---|
| File | New File (`Ctrl+N`) · New Project… (`Ctrl+Shift+N`) · Open File… (`Ctrl+O`) · Open Folder… (`Ctrl+Alt+O`) · Open Workspace… · Open Recent ▸ · Connect to Host… · Save (`Ctrl+S`) · Save As… (`Ctrl+Shift+S`) · Save All (`Ctrl+Alt+S`) · Revert File · Workspace Trust… · Checkpoint Workspace · Print… (`Ctrl+Shift+P`) · Close Tab (`Ctrl+W`) · Close Workspace (`Ctrl+Shift+W`) · Quit (`Ctrl+Q`) |
| Edit | Undo (`Ctrl+Z`) · Redo (`Ctrl+Shift+Z`) · Cut, Copy, Paste (`Ctrl+X/C/V`) · Find (`Ctrl+F`) · Replace (`Ctrl+H`) · Find in Workspace (`Ctrl+Shift+F`) · Replace in Workspace (`Ctrl+Shift+H`) · Format Document (`Ctrl+Shift+I`) · Toggle Comment (`Ctrl+/`) · Code Actions… (`Ctrl+.`) · Rename Symbol… (`F2`) · Preferences (`Ctrl+,`) |
| View | Command Palette (`Ctrl+K`) · Project, Outline, Search, Tests, Version Control (`Ctrl+Shift+1..5`) · Diagnostics (`Ctrl+Shift+M`) · Output (`Ctrl+Shift+U`) · Terminal (``Ctrl+` ``) · REPL (`Ctrl+Alt+R`) · Sidebar (`F9`) · Inspector (`F4`) · Layouts ▸ Edit, Debug, Review, Save Layout… · UI Preview (`Ctrl+Shift+V`) · Show Bytecode · Inlay Hints (`Ctrl+Alt+H`) · Zoom In, Out, Reset (`Ctrl++/-/0`) · Full Screen (`F11`) |
| Go | Go to File… (`Ctrl+P`) · Symbol in File… (`Ctrl+Shift+O`) · Symbol in Workspace… (`Ctrl+T`) · Line… (`Ctrl+L`) · Definition (`F12`) · Declaration (`Ctrl+F12`) · Type Definition (`Ctrl+Shift+F12`) · Implementation (`Ctrl+Alt+F12`) · Find References (`Shift+F12`) · Call Hierarchy (`Ctrl+Alt+Shift+H`) · Switch Header/Source (`Alt+O`) · Back, Forward (`Alt+Left/Right`) · Last Edit (`Ctrl+Shift+Backspace`) · Next, Previous Problem (`F8`, `Shift+F8`) |
| Build | Check (`Ctrl+Shift+C`) · Configure · Build (`Ctrl+B`) · Rebuild · Clean · Run Task… (`Ctrl+Shift+B`) · Cancel (`Ctrl+Break`) · Fetch Dependencies… · Package and Sign… · Verify Bundle… · Run PRISM Gate |
| Run | Run Without Debugging (`Ctrl+F5`) · Start or Continue Debugging (`F5`) · Stop (`Shift+F5`) · Restart (`Ctrl+Shift+F5`) · Step Over (`F10`) · Step Into (`F11`) · Step Out (`Shift+F11`) · Run to Caret (`Ctrl+F10`) · Toggle Breakpoint (`F9` with editor focus) · New Breakpoint ▸ Conditional, Log Point, Exception, Capability · Run with Capabilities… (`Ctrl+Alt+F5`) · Edit Launch Configurations… |
| Test | Run All (`Ctrl+; A`) · Run Failed (`Ctrl+; E`) · Run at Caret (`Ctrl+; C`) · Run in File (`Ctrl+; F`) · Debug Test at Caret (`Ctrl+; D`) · Run With Coverage · Fuzz Target… |
| Tools | Toolchains… · Extensions… · Sandbox Report · Kernel ▸ Build Kernel, Boot in VM, Attach Kernel Debugger · Assistant ▸ Explain Selection, Explain Problem, Suggest Edit |
| Help | Help for Symbol (`F1`) · IDE Handbook · Keyboard Shortcuts · About |

`F9` toggles the sidebar when focus is outside an editor and toggles a breakpoint when focus is in an editor; both are rebindable. Call hierarchy uses `Ctrl+Alt+Shift+H` so it does not collide with inlay hints.

### 5.5 Keyboard and pointer

- Chords: `Ctrl+;` then a letter for tests. All chords show a hint in the status bar after the first key.
- Editor context menu: Go to Definition, Find References, Peek, Rename, Code Actions, Format Selection, Run to Caret, Add Watch, Toggle Breakpoint, Send to REPL, Explain (if AI on), Cut, Copy, Paste.
- Project tree context menu: New File, New Folder, Rename, Delete, Duplicate, Copy Path, Reveal in Files, Open in Terminal, Find in Folder, Run Tests in Folder, Sign File, Compare With…, Exclude.
- Diagnostics row: Go to, Quick Fix, Suppress, Copy, Open Rule Help, Open in Proof Workbench (PRISM rows).
- Drag and drop: files from Files into the editor area (open) or the tree (copy into project, after a confirm listing the target); tabs between groups and to edges; a tab out of the window makes a new window; a test onto the debug button starts a debug run; text between editors.
- Gutter: click sets a breakpoint, `Shift+click` a log point, right click the breakpoint menu; click on a coverage bar shows hit counts.
- `Ctrl+click` on a symbol goes to definition; `Ctrl+hover` underlines it.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start page: recent workspaces, Open Folder, New Project, a one-line note "Opening a folder runs nothing." |
| Loading | Tree and editor usable at once; index and server start-up show in the status bar; features that need them show "indexing…" in place of results. |
| Error | Tool missing or crashed: banner naming the tool, "Restart" and "Open Software". Build failure: Diagnostics panel raised, first error selected. |
| Offline | Normal. Network-needing tasks state which host they need before they start. |
| Locked | Session lock pauses nothing by default; option "pause debuggee and hide terminals on lock". |
| Read-only | Restricted trust on a read-only handle, snapshot checkpoints, generated files: lock icon on the tab, edits refused with a toast. |
| Narrow (< 640 px) | Left dock becomes an overlay, right dock a sheet, bottom dock a full-height sheet; build bar collapses to run, debug and a `…` menu. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Workbench docks | `dockArea`, `dockWidget`, `splitter`, `tabWidget` (FUSE-WP-31 for IDE-quality docking and tear-off) |
| Editor | code editor engine behind `codeEdit` (FUSE-WP-37) |
| Diff and merge lenses | diff view (FUSE-WP-38) |
| Project, outline, call hierarchy, variables, tests | `treeView` (FUSE-WP-28 `treeView2` for columns) |
| Diagnostics, denials, coverage table | `tableView` with `SortFilterProxy` |
| Build bar | `toolButton`, `comboBox` |
| Preview | `viewportWidget` |
| Manifest form, launch editor | `propertyGrid`, `checkBox`, `radioButton`, `lineEdit`, `spinBox` |
| Docs and assistant output | `markdownView` |
| Pipeline DAG, include graph | `nodeGraph` (read-only use) |
| Profiler frame graph | `chart`, `timeline` |
| Menus, dialogs | `menuBar`, `popupMenu`, `dialog`, `tooltip` |

Missing:

| ID | Missing | Note |
|---|---|---|
| G-13 | Terminal grid | FUSE-WP-39 |
| DV01-GAP-1 | Flame chart widget | Could be built on `viewportWidget` with own drawing; a shared widget is better (SY04 also needs it) |
| DV01-GAP-2 | System debug broker | Not a widget: a service that lets a debugger attach to or launch a process under policy. See section 13 |
| DV01-GAP-3 | Completion and hover popover anchored to a text position | Needs popover and caret-rect API from FUSE-WP-37 |
| DV01-GAP-4 | Breadcrumb bar | Listed as missing in FUSE_BUILD_PLAN.md |
| DV01-GAP-5 | Inline decorations in the code editor (end-of-line text, code lens rows, inlay hints, ghost text) | Requirement on FUSE-WP-37 |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Workbench | ForgeScript + FUSE UI | app host | Windows, docks, commands, settings, session, all view models |
| Project model | ForgeScript | app host | Workspace file, folders, trust store, exclusions, task and launch configurations |
| Diagnostics model | ForgeScript | app host | Merges diagnostics from every source into one table keyed by file hash |
| Language client | ForgeScript | app host | Speaks the language server protocol to servers over pipes passed by the launcher |
| Debug client | ForgeScript | app host | Speaks the debug adapter protocol |
| ForgeScript language server | C++23 (links `forge_script` checker and parser; FORGE-WP-37) | sandbox, read-only project | Diagnostics, completion, navigation, rename, formatting |
| ForgeScript debug adapter | C++23 (FORGE-WP-36) | sandbox with the debuggee engine | Breakpoints, stepping, data |
| UI language service | ForgeScript | app host | `.fuseui` schema, binding and action checks, CSS subset, locale tables |
| Preview helper `ui-preview` | C++23 (FUSE UI, headless) | bundle helper, no capabilities | Renders a `.fuseui` with mock model to shared-memory frames |
| Search and index helper `index-helper` | C++23 | bundle helper, project `r`, cache `rw` | Text search, file-name index, symbol index store |
| Protocol codec helper `wire-helper` | C++23 | bundle helper, no capabilities | Parses JSON-RPC framing from servers and adapters, enforces size and depth limits before the app host sees messages |
| C++ language server | clangd (external system package, not safe subset) | analysis sandbox | C++ intelligence |
| Native debug adapter | lldb debug adapter (external package) | debug sandbox | Native and kernel debugging |
| Build tools | cmake, ninja, clang, llvm-cov, clang-format (external packages) | build sandbox | Build, coverage, format |
| Bundle tools | `forge bundle sign/verify/inspect` (FORGE-WP-09.3), cooker | build sandbox | Packaging |
| Extension hosts | ForgeScript engines | one per extension, in-host isolated engines or child hosts | Extensions |

```
                    +-------------------- app host (org.pbsd.ide) --------------------+
 user ── FUSE UI ── | Workbench · Project model · Diagnostics model · Language client | 
                    | Debug client · UI language service · Extension engines          |
                    +---+-----------+-----------+-------------+-----------+----------+
                        |           |           |             |           |
                  wire-helper  index-helper  ui-preview   session bus   system launcher (proc.exec)
                  (no caps)    (project r)   (no caps)    |              |
                        |                                 |      +-------+---------------------------+
          pipes to servers/adapters                 svc.* , DV03, |  analysis sandbox: FS server, clangd |
                                                    DV06, SE04    |  build sandbox: cmake, ninja, clang  |
                                                                  |  debug sandbox: adapters + debuggee  |
                                                                  |  kernel VM: QEMU or VM01 service     |
                                                                  +-------------------------------------+
```

External tools are not bundle helpers. They are installed packages started by the launcher in fresh domains whose handles the IDE lists explicitly.

### 6.2 Data model

Workspace file `*.pbsdws` (JSON, `schema: 1`):

| Field | Type | Notes |
|---|---|---|
| `schema` | int | forward-only migration, previous version kept as `.bak` |
| `folders[]` | `{grant_id: string, name: string, kind: "app"\|"native"\|"kernel"\|"package"\|"plain"}` | no absolute paths |
| `settings` | map | folder and workspace layers |
| `tasks[]` | `{id, label, program: {package, name}, args: string[], cwd: grant_id, env: map, sandbox: profile_id, matcher: string[], depends: id[]}` | |
| `launch[]` | `{id, label, kind: "forgescript"\|"app"\|"native"\|"kernel-vm"\|"attach", target, args, sandbox, capabilities: map<cap, mode>, pre_task}` | |
| `sandboxes[]` | `{id, project: "r"\|"rw", build_dir, extra_ro: grant_id[], net: "none"\|{hosts[]}, cpu, mem_bytes, time_s, env}` | |
| `toolchain` | `{package: string, version: string, hash: string}` | |

App store (SQLite, `~/.local/share/org.pbsd.ide/ide.db`, `user_version` = schema):

| Table | Fields |
|---|---|
| `workspace` | `id TEXT PK`, `name`, `last_opened INT`, `layout TEXT` (dock text), `file_grant TEXT` |
| `trust` | `workspace_id`, `folder_grant`, `level INT (0..2)`, `marker_hash BLOB(32)`, `decided_at INT` |
| `session_tab` | `workspace_id`, `group INT`, `ord INT`, `file_ref`, `caret`, `scroll`, `folds BLOB` |
| `breakpoint` | `id`, `workspace_id`, `file_ref`, `line`, `kind`, `condition`, `hit_count`, `log_message`, `enabled` |
| `watch` | `workspace_id`, `ord`, `expr` |
| `test_run` | `id`, `workspace_id`, `started`, `duration_ms`, `passed`, `failed`, `skipped` |
| `test_result` | `run_id`, `test_id TEXT`, `status`, `duration_ms`, `message` |
| `diag_cache` | `file_hash BLOB(32)`, `source TEXT`, `payload BLOB` (so stale results are detectable) |
| `gate_result` | `file_hash`, `tool`, `stage`, `tier INT`, `status`, `report_ref`, `recorded_at` |
| `sandbox_run` | `id`, `workspace_id`, `profile_id`, `started`, `wall_ms`, `cpu_ms`, `peak_bytes`, `denials INT` |
| `denial` | `run_id`, `ts`, `capability`, `resource`, `source_file`, `line`, `handled INT` |

Cache (`~/.cache/org.pbsd.ide/<workspace>/`): symbol index, clangd index, build directories (default), parse cache (signed), preview frames. All may be deleted.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| ForgeScript source | yes | yes | FORGE parser and checker | ForgeScript language server sandbox |
| C++ source | yes | yes | clangd | analysis sandbox |
| `.fuseui`, JSON, manifest | yes | yes | own strict JSON in ForgeScript; FUSE doc loader for preview | app host (size-capped) and `ui-preview` |
| CSS subset | yes | yes | own parser in ForgeScript | app host |
| `.tbl` locale tables | yes | yes | own | app host |
| CMake files, presets | yes (grammar), presets JSON | presets | own grammar; cmake itself | app host; cmake in build sandbox |
| `compile_commands.json` | yes | no | own JSON | `index-helper` |
| Language server protocol 3.17 (JSON-RPC) | yes | yes | own codec | `wire-helper` |
| Debug adapter protocol | yes | yes | own codec | `wire-helper` |
| gdb-remote serial protocol | via LLDB | via LLDB | LLDB | debug sandbox |
| SARIF 2.1.0, PRISM `report.json`, `stages.jsonl` | yes | SARIF export | own JSON | `index-helper` |
| Sanitizer and compiler text output | yes | no | problem matchers (regex with step limit) | app host |
| Coverage (`llvm-cov export` JSON, lcov) | yes | lcov | own | `index-helper` |
| CycloneDX JSON | no | yes | own | app host |
| SSH, SFTP | yes | yes | system SSH client service (section 13) | that service |
| ELF, DWARF | via LLDB | no | LLDB | debug sandbox |

### 6.4 Integration

- Intents handled: `open`, `edit` for source types and folders; `open` with `file:line:col`; `new` (project). Emitted: `edit` to DV02 (`.fuseui`), `open` to DV03 (repository view), DV04 (binary, memory region), DV06 (`prism-item`), DV07 (binary), UT05 (help page), SY02 (reveal), `share`, `print`.
- MIME: see `manifest.json`.
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.model`, `svc.backup`, `svc.audit`, `svc.undo-trash`, `svc.a11y`, `svc.power` (inhibit sleep during long builds).
- IPC offered: `org.pbsd.ide.Open(file_handle, line, col)`; `org.pbsd.ide.Diagnostics` (read-only stream for DV06 to mirror findings).
- DV03: repository service for status, hunks, blame, commit. DV06: job submit and result query. SE04: sign digest. SY03: terminal session service. VM01: disposable VM for kernel boots.
- Backing repos. Verified: checker JSON output, REPL commands, `disassemble`, `type_of`, debug detectors, signing, emit targets (`FORGE:docs/scripting.md`); MathScript Clang plugin exists with twenty rules (`MATHSCRIPT:README.md`); PRISM report files (`PRISM:README.md`); PBSD analyser plugin (`docs/specs/ANALYSER.md`); `.fuseui` hot reload and cooker (`FUSE:Source/FUSE/UI/Doc`). Proposed: language server, debugger, profiler, test framework, formatter (FORGE-WP-35, 36, 37), everything in the IDE itself.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Settings, session store, caches, default build directories |
| `fs.portal` | Project folders and files the user opens; grants remembered per workspace |
| `proc.exec` | Start toolchain programs, language servers, debug adapters and debuggees through the system launcher, each in its own domain |
| `proc.helper:ui-preview`, `proc.helper:index-helper`, `proc.helper:wire-helper` | Bundle helpers |
| `net.none` | Default. The IDE process itself opens no sockets |
| `net.user` | Only for remote development: hosts the user types. Off until the first remote connection |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.backup`, `svc.audit`, `svc.undo-trash`, `svc.a11y`, `svc.power` | As in 6.4 |
| `svc.model`, `ai.local` | Optional local assistance; off by default |

Assets: the user's source and signing ability; keys (never in the IDE); the rest of the user's files; the session.

Adversaries: a malicious repository (the main one); a malicious dependency or toolchain plugin; a malicious extension; a compromised language server; hostile build output; a hostile remote host; a model prompt-injected by project text.

| Threat | Mitigation |
|---|---|
| Code runs on open (task files, CMake, hooks, formatter configs, server configs) | Trust levels (F020 to F022). Restricted mode uses only in-process ForgeScript parsers with size and step limits |
| Build script reads the home directory or keys | Build sandbox has only the project, toolchain and a private temp; no keyring, no session bus services |
| Build exfiltrates source | No network in the build sandbox; named hosts need a prompt and are logged |
| Compromised clangd or adapter | Runs with project read-only (servers) and no network; its output is parsed by `wire-helper` with limits; edits it proposes are applied by the app host only inside the workspace handles |
| Path escape in server-proposed edits or problem-matcher paths | All file references are resolved beneath granted directory handles (`openat` beneath); anything else is refused |
| Trojan source (bidi controls, homoglyphs) | F054 diagnostics; diff views show them as visible chips |
| Changed run-defining files after trust | Marker hash check and review-before-run (F030, F031) |
| Debugger abused to read other processes | Launch-only by default; attach goes through the debug broker with a shell prompt naming the target; never across users |
| Signing key theft | Key stays in SE04; IDE passes a digest; signing shows an SE04 prompt with the bundle id and tree hash |
| Malicious extension | Signed ForgeScript only, own engine, default-deny policy, limits; no native code |
| Model prompt injection | The assistant has no tools; output is text or a diff the user applies; suggestions are checker-screened |
| Hostile remote host | Host key pinning; remote paths never interpreted locally; no automatic binary upload |
| Terminal escape sequences from build output | Output panel renders a safe subset; the terminal widget's own rules (SY03) apply |

Stored: workspace file (user's place), app store and caches (section 6.2). Leaves the machine: nothing by default. With remote development: SSH traffic to the typed host. With fetch tasks: requests to the named hosts.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `trust.default_level` | enum | `restricted` | Level for new folders (cannot be set above `analyse`) |
| `trust.review_run_files` | bool | true | Ask when run-defining files change |
| `sandbox.build.mem_bytes` | int | 8 GiB | Default build memory cap |
| `sandbox.build.time_s` | int | 1800 | Default task time limit |
| `sandbox.server.mem_bytes` | int | 4 GiB | Language server memory cap |
| `editor.*` | — | as OF01 | Shared editor settings |
| `editor.inlay_hints` | bool | false | Show inlay hints |
| `editor.format_on_save` | enum | `off` | `off`, `changed_lines`, `file` |
| `forgescript.strict_check` | bool | false | Warnings refuse the run |
| `forgescript.debug_detectors` | bool | true | Run debug sessions with race, lock-order and stall detectors |
| `forgescript.deadlock_ms` | int | 1000 | Stall detector timeout |
| `cpp.server_package` | string | `clangd` | Which installed server to use |
| `cpp.subset_profile` | enum | per target | `app-helper`, `service`, `kernel`, `none` |
| `build.parallel_jobs` | int | cores − 1 | Ninja jobs |
| `build.notify_when_unfocused` | bool | true | Notification at build end |
| `test.run_on_save` | bool | false | Run tests of the saved file |
| `debug.switch_layout` | bool | true | Switch to the Debug layout |
| `debug.allow_side_effects` | bool | false | Evaluate with side effects |
| `debug.pause_on_lock` | bool | false | Pause debuggee on session lock |
| `preview.theme`, `preview.density`, `preview.scale` | enum, enum, int | dark, compact, 100 | Preview defaults |
| `proof.show_tier_chips` | bool | true | Tier chips in code lens |
| `ai.enabled` | bool | false | Local assistance |
| `ai.inline_completion` | bool | false | Ghost text |
| `ai.exclude_globs` | string[] | `*.key, *.pem, .env*` | Never sent to the model |
| `remote.cache_encrypt` | bool | true | Encrypt the remote file cache |
| `kernel.emulator` | enum | `qemu` | `qemu`, `bhyve` (through VM01) |
| `layout.default` | string | `Edit` | Start layout |
| `keymap.preset` | enum | `default` | Keymap preset |

## 9. Accessibility and languages

- Diagnostics are reachable by keyboard from the line (`Alt+F8` reads all problems on the line); severity is icon, word and colour.
- Completion and signature popups are in the a11y tree as a list with the active item announced; the announcement rate is limited so typing is not drowned.
- Debugger stops announce "Stopped at file, line, reason".
- Coverage and diff use patterns as well as colour (hatched for uncovered).
- Identifiers are never translated; UI strings are. Right-to-left UI mirrors docks; code stays left-to-right with UAX 9 per line.
- Font size, line height and ligatures are settable separately for editor, terminal and panels.

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame, no workspace | ≤ 400 ms |
| Open a 10 000-file workspace to a usable tree and editor | ≤ 1 s (index continues in background) |
| Keystroke to paint | ≤ 8 ms median, 16 ms p99; never waits for a server |
| ForgeScript checker, one file re-check | ≤ 30 ms (FORGE build plan budget); 20 000 lines ≤ 300 ms |
| Completion popup after typing stops | ≤ 100 ms for ForgeScript, ≤ 300 ms for C++ warm |
| Workspace text search, 1 GiB of sources | first results ≤ 200 ms, complete ≤ 5 s on NVMe |
| Preview refresh after a valid edit | ≤ 50 ms |
| Memory, app host with 20 tabs | ≤ 400 MiB excluding servers |
| Idle | zero frames, zero wake-ups with no jobs running |
| Largest workspace | 1 million files indexed by name; symbol index for 5 million lines |

## 11. Testing and proof

Test plan:

- Unit (ForgeScript): project model, trust store, task expansion, manifest lint rules, problem matchers, binding checker, CSS subset parser, locale table checks.
- Golden renders: every panel and dialog in dark, light, high contrast, at 100% and 200%.
- Scripted UI through the a11y tree: the 15 top tasks.
- Protocol conformance: recorded language-server and debug-adapter sessions replayed against the clients.
- Fuzz: `wire-helper` framing and JSON, `index-helper` parsers (SARIF, coverage, compile database), workspace file loader, problem matchers (regex step limits), `.fuseui` schema checker.
- Sandbox tests: a hostile fixture project that tries to run on open, read `~`, open a socket, escape by symlink, and write outside the build directory; each must be denied and recorded.
- Differential: ForgeScript language server diagnostics equal `forge script --check --json` on the same files; IDE bundle verify equals the CLI.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Trust store and run gate | No program starts for a folder below "Build"; marker hash change lowers trust | T2 | PRISM on the gate function; model of the state machine | Launcher is outside the module |
| Sandbox profile compiler | Emitted handle set equals the profile; never includes an ungranted path | T1 | Lean model shared with the manifest-to-policy compiler (FORGE-WP-08.3) | Needs that model first |
| `wire-helper` | No runtime errors on any input; output bounded | T2 | PRISM + fuzz | — |
| `index-helper` | Safe subset, no out-of-handle access | T3 | Subset plugin, sanitizers, fuzz | Size of code |
| `ui-preview` | Safe subset | T3 | Subset plugin, sanitizers, fuzz of documents | Depends on FUSE tier |
| Manifest linter | Flags every capability used and not declared, for built-ins in the catalogue | T3 | Differential against run-time denials in the simulator | Static analysis is incomplete by nature |
| Workbench ForgeScript | Engine safety only | T4 | Tests | Interpreter semantics not modelled |
| clangd, LLDB, cmake | none claimed | T4 | Confined by sandbox | External code |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| DV01-W01 | Workbench shell: docks, tabs, command palette, status bar, layouts | G-01, G-02, G-07, FUSE-WP-31 |
| DV01-W02 | Project model, workspace file, project tree, file operations | DV01-W01, G-09 (portal) |
| DV01-W03 | Editor integration on the shared core, decorations API | FUSE-WP-37, OF01, DV01-GAP-3, DV01-GAP-5 |
| DV01-W04 | Trust levels, trust banner, trust store | DV01-W02 |
| DV01-W05 | Sandbox profiles and launcher integration | FORGE-WP-27.5, G-06, DV01-W04 |
| DV01-W06 | `wire-helper` and language client | FORGE-WP-28.1 |
| DV01-W07 | ForgeScript language server integration: diagnostics, completion, navigation | FORGE-WP-37, FORGE-WP-15, DV01-W06 |
| DV01-W08 | Diagnostics model and panel, problem matchers | DV01-W03 |
| DV01-W09 | Task model, build bar, output panel, run configurations | DV01-W05 |
| DV01-W10 | CMake, Ninja, clangd integration; subset plugin diagnostics from builds | DV01-W09, toolchain packages (SY05) |
| DV01-W11 | UI language service: `.fuseui` schema, CSS subset; preview helper | DV01-W03, FUSE doc library |
| DV01-W12 | Manifest editor and capability lint | DV01-W07 |
| DV01-W13 | `index-helper`: search in workspace, go to file | DV01-W02 |
| DV01-W14 | New project wizard and templates | DV01-W02, G-07 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| DV01-W20 | Debug client, debug layout, breakpoints store | DV01-W06 |
| DV01-W21 | ForgeScript debugger integration | FORGE-WP-36, DV01-W20 |
| DV01-W22 | Native debugging through LLDB adapter (launch) | DV01-W20, DV01-GAP-2 for attach |
| DV01-W23 | Test explorer, golden review, coverage | FORGE-WP-35, FUSE-WP-38, DV01-W09 |
| DV01-W24 | Sanitizer and fuzzer configurations | DV01-W10 |
| DV01-W25 | PRISM results inline, tier chips, DV06 hand-off | DV06, DV01-W08 |
| DV01-W26 | REPL panel, bytecode view, signing commands | DV01-W07, SE04 |
| DV01-W27 | Capability simulator, denial log | FORGE-WP-08.5, DV01-W12 |
| DV01-W28 | Bundle builder, packaging and signing wizard, verify | FORGE-WP-09.3, SE04 |
| DV01-W29 | Terminal panel | G-13, FUSE-WP-39, SY03 |
| DV01-W30 | Version-control panel, diff gutter | DV03 service |
| DV01-W31 | Rename, code actions, replace in workspace, symbol index | DV01-W07, DV01-W13 |
| DV01-W32 | Docs panel | UT05, G-09 (`svc.search`) |
| DV01-W33 | Kernel project kind, kernel build, VM boot, serial console | VM01, DV01-W29 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| DV01-W40 | Profilers and memory tools, flame chart | FORGE-WP-36, DV01-GAP-1, DV01-GAP-2 |
| DV01-W41 | Local AI assistance | G-15, AI02, `svc.model` |
| DV01-W42 | Extensions | DV01-W01, signing chain |
| DV01-W43 | Remote development | SSH client service (section 13), SE04 |
| DV01-W44 | Kernel debugger, boot test loop, crash dumps | DV01-W22, DV01-W33 |
| DV01-W45 | Disposable workspaces, reproducible build check, SBOM | DV01-W05, DV04 |
| DV01-W46 | UI debugging, IPC trace, mixed sessions | DV01-W21, G-08 |
| DV01-W47 | Emit to shell, AST view, parity runs | DV01-W26 |

## 13. Open questions and risks

1. **Debug broker (DV01-GAP-2).** HardenedBSD restricts tracing. A system service must decide who may launch-under-debugger or attach, show the prompt and hand a process descriptor to the adapter. Not in G-01..G-18.
2. **External tools are not in the safe subset.** clangd, LLDB, cmake and ninja are large C++ programs. They are confined, not trusted. A safe-subset C++ language server is out of reach; this stays a T4 island.
3. **Toolchains as read-only packages.** The launcher must be able to mount a toolchain package into a sandbox. Needs SY05 and FORGE-WP-27.5 to agree on a handle form.
4. **Sanitizers versus hardening.** Some sanitizer run times need mappings or ASLR settings HardenedBSD forbids. Each variant needs a tested exemption inside its sandbox, or it is dropped.
5. **ForgeScript tooling does not exist yet.** Language server, debugger, formatter, test framework and profiler are all Proposed (FORGE-WP-35 to 37). Phase 1 depends on WP-37.
6. **SSH client service.** Remote development, DV03, DV05, SY03, VM04 and IN03 all need SSH. One shared, sandboxed client service is better than five copies. Not in section 8 of PLATFORM.md.
7. **Subset plugin in the language server.** Whether the MathScript and PBSD plugins can load into clangd for on-type results is Unverified; until then their diagnostics appear after a build.
8. **Unverified.** The exact rule list and output format of the MathScript plugin (only the README line was read); PRISM's job interface (only report file names were read); QEMU and kgdb availability on HardenedBSD 15-STABLE; whether LLDB's adapter builds there.
9. **`Ctrl+P` conflict.** The design system reserves `Ctrl+P` for print. This spec gives it to "Go to File" and moves print to `Ctrl+Shift+P`. Needs a ruling; the alternative is `Ctrl+E` for Go to File.
