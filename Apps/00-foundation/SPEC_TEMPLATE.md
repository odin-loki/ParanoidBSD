<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# App spec template

By Odin Loch

Every `Apps/<category>/<slug>/SPEC.md` has exactly these sections, in this order, with these headings. A draft `manifest.json` (schema in PLATFORM.md section 12) sits beside it.

```markdown
<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# <Name>

By Odin Loch

| | |
|---|---|
| ID | `<APPID>` (from CATALOGUE.md) |
| Bundle | `org.pbsd.<slug>` |
| Category | ... |
| Priority | P0 / P1 / P2 / P3 |
| Backing repos | ... or none |
| Status | Planned. Nothing built. |

## 1. Purpose
Two or three sentences. Who uses it and for what. What it replaces.

## 2. Scope
In scope / out of scope (and which other app owns each out-of-scope item).

## 3. Users and top tasks
Personas, then the 8–15 tasks the app must make fast, each with its target number of steps.

## 4. Features
The full feature table. One row per feature, grouped under `### 4.x <group>` headings.
Columns: ID | Feature | Detail | Priority (M must / S should / C could) | Phase (1, 2, 3).
IDs are `<APPID>-F001`, `<APPID>-F002`, ... unique and never reused.
Be exhaustive: view, create, edit, organise, search, import, export, share, print, automate,
settings, security, accessibility, recovery.

## 5. GUI
### 5.1 Window map
Every window, dialog, sheet, popover and panel, as a tree.
### 5.2 Main window
ASCII wireframe (notation in DESIGN_SYSTEM.md section 8) with numbered callouts, sizes, and each region described.
### 5.3 Other views and dialogs
A wireframe and description for every other important view (at least the 4–8 most used).
### 5.4 Menus
The full menu tree with shortcuts.
### 5.5 Keyboard and pointer
App-specific shortcuts (on top of the reserved ones), context menus, drag and drop, gestures.
### 5.6 States
Empty, loading, error, offline, locked, read-only, narrow-window behaviour.
### 5.7 Widgets
Which FUSE widgets each region uses, and which widgets are missing (reference platform gap IDs or add `<APPID>-GAP-n`).

## 6. Architecture
### 6.1 Components
Table: Component | Language (ForgeScript / FUSE UI / C++23 / Rust) | Process | Responsibility.
Include a small Mermaid or ASCII diagram.
### 6.2 Data model
Entities, fields, stores, on-disk formats, schema versioning, migration.
### 6.3 File formats and protocols
Table: Format or protocol | Read | Write | Library or own code | Parser runs in (sandbox).
### 6.4 Integration
Intents handled and emitted, MIME types, system services used, IPC interfaces offered, links to other apps
(by APPID) and to backing repos (state what is Verified from the repo, with path, and what is Proposed).

## 7. Security and privacy
Capabilities requested (PLATFORM.md section 6) each with a reason; threat model (assets, adversaries, attack
surface); mitigations; what is stored and where; what leaves the machine (default: nothing).

## 8. Settings
Table: Key | Type | Default | Description.

## 9. Accessibility and languages
Anything app-specific beyond the system rules.

## 10. Performance budgets
Start time, memory, largest supported input, frame time under load.

## 11. Testing and proof
Test plan (unit, golden, scripted UI, fuzz, differential against a reference implementation where one exists).
Proof table: Module | Property | Target tier (T1–T4) | How it would be checked | What blocks a higher tier.
Never state a proof or test as passing: nothing has run.

## 12. Build plan
Phased milestones (Phase 1 usable core, Phase 2 full feature, Phase 3 advanced), each a list of work items
`<APPID>-W01` ... with dependencies (other work items, platform gaps `G-nn`, other apps).

## 13. Open questions and risks
```

Rules for writers:

- Plain English. Short sentences. No filler.
- Use exact, concrete detail: real field names, real shortcut keys, real file formats, real limits.
- Do not invent facts about existing repos. Read the repo; cite `REPO:path` for anything stated as existing; mark everything else Proposed.
- Do not name new components with invented brand names. Use plain functional names ("archive helper", "index service").
- Honour the platform rules: ForgeScript + FUSE UI for the app, C++23 helpers only where PLATFORM.md section 2 allows, no Qt, no JIT, no W+X, default-deny capabilities, no telemetry, no accounts, offline first.
- Third-party libraries: prefer own code or FORGE/FUSE facilities; when a library is proposed give its name, licence and why; it must build on HardenedBSD without network access.
