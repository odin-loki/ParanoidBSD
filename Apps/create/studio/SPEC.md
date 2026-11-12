<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Studio

By Odin Loch

| | |
|---|---|
| ID | `CR04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.studio` |
| Category | create |
| Priority | P2 |
| Backing repos | FUSE (engine, editor controller, Qt-free editor shell, tools), FORGE (ForgeScript), NeuralVFX (effect library, optional) |
| App kind | Native app (documented exception to PLATFORM.md section 2; see section 2 and 13) |
| Status | Planned. Nothing built as an app for this OS. The FUSE editor exists upstream (see 6.4). |

Labels: **Verified** = read in a repo at the cited path. **Proposed** = design in this plan. **Unverified** = could not be read.

## 1. Purpose

Studio is the FUSE editor shipped as an app: a workbench for building 2D, 3D and hybrid worlds, simulations and games, and for exporting them as signed app bundles for this OS. It is for game developers, simulation authors, teachers and hobbyists. It replaces the Qt 6 FUSE editor on this OS and stands where Godot, Unity or Unreal editors would. By owner decision the app is FUSE's existing Qt-free C++ editor shell (`fuse_editor_fuse` on `EditorController`), shipped directly.

## 2. Scope

**In scope:** project hub; viewports; scene hierarchy; inspector; asset browser and import; materials; SDF sculpt; terrain; navigation; animation and cinematics; physics set-up; VFX; audio placement; scripting with an embedded quick editor; visual scripting; play mode in a sandbox; profiler and debugger; build and export; simulation runs; templates and samples.

**Out of scope:**

| Item | Owner |
|---|---|
| Full code editing, language servers, C++ builds | DV01 IDE (Studio opens files there) |
| Designing `.fuseui` game and app interfaces | DV02 UI Designer (Studio hands off and embeds the result) |
| Commits, branches, merges | DV03 Version Control (Studio shows status only) |
| Texture painting and image editing | CR01 Image Editor |
| Audio editing | CR03 Audio Editor |
| Video editing of captured footage | CR02 Video Editor |
| Training neural effect models | NeuralVFX's own tools |
| Installing exported games | SY05 Software |
| Signing key storage | SE04 Passwords and Keys |
| The small games pack | GM01 Games (built with Studio, shipped separately) |
| Mobile and other-OS export | later; not in this plan |

## 3. Users and top tasks

Personas: **Game developer** (levels, scripts, play-test, export). **Simulation author** (deterministic runs, parameter sweeps, data out). **Technical artist** (materials, VFX, terrain, lighting). **Teacher or student** (templates, safe sharing of projects). **Reviewer** (opens someone else's project without trusting it).

| # | Task | Target steps |
|---|---|---|
| T1 | Create a 3D project from a template and see it in the viewport | 3 |
| T2 | Place a mesh, move it, undo | 3 |
| T3 | Import a glTF model and drop it in the world | 2 |
| T4 | Change a material's colour and roughness | 3 |
| T5 | Attach a script to an entity and open it | 2 |
| T6 | Press Play, test, press Stop, with the world restored | 2 keys (`F5`, `Shift+F5`) |
| T7 | Sculpt terrain and bake a navmesh | 4 |
| T8 | Key a camera move on the timeline | 5 |
| T9 | Find what is slow with the profiler | 3 |
| T10 | Export a signed app bundle and install it | 4 |
| T11 | Run a simulation 100 times with a seed sweep and get a CSV | 4 |
| T12 | Open an untrusted project safely | 1 (it opens restricted by default) |
| T13 | See what changed since the last commit | 1 |
| T14 | Recover work after a crash | 1 (accept the restore offer) |

## 4. Features

Priority: M must, S should, C could. Phase 1 usable core, 2 full feature, 3 advanced. The column **Today** says whether the upstream Qt-free shell already has it: **Y** = Verified in `FUSE:docs/plans/FUSE_EDITOR_QT_FIRST_PLAN.md` §11 or `FUSE:docs/ui/README.md` ("Editor shell (UI9)"); **P** = partly; **N** = Proposed here.

### 4.1 Project hub and projects

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F001 | Project hub | List of known projects, open, new; shown at start and from File. | Y | M | 1 |
| CR04-F002 | New Project wizard | Name, location (portal folder), dimension (3D, 2D, Hybrid), UI layer, the five module toggles (AI, cinematics, FX, mechanics, adventure); shows the folder it will create. | Y | M | 1 |
| CR04-F003 | Recent projects | File > Recent Projects; pinned projects. | Y | M | 1 |
| CR04-F004 | `project.json` | Schema 1: `schemaVersion`, `name`, `dimensions`, `modules`, `defaultWorld3D`, `defaultWorld2D`. Studio edits it through Project Settings. | Y | M | 1 |
| CR04-F005 | Project settings | Name, version, icon, dimensions, modules, default worlds, input map, physics settings, render settings, declared capabilities (section 7), export targets. | P | M | 2 |
| CR04-F006 | Templates | Empty 3D, empty 2D, hybrid with HUD, first-person, third-person, top-down, 2D platformer, simulation (headless-capable), tool app. Templates are signed project archives. | N | S | 2 |
| CR04-F007 | Samples browser | The upstream samples (`Samples/unification`: `demo_2d_sprites`, `demo_3d_empty`, `demo_adventure_stub`, `demo_ai_bt`, `demo_fx`, `demo_hybrid_hud`, `demo_timeline`) offered read-only with "Copy to my projects". | P | S | 2 |
| CR04-F008 | Project trust state | Each project is Trusted or Restricted. Restricted: scripts do not run, play mode is off, import of executable content is off. New projects made here are trusted; projects from elsewhere start restricted. | N | M | 1 |
| CR04-F009 | Project health check | Lists missing assets, broken references, schema versions, unsigned scripts, declared capabilities. | N | S | 2 |
| CR04-F010 | Project archive | Export and import a project as one signed archive without caches. | N | S | 2 |
| CR04-F011 | Multiple projects | One window and one Studio process per project. | N | S | 2 |

### 4.2 Viewports

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F012 | Live 3D viewport | The engine's scene colour target shown zero-copy in a `viewportWidget`; renders only when camera, size or scene changes, or every frame while playing. | Y | M | 1 |
| CR04-F013 | Software fallback | Without a GPU device the viewport shows a software grid view. | Y | M | 1 |
| CR04-F014 | Fly camera | RMB + WASDQE, wheel for dolly and fly speed; speed and sensitivity in Preferences. | Y | M | 1 |
| CR04-F015 | Orbit, pan, zoom | Alt+LMB orbit, MMB pan, wheel zoom; turntable or trackball. | P | M | 1 |
| CR04-F016 | Orthographic views | Top, Front, Side with grid, axes, bounds and selection; click selects, drag moves in the view plane as one undo step. | Y | M | 1 |
| CR04-F017 | Viewport layouts | Single, Two (Perspective + Top), Quad on `Ctrl+Alt+1..3`. | Y | M | 1 |
| CR04-F018 | View modes | Lit, Unlit, Wireframe, Lit Wireframe on `Alt+1..4`; Show Mesh Bounds. | Y | M | 1 |
| CR04-F019 | Debug views | Albedo, normals, roughness, metallic, depth, overdraw, light complexity, GI probes, shadow cascades, physics shapes, navmesh. | P | S | 2 |
| CR04-F020 | Transform gizmo | Select, Move, Rotate, Scale on keys `1`–`4`; World/Local toggle; Snap toggle; each drag is one undo macro; multi-selection follows. | Y | M | 1 |
| CR04-F021 | Snapping | Grid, angle, scale increments; surface snap; vertex snap; align to view. | P | M | 2 |
| CR04-F022 | Picking and box select | Click picks; Ctrl/Shift toggles; marquee select; select through. | P | M | 1 |
| CR04-F023 | Frame selected | `F` frames the selection; `Shift+F` frames all. | Y | M | 1 |
| CR04-F024 | 2D viewport | Pan and zoom canvas for World2D: sprites, tilemaps, 2D colliders, pixel grid, pixel-perfect preview. | N | M | 2 |
| CR04-F025 | Hybrid view | 3D world with the 2D HUD layer composed on top, as the frame composer does; toggle each layer. | P | S | 2 |
| CR04-F026 | Camera bookmarks and game cameras | Save and recall views; look through any camera entity; picture-in-picture camera preview. | N | S | 2 |
| CR04-F027 | Viewport statistics | Frame time, draw calls, triangles, entity count, GPU memory overlay. | N | S | 2 |
| CR04-F028 | Screenshot and capture | Save the viewport as PNG or EXR; record an image sequence for CR02. | P | S | 2 |
| CR04-F029 | Collider display | Collider wireframes and drag handles. | Y | M | 1 |
| CR04-F030 | Lighting preview controls | Preview sun, exposure, time of day, sky; do not change the saved world unless applied. | P | S | 2 |

### 4.3 Scene hierarchy and entities

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F031 | Hierarchy panel | Tree with filter, multi-select and reparent by drag. | Y | M | 1 |
| CR04-F032 | Entity operations | Create, rename, delete, duplicate, duplicate subtree, group, ungroup, copy and paste. | Y | M | 1 |
| CR04-F033 | Create menu | Empty, mesh primitives (cube and others), SDF shape, light (sun, point, spot), camera, sprite, tilemap, terrain, particle emitter, audio source, trigger volume, spawn marker, nav agent. | P | M | 1 |
| CR04-F034 | Visibility and lock | Eye and lock per entity, inherited by children; editor-only. | N | M | 2 |
| CR04-F035 | Prefabs | Save a subtree as a prefab asset; instances with per-instance overrides; apply and revert; nested prefabs. Upstream has prefab links in the editor library (`editor_prefab_links.cpp`). | P | M | 2 |
| CR04-F036 | Layers and tags | Named layers for rendering, physics and selection filters; free tags. | N | S | 2 |
| CR04-F037 | Search and filters | Filter by name, component type, tag, layer; saved searches. | P | S | 2 |
| CR04-F038 | World files | Open and save worlds as `.fuselevel`; several worlds per project; world tabs. | P | M | 1 |
| CR04-F039 | World partition | Streaming volumes, residency and budgets shown and edited in the viewport. | N | C | 3 |
| CR04-F040 | Undo and redo | Every edit goes through the host undo stack; history list; undo survives play mode. | Y | M | 1 |

### 4.4 Inspector and components

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F041 | Inspector | Property grid of the selection; add and remove components. | Y | M | 1 |
| CR04-F042 | Typed editors | Vectors, quaternions as Euler, colours (HDR), enums, asset references, curves, gradients, arrays, flags. | P | M | 2 |
| CR04-F043 | Multi-edit | Edit many entities at once; mixed values shown as "—". | N | M | 2 |
| CR04-F044 | Reset, copy, paste | Per property and per component; revert prefab override. | N | S | 2 |
| CR04-F045 | Script properties | Exposed script variables appear as typed rows. | N | M | 2 |
| CR04-F046 | Component search | Add Component opens a searchable list grouped by module. | P | M | 1 |
| CR04-F047 | Reference picker | Pick an asset or entity by search or by eyedropper in the viewport. | N | S | 2 |
| CR04-F048 | Lock and second inspector | Lock an inspector to an entity; open a second inspector. | N | C | 3 |

### 4.5 Assets

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F049 | Asset browser | Folders, filter, breadcrumb, list and grid, type filter (Mesh … Unknown), status line, context menu (Open, Copy Path, Copy Relative Path, Rename, New Folder, Refresh). | Y | M | 1 |
| CR04-F050 | Import | Drag files in or File > Import through the portal. Source formats: glTF 2.0 / GLB, OBJ, FBX (if a reader is licensed), PNG, TGA, JPEG, HDR, KTX2, WAV, FLAC, Ogg. | P | M | 1 |
| CR04-F051 | Cook | The cook tool turns sources into runtime formats: meshes to FMSH (`.fusemesh`), textures to `.fusetex` (BC1/4/5/6H/7), materials to `.fusemat`, audio to `.fuseaudio`, collision to `.fusecol`. Runs in a sandboxed helper. | P | M | 1 |
| CR04-F052 | Import settings | Per asset: scale, axis, normals, tangents, LODs, collision mode (hull, mesh, both), texture format, mip settings, normal-map flag, audio quality. Stored beside the source as `.import.json`. | P | M | 2 |
| CR04-F053 | Auto re-import | Watch the project folder; re-cook changed sources; cache keyed by source hash and settings. | P | M | 2 |
| CR04-F054 | Thumbnails and preview | Mesh, material, texture, audio and prefab previews; a preview pane with orbit. | P | S | 2 |
| CR04-F055 | Dependencies | "Used by" and "Uses" lists; safe rename and move that fix references; find unused assets. | N | M | 2 |
| CR04-F056 | Asset ids | Stable `AssetId` per asset; paths can change without breaking references. | P | M | 1 |
| CR04-F057 | Asset check and lint | Run the asset checker and lint on demand and before export; results in the Problems dock. | P | S | 2 |
| CR04-F058 | Asset provenance | Each imported asset records source file hash, import time, tool version, and a licence and author field the user fills. A project-wide licence report. | N | S | 2 |
| CR04-F059 | Open in other apps | Texture → CR01, audio → CR03, script → built-in editor or DV01, `.fuseui` → DV02. | N | M | 2 |
| CR04-F060 | Effect library import | Add a NeuralVFX effect pack (`.nvfxp`) as an asset; place library effects as sprites in the world. | N | C | 3 |

### 4.6 Materials, lighting, rendering

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F061 | Material editor | Preview sphere, new, roughness, metallic, shader, UV scale, base colour, texture, assign to selection. | Y | M | 1 |
| CR04-F062 | Full PBR inputs | Normal, emissive, ambient occlusion, opacity and alpha modes, clear coat, subsurface where the renderer supports them. | N | M | 2 |
| CR04-F063 | Material graph | Node-based materials in a `nodeGraph`; compiles to the engine's shader path. | N | S | 3 |
| CR04-F064 | Material library | Saved materials, drag onto meshes; material instances with parameter overrides. | P | S | 2 |
| CR04-F065 | Lights | Sun, point, spot, area; intensity units; shadows on or off; cookies. | P | M | 2 |
| CR04-F066 | Global illumination and post settings | Panels for DDGI, screen-space effects (HBAO, SSR, SSGI), fog, atmosphere, exposure, tone mapping, bloom; per-world and per-volume. | N | S | 2 |
| CR04-F067 | Environment | Sky, atmosphere, time of day, weather hooks. | N | S | 3 |
| CR04-F068 | SDF sculpt | Sculpt-in-viewport mode: op, shape, material, radius, strength, spacing, blend, mirror X; one undo step per stroke. | Y | S | 1 |
| CR04-F069 | Quality presets | Low to ultra presets mapped to render settings; preview each in the viewport. | N | S | 3 |

### 4.7 Terrain and navigation

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F070 | Terrain creation | Cells, size, height, seed; Create Terrain as one undo step. | Y | M | 1 |
| CR04-F071 | Terrain brushes | Raise, Lower, Flatten with radius and strength; sculpt in the viewport. | Y | M | 1 |
| CR04-F072 | More brushes | Smooth, noise, erode, stamp from heightmap, set height, ramp. | N | S | 2 |
| CR04-F073 | Terrain painting | Material layers painted by weight; auto rules by slope and height. | N | S | 3 |
| CR04-F074 | Heightmap import and export | 16-bit PNG, raw, EXR. | N | S | 2 |
| CR04-F075 | Foliage and scatter | Paint instanced meshes with density, scale and alignment rules. | N | C | 3 |
| CR04-F076 | Navigation panel | Cell size, half extent, Bake Navmesh (grid navmesh; colliders block cells), Show in viewport, From/To and Test Path (A*). | Y | M | 1 |
| CR04-F077 | Navigation volumes and agents | Bake bounds, cost areas, agent radius and step height; off-grid links. | N | S | 3 |

### 4.8 Animation and cinematics

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F078 | Timeline panel | Key selection, delete key, playhead, length, play; zoom, pan, loop, snap, key jumps. | Y | M | 1 |
| CR04-F079 | Sequencer panel | Scrub, link to the timeline, preview sample. | Y | M | 1 |
| CR04-F080 | Tracks | Camera, sprite, property, audio and event/cue lanes of the cinematics module, grouped. | P | M | 2 |
| CR04-F081 | Record keys | Auto-key mode: changing a property while the playhead is set adds a key. | N | M | 2 |
| CR04-F082 | Curve editing | Edit key curves in a `curveEditor` dock (needs the upgraded widget). | N | M | 2 |
| CR04-F083 | Skeletal animation | Clip preview, skeleton view, playback speed, root motion toggle, events on clips. | N | S | 2 |
| CR04-F084 | Animation state graph | States, transitions, blend trees and parameters in a `nodeGraph`. | N | S | 3 |
| CR04-F085 | Sprite animation | Frame strips, per-frame duration, pivot, flipbook preview. | N | S | 2 |
| CR04-F086 | Cinematic render | Render a sequence to an image sequence with fixed time step; hand off to CR02. | N | S | 3 |

### 4.9 Physics, VFX, audio, AI

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F087 | Colliders and bodies | Box, sphere, capsule, hull, mesh colliders; static, kinematic, dynamic bodies; mass, friction, restitution; triggers. | P | M | 2 |
| CR04-F088 | Joints and constraints | Hinge, slider, ball, fixed, spring with limits; gizmos in the viewport. | N | S | 3 |
| CR04-F089 | Physics debug | Contacts, sleeping, broad-phase, velocity vectors; step one physics tick. | N | S | 2 |
| CR04-F090 | Particle editor | Emitters, modules (spawn, velocity, forces, colour and size over life), preview; CPU particles (the engine's `fuse_vfx` is CPU particle scaffolding today). | N | S | 2 |
| CR04-F091 | Effect composer | Author `fuse_fx` effects: descriptors, timelines, graphs, sockets, casts. | N | S | 3 |
| CR04-F092 | Audio sources and listeners | Place sources; attenuation shapes shown in the viewport; preview in the editor; mixer buses for the game. | N | M | 2 |
| CR04-F093 | Behaviour tree editor | Edit `fuse_ai` trees (sequence, selector, parallel, decorators, leaves) in a `nodeGraph`; live highlight of the running node in play mode. | N | S | 3 |
| CR04-F094 | Mechanics and adventure tools | Interactable set-up, inventory items, puzzle gates as component editors. | N | C | 3 |

### 4.10 Scripting

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F095 | Open script | Entity > Open Script opens the attached script. | Y | M | 1 |
| CR04-F096 | Quick script editor | A `codeEdit` dock with highlighting, line numbers, find, go to line, error markers. "Open in IDE" sends the file to DV01. | N | M | 2 |
| CR04-F097 | Lua (today) | The engine's script host has a Null backend and an optional Lua backend. Studio supports Lua projects as the upstream does. | P | S | 2 |
| CR04-F098 | ForgeScript gameplay (Proposed) | A third script backend that runs signed ForgeScript on the game thread, with the engine API as host bindings. Default for new projects on this OS. | N | M | 2 |
| CR04-F099 | Script lifecycle | `OnStart`, `OnUpdate`, `OnDestroy`, collision and trigger hooks; errors are isolated per script and listed in the Console. | P | M | 2 |
| CR04-F100 | Hot reload | Reload changed scripts while playing; state kept where the script declares it. Upstream says hot reload is not in its current milestone. | N | S | 3 |
| CR04-F101 | Console | Log view with filters, a command line with history and prefix completion. | Y | M | 1 |
| CR04-F102 | Visual scripting | Event graphs in a `nodeGraph` that compile to ForgeScript source the user can read. Nodes: events, flow, maths, entity, component, timeline, audio. | N | S | 3 |
| CR04-F103 | API reference | Offline reference for the script API in the Help dock, generated from the bindings. | N | S | 2 |
| CR04-F104 | Editor scripting | Signed ForgeScript editor tools: menu commands, batch operations, custom inspectors. No native editor plugins. | N | C | 3 |

### 4.11 Play mode, profiling, debugging

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F105 | Play, pause, step, stop | Play in the viewport from the spawn marker; green frame and `PLAYING t s - N ticks` badge; amber `PAUSED`; Step ticks; Stop restores the edited world. | Y | M | 1 |
| CR04-F106 | Maximise on play | Play > Maximize Viewport on Play hides the docks. | Y | S | 1 |
| CR04-F107 | Sandboxed play | Play runs in a separate process with only the project's declared and granted capabilities, never the editor's. See section 7. | N | M | 2 |
| CR04-F108 | Play as installed app | Launch the project as a development bundle through the shell, so prompts, indicators and limits are exactly those of the exported app. | N | S | 2 |
| CR04-F109 | Play from here, play this world | Start at the editor camera; start a chosen world. | P | S | 2 |
| CR04-F110 | Live inspection | While playing, the hierarchy and inspector show the running world; edits are marked temporary and discarded on Stop unless "Keep". | N | S | 2 |
| CR04-F111 | Profiler | CPU history chart against the frame budget, pause, clear; samples only while its tab is shown. | Y | M | 1 |
| CR04-F112 | Profiler detail | Flame graph per frame (upstream has `profiler_flame_graph.cpp` in the editor library), GPU pass times, memory by allocator domain, job-system lanes, capture to file. | P | S | 2 |
| CR04-F113 | Script debugger | Breakpoints, step, call stack, locals for ForgeScript gameplay through FORGE's debugger protocol. | N | S | 3 |
| CR04-F114 | Input recording and replay | Record inputs in play; replay with single-thread jobs for a repeatable run. | N | S | 3 |
| CR04-F115 | Problems dock | Errors and warnings from scripts, cooks, lint and validation, each with "go to". | N | M | 2 |
| CR04-F116 | Capability log | A dock that lists every capability the running project used or was denied. | N | M | 2 |

### 4.12 Build and export

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F117 | Export wizard | Target (this OS), bundle id, name, version, icon, category, capabilities, entry world, quality preset, signing key. | N | M | 2 |
| CR04-F118 | App bundle output | Writes `<id>.app/` per PLATFORM.md section 3: `manifest.json`, scripts, cooked content, the runtime as a signed helper, `SIGNATURES`. | N | M | 2 |
| CR04-F119 | Capability review | The wizard lists what the project declared, what play-testing actually used (from the capability log), and flags declared-but-unused items. | N | M | 2 |
| CR04-F120 | Signing | Sign with a developer key held in `svc.keyring`; the private key never enters Studio. | N | M | 2 |
| CR04-F121 | Reproducible export | Same project, same tool versions, same output bytes; a build manifest with input hashes and an SBOM. | N | S | 3 |
| CR04-F122 | Content packing | Cooked assets packed with a table of contents and hashes; optional compression; strip editor-only data. | N | M | 2 |
| CR04-F123 | Export validation | Run the bundle linter, launch the bundle once in a throwaway sandbox, report first-frame time and errors. | N | S | 2 |
| CR04-F124 | Install and run | Hand the bundle to SY05 Software to install for this user. | N | S | 2 |
| CR04-F125 | Headless build | `studio --export <project> --preset <name>` for scripted builds, run from SY03 Terminal or DV01. | N | S | 3 |
| CR04-F126 | Export presets | Saved per project; debug and release variants. | N | S | 2 |

### 4.13 Simulation workflows

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F127 | Deterministic run mode | Fixed time step, single-thread jobs, seeded random; the same inputs give the same run. The engine has a single-thread job switch for replay and golden tests. | P | S | 3 |
| CR04-F128 | Headless runs | Run a world without a window for N ticks; progress in the Jobs dock. | N | S | 3 |
| CR04-F129 | Parameter sweeps | Define parameters and ranges; run a batch; one row per run. | N | S | 3 |
| CR04-F130 | Data recorders | Record chosen properties per tick to CSV or Parquet; open in OF09 Data Studio or AI04 Notebook. | N | S | 3 |
| CR04-F131 | Live plots | Chart recorded values during a run in a `chart` dock. | N | C | 3 |
| CR04-F132 | Checkpoints | Save and restore full simulation state; compare two runs tick by tick. | N | C | 3 |

### 4.14 Workbench, hand-offs, safety, recovery

| ID | Feature | Detail | Today | Pri | Ph |
|---|---|---|---|---|---|
| CR04-F133 | Docking workbench | Three dock regions with tabs, viewport, status bar; reset layout (left 260, right 320, bottom 200 px). | Y | M | 1 |
| CR04-F134 | Menus and toolbar from the action table | One table of actions feeds menus, toolbar, palette and shortcuts. | Y | M | 1 |
| CR04-F135 | Preferences | Preview sun, camera speed, look sensitivity, restore layout; shortcut list with filter, record, conflict check. | Y | M | 1 |
| CR04-F136 | Layouts | Saved layouts: Default, Level design, Animation, Scripting, Profiling, 2D. | N | S | 2 |
| CR04-F137 | UI Designer hand-off | A `.fuseui` asset opens in DV02; Studio shows a live preview of the game UI layer and reloads on save. | N | S | 2 |
| CR04-F138 | Version control hand-off | File status badges in the asset browser and hierarchy (modified, added, untracked); "Show changes" and "Commit…" open DV03 at the project. Studio never runs VCS commands itself. | N | S | 2 |
| CR04-F139 | Text-friendly files | Worlds, prefabs, materials and settings are saved as stable, diffable text. Upstream lists native world files as later work. | P | M | 2 |
| CR04-F140 | Crash recovery | Editor and engine share one process. After a crash the shell offers to reopen Studio; it reloads the last saved world and replays the edit journal. | N | M | 1 |
| CR04-F141 | Autosave | Journal of editor commands; snapshot every 2 minutes; restore offered at start. | N | M | 1 |
| CR04-F142 | Untrusted import | Imported files are parsed only in the cook helper; a failed cook gives a labelled placeholder (the cook tool has a lenient mode) and a report. | P | M | 1 |
| CR04-F143 | Help and tours | Offline help in a `markdownView` dock; first-run tour of the workbench. | N | S | 2 |
| CR04-F144 | Font scale and themes | UI scale 1 to 2 (the shell is gated at 1, 1.25, 1.5, 2), system theme tokens, high contrast. | Y | M | 1 |

## 5. GUI

### 5.1 Window map

```
Project hub window (start)
└─ New Project wizard · Template gallery · Samples · Open folder (portal)
Main window (one per project; workbench with docks)
├─ Header bar: menu, project — world, search, badges ([gpu]; play badges mirror the running game)
├─ Toolbar: tools, space, snap, play controls, layout
├─ Left dock: Hierarchy · Asset Browser (tree)
├─ Centre: Viewport(s) — single / two / quad · 2D view · game view
├─ Right dock: Inspector · Navigation · Terrain · Material Editor · SDF Sculpt
├─ Bottom dock: Asset Browser · Console · Problems · Timeline · Sequencer · Curves · Profiler ·
│               Capability log · Jobs · Script editor · Help
├─ Graph editors (tabs in the centre): Material graph · Visual script · Behaviour tree ·
│               Animation state graph · Effect composer
├─ Dialogs
│   ├─ Project settings        ├─ Import settings         ├─ Export wizard
│   ├─ Preferences             ├─ Shortcuts               ├─ Project trust
│   ├─ Simulation batch        ├─ Add component           ├─ Project health
│   └─ Open world / Save world as
└─ Popovers: create menu, view mode, snap settings, camera bookmarks, quick asset picker
```

### 5.2 Main window

Shown at 1600 × 900, default layout (left 260, right 320, bottom 200 px, as the upstream layout reset sets). States: empty, loading, playing, paused, restricted, engine restarting.

```
+------------------------------------------------------------------------------------------+
| [=] Studio — island / main.fuselevel*      [search Ctrl+K]          [gpu] [▁] [▢] [✕]   |(1)
+------------------------------------------------------------------------------------------+
| [↖1][✥2][⟳3][⤢4] [World v] [snap ▦] | [▶ Play F5] [⏸] [⏭] [■] | [Lit v] [▣ single v]    |(2)
+--------------+----------------------------------------------------------+----------------+
| Hierarchy    | Perspective                                    60 fps    | Inspector      |
| [filter___]  |                                                          | Rock_03        |
| ▾ World      |                                                          | ▾ Transform    |
|   ☀ Sun      |                      ▲                                   |  Pos  4 0 -2   |
|   ▣ Terrain  |                     ╱│╲    (gizmo)                       |  Rot  0 35 0   |
|  >▣ Rock_03  |               ░░░░░░░░░░░░░                              |  Scale 1 1 1   |
|   ▾ Village  |            ░░░░░░░░░░░░░░░░░░░                           | ▾ Mesh         |
|     ▣ Hut_01 |                                                          |  Asset rock.fm…|
|     ▣ Hut_02 |                                                          |  Material moss |
|   ◉ Player   |                       (4)                                | ▾ Collider     |
|   ⚑ Spawn    |                                                          |  Shape [Hull v]|
|     (3)      |                                                          | [+ Add component]|
|              |                                                          |      (5)       |
+--------------+----------------------------------------------------------+----------------+
| Assets | Console | Problems 2 | Timeline | Profiler | Capability log                     |(6)
| Project > meshes > rocks        [filter___] [type: All v] [▤|▦]                          |
| [▣ rock_01] [▣ rock_02] [▣ rock_03] [▣ cliff] [▣ pebble]                                 |
| 5 items (0 folders) · rock_03 (Mesh)                                                     |
+------------------------------------------------------------------------------------------+
| Trusted project · world saved 2 min ago · engine ok · cook queue 0 · git: 3 modified     |(7)
+------------------------------------------------------------------------------------------+
```

1. **Header bar.** Studio itself has no network use, so it shows no `[net]` badge. While a project plays, the running game's own indicators appear next to the Play controls, labelled with the project name.
2. **Toolbar**, 36 px. Gizmo tools (keys `1`–`4`), space toggle, snap, play controls, view mode, viewport layout. Built from the shared action table.
3. **Hierarchy**, 260 px. `treeView` with filter, multi-select, drag to reparent.
4. **Viewport.** `viewportWidget` showing the engine frame; the gizmo and overlays are a vector overlay. A 4 px green frame and a badge show play state.
5. **Inspector**, 320 px. `propertyGrid` with one group per component.
6. **Bottom dock**, 200 px. Tabs; Asset Browser shown with breadcrumb, filter, type filter, grid toggle and status line.
7. **Status bar.** Trust state, save state, renderer state (GPU or software), cook queue, version-control summary.

### 5.3 Other views and dialogs

**Project hub** (960 × 640). States: no projects (empty state), project folder missing (row greyed with "Locate…"), restricted project (shield icon).

```
+--------------------------------- Studio — Projects ---------------------------------------+
| [New project…] [Open folder…] [Import archive…]                      [search________]     |
| Recent                          | Templates                                               |
| > island          3D   today    | [▣ Empty 3D] [▣ Empty 2D] [▣ Hybrid HUD] [▣ First person]|
|   pong            2D   Tuesday  | [▣ Top-down] [▣ Platformer] [▣ Simulation] [▣ Tool app] |
|   crowd-sim       3D   3 Nov  ⛨ | Samples (read-only)                                     |
|   ⚠ old-demo      missing       | [▣ demo_hybrid_hud] [▣ demo_ai_bt] [▣ demo_timeline] …  |
| ⛨ restricted: scripts will not run until you trust the project                            |
+-------------------------------------------------------------------------------------------+
```

**New Project wizard** (640 × 520). States: name invalid, folder not empty.

```
+--------------------------- New project ----------------------------+
| Name      [my_game___________]                                     |
| Location  [~/Projects…]            → creates ~/Projects/my_game    |
| Dimension (•) 3D  ( ) 2D  ( ) Hybrid       [x] UI layer            |
| Modules   [ ] AI  [x] Cinematics  [x] FX  [ ] Mechanics  [ ] Adventure |
| Script    (•) ForgeScript  ( ) Lua  ( ) none                       |
| Template  [Empty 3D v]                                             |
| Capabilities the game will ask for: none  [Edit…]                  |
|                                        [Cancel] [Create]           |
+--------------------------------------------------------------------+
```

**Play mode** (viewport while playing, maximised). States: playing, paused, capability prompt pending (the shell's prompt is over the game view), crashed (overlay with the error and "Stop").

```
+------------------------------------------------------------------------------------------+
| [■ Stop Shift+F5] [⏸ F6] [⏭ step F10]     PLAYING  12.4 s - 744 ticks      [mic] island  |
+==========================================================================================+
‖                                                                                          ‖
‖                                 (game view)                                              ‖
‖                                                                                          ‖
+==========================================================================================+
| Capability log:  fs.app ✓ used · dev.mic ✓ granted once · net.host:example.org:443 ✕ not declared |
+------------------------------------------------------------------------------------------+
```

**Export wizard, capability step** (760 × 560). States: no signing key (step blocked with a link to SE04), linter errors.

```
+------------------------------- Export app bundle (3 of 5) -------------------------------+
| Capabilities                                                                              |
| Declared            Used in play tests     Note                                           |
| fs.app              ✓ yes                  always granted                                 |
| dev.gpu.compute     ✓ yes                                                                 |
| dev.mic             ✓ yes (2 sessions)     user is asked on first use                     |
| net.host:lobby:7777 ✕ never used           ⚠ declared but unused — [Remove]               |
| (not declared)      net.user ✕ denied 3×   a script tried to open a socket — [Show]       |
|                                                                                           |
| The installer will show this list to the user.                                            |
|                                             [Back] [Next: Signing]                        |
+-------------------------------------------------------------------------------------------+
```

**Timeline and curves** (bottom dock, 1600 × 260). States: no sequence (empty state "Create sequence").

```
+------------------------------------------------------------------------------------------+
| [|◀][▶][loop][snap 0.1s] 00:04.20 / 00:30.00   [auto-key ●]            [curves ▾]       |
| Director                |0s      5s       10s      15s      20s      25s      30s        |
|  ▸ MainCam (camera)     |◆────────◆─────────────────◆                                    |
|  ▸ Door (property rot)  |         ◆────◆                                                 |
|  ▸ Music (audio)        |[ theme.fuseaudio ~~~~~~~~~~~~~~~~~~~~~~~~~~ ]                  |
|  ▸ Cues (event)         |    ▼ start      ▼ door_open              ▼ fade                |
+------------------------------------------------------------------------------------------+
```

**Profiler** (bottom dock). States: paused, no samples yet.

```
+------------------------------------------------------------------------------------------+
| [⏸ pause] [clear] [capture…]   frame budget 16.7 ms      view [CPU history v]            |
| 20ms ┤                                ▂                                                  |
| 16.7 ┤─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─█─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ budget  |
| 10ms ┤▂▃▂▂▃▂▃▃▂▂▃▂▂▃▂▃▂▂▃▃▂▂▃▂▂▃▂▃▂▂▃█▃▂▂▃▂▂▃▂▃▂▂▃                                        |
| Frame 744: game 3.1 · jobs 4.0 · render record 2.2 · present 0.9 · scripts 0.6 ms        |
| [flame graph of the selected frame]                                                      |
+------------------------------------------------------------------------------------------+
```

**Simulation batch** (720 × 520): world, ticks, time step, seed list or range, parameter table (`tableView`: name, from, to, steps), recorders (property, every N ticks), output folder; Run sends jobs to the Jobs dock; results open in OF09 or AI04.

**Project trust** (sheet on opening a restricted project): says where the project came from, how many scripts it has and which are signed and by whom, what capabilities it declares; buttons "Keep restricted" (default) and "Trust this project".

### 5.4 Menus

Names follow the upstream action table where one exists.

```
File      New Project… Ctrl+Shift+N · Open Project… Ctrl+Shift+O · Recent Projects ▸
          New World Ctrl+N · Open World Ctrl+O · Save World Ctrl+S · Save World As Ctrl+Shift+S
          Save All Ctrl+Alt+S · Import… Ctrl+I · Export App Bundle… Ctrl+E
          Export Project Archive… · Project Settings… · Close Project · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut/Copy/Paste · Duplicate Ctrl+D · Delete Del
          Select All Ctrl+A · Deselect Esc · Find Entity Ctrl+F · Preferences… Ctrl+,
Entity    Create ▸ · Group Ctrl+G · Ungroup Ctrl+Shift+G · Duplicate Subtree
          Frame Selected F · Make Prefab · Apply/Revert Prefab · Add Component…
          Open Script · Snap to Ground End · Align ▸
View      View Mode ▸ (Lit Alt+1, Unlit Alt+2, Wireframe Alt+3, Lit Wireframe Alt+4)
          Show Mesh Bounds · Debug View ▸ · Viewport Layout ▸ (Single Ctrl+Alt+1,
          Two Ctrl+Alt+2, Quad Ctrl+Alt+3) · Camera ▸ (Perspective, Top, Front, Side,
          Bookmarks) · Grid · Colliders · Navmesh · Statistics · Full Screen F11
Play      Play F5 · Pause F6 · Step F10 · Stop Shift+F5 · Play From Here Alt+F5
          Play as Installed App Ctrl+F5 · Maximize Viewport on Play · Record Inputs
Assets    Import… · Re-import Selected · Cook All · Check Assets · Find Unused
          Licence Report · Open in ▸ (Image Editor, Audio Editor, UI Designer, IDE)
Tools     Terrain · Navigation · SDF Sculpt · Material Editor · Timeline · Sequencer
          Visual Script · Behaviour Tree · Simulation Batch… · Editor Scripts ▸
Window    Layout ▸ · Reset Layout · (each dock) · Jobs · Capability Log · New Inspector
Help      Studio Help F1 · Script API · Samples · Tour · Shortcuts · About
```

### 5.5 Keyboard and pointer

Viewport: `1` select, `2` move, `3` rotate, `4` scale (Verified upstream keys), `X` world/local, `Ctrl` held snaps, RMB + `WASDQE` fly, wheel dolly or fly speed, `F` frame selected, `Alt+LMB` orbit, `MMB` pan, `H` hide, `Alt+H` show all, `G` toggle grid. Play: `F5`, `F6`, `F10`, `Shift+F5`. `Ctrl+P` quick-open any asset or entity (print is not used in this app, so the reserved key is rebound here and listed in Shortcuts).

| Where | Context menu |
|---|---|
| Viewport (entity) | Create ▸, Duplicate, Delete, Group, Frame, Make Prefab, Open Script, Add Component…, Snap to Ground |
| Viewport (empty) | Create ▸, Paste Here, Play From Here, Camera ▸ |
| Hierarchy row | Rename, Duplicate, Duplicate Subtree, Delete, Group, Ungroup, Create Child ▸, Make Prefab, Select Children, Copy Path |
| Asset | Open, Open in ▸, Re-import, Import Settings…, Rename, Copy Path, Copy Relative Path, Show Dependencies, New Folder, Refresh, Show Changes (DV03) |
| Inspector property | Reset, Copy, Paste, Revert Override, Add Key, Copy Script Path |
| Timeline key | Interpolation ▸, Copy, Delete, Jump To |
| Graph node | Duplicate, Delete, Disable, Group, Comment |
| Console line | Copy, Go to Source, Clear |

Drag and drop: files from SY02 into the asset browser (import); asset to viewport (place with surface snap) or to hierarchy (as child); material onto a mesh in the viewport; asset onto an inspector reference field; entity from hierarchy to an inspector entity field or to the asset browser (make prefab); script onto an entity. Out of the app: an asset dragged to SY02 gives the source file. Gestures: pinch zoom and two-finger orbit in the viewport.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No project: the hub. Empty world: viewport grid with "Create your first entity" and the Create button. |
| Loading | Hub row shows progress; the workbench opens with docks filled as the engine reports; assets show placeholders until cooked. |
| Playing / paused | Green frame and badge / amber badge; edits in the inspector are marked temporary. |
| Restricted | Banner "Restricted project: scripts and play are off" with "Review and trust…". Viewing and editing worlds still works. |
| Recovered | After a crash and restart: banner "Restored N edits from the journal" with "Review" (history list) and "Discard". |
| No GPU | Banner "No GPU device: software viewport"; lit view modes disabled with the reason. |
| Error | Cook and script errors go to Problems with "go to"; a failed world load offers the last autosave. |
| Offline (network) | No change for Studio. A game that declared network capabilities is tested only if the user grants them. |
| Locked | Session lock pauses play and blanks the viewport. |
| Read-only | Sample projects and read-only folders: banner and "Copy to my projects". |
| Narrow (< 1100 px) | Right dock becomes a sheet (`F4`); under 640 px only the viewport and one overlay dock at a time. |

### 5.7 Widgets

| Region | FUSE widget | Note |
|---|---|---|
| Workbench | `dockArea`, `dockWidget`, `tabWidget`, `splitter`, `menuBar`, `popupMenu` | Used by the upstream shell today |
| Viewports | `viewportWidget` with overlay | Zero-copy external texture in-process today; cross-process is CR04-GAP-1 |
| Hierarchy | `treeView` | |
| Inspector, panels | `propertyGrid`, `spinBox`, `slider`, `comboBox`, `colorPicker`, `checkBox`, `lineEdit` | |
| Asset browser | `gridView`, `listView`, `treeView` | |
| Console, problems | `modelListView`, `lineEdit` | |
| Timeline, sequencer | `timeline` | Upgrades in FUSE-WP-50 for typed tracks and key lanes |
| Curves | `curveEditor` | One curve today; FUSE-WP-50 |
| Graph editors | `nodeGraph` | Typed pins, links, no cycles today; groups, comments, search, minimap in FUSE-WP-50 |
| Profiler | `chart` | Flame graph is CR04-GAP-2 |
| Script editor | `codeEdit` | Line numbers and highlighting today; FUSE-WP-37 for the rest |
| Help | `markdownView` | |
| Wizards, dialogs | `dialog`, `radioButton`, `toggleSwitch`, `progressBar` | Folder choice goes through the portal, not `fileDialog` |

Gaps:

- **G-01**: the app shell has X11 and Win32 windows only (V5); Studio needs the Wayland backend like every app.
- **CR04-GAP-1**: showing the play instance's frames inside the Studio viewport without a copy. The editor's own viewport needs nothing new: upstream shares the frame inside one process through a `ui_vk` external-texture slot on an adopted device (Verified). The play instance is another process, so its surface must be handed over by the shell or imported as a GPU image. Not in G-01..G-18.
- **CR04-GAP-5**: a native-entry path in the app host: verify the bundle, compile the manifest policy, enter capability mode, then start a signed native binary as the app instead of a ForgeScript engine. Needed by the decision in section 13. Not in G-01..G-18.
- **CR04-GAP-2**: flame-graph widget (rectangles with labels, zoom, virtualised).
- **CR04-GAP-3**: development launch of an unsigned or developer-signed bundle as its own confined app instance, started by the shell on Studio's request. Not in G-01..G-18.
- **CR04-GAP-4**: engine shaders on HardenedBSD. The FUSE build plan notes that the shader compiler it depends on has no vendored source and no known FreeBSD binary (FUSE-WP-02 risks, Q-09 there). Studio's lit viewport depends on the answer.
- 2D viewport can use the G-12 canvas or the engine's own 2D render through `viewportWidget`; the second is preferred (one render path).

## 6. Architecture

### 6.1 Components

Studio is a native app (decision in section 13): the upstream Qt-free shell `fuse_editor_fuse` is the app process. Editor chrome, controller and engine run in that one process, as they do upstream.

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Editor shell | C++23 on FUSE UI (`Source/FUSE/EditorShell/Fuse`) | app process (`org.pbsd.studio`), UI thread | Docks, menus and toolbar from the action table, panels, dialogs, viewport widget and overlays |
| Editor controller and panel models | C++23 (`fuse_editor_controller`, `EditorPanels`) | app process | Qt-free `EditorController`, stable action table, panel models, gizmo, preferences |
| Editor host and engine | C++23 (`fuse_editor_api`, FUSE engine, scene host) | app process, game thread and job workers | `EditorHost`, command drain, undo stack, the edited world, viewport rendering, picking |
| Platform glue | C++23 | app process | Portal file access, `svc.*` clients, intents, trust state, edit journal, system theme and shortcuts |
| Extension engine | ForgeScript in an embedded `forge::script::engine` | app process | Signed editor extensions with a policy narrowed to the editor API |
| `cook` | C++23 (FUSE tools: cook, import, convert, asset check, lint) | helper, one per job, no capabilities | Parses untrusted source assets and writes cooked files to the passed output handle; pre-checks foreign project files |
| `play-runner` | C++23 (`fuse_runtime`) + the project's scripts (ForgeScript; Lua for development) | separate confined app instance | Runs the project in play mode with the project's capabilities only |
| `export` | C++23 | helper | Packs content, writes the bundle, calls the signer, runs the linter |

```
 +---------------- app process: org.pbsd.studio (fuse_editor_fuse, native, confined) ----------------+
 | UI thread: FUSE UI shell ── EditorController, action table, panel models                         |
 |      │ posts EditorCommand (handles only)                ▲ viewport image (zero-copy, same device)|
 |      ▼                                                   │                                        |
 | game thread: EditorHost::gameTick() ── FUSE engine ── scene host ── job workers                   |
 | embedded ForgeScript engine: signed editor extensions (editor API only)                           |
 +------┬--------------------------------┬-------------------------------┬---------------------------+
        │ source + output handles        │ launch request (shell)        │ pack + sign request
        ▼                                ▼                               ▼
   cook (no caps)              play-runner = the project as its     export ── svc.keyring
                               own confined app instance, with
                               its own manifest and prompts
                                         │ frames (surface from the shell), debug channel (opt-in)
                                         └──────────────▶ back to the Studio viewport
```

Verified upstream rules that this keeps: the UI thread only posts `EditorCommand` envelopes; the game thread drains them in `EditorHost::gameTick()`; no raw scene pointers cross, only `fuse::Handle<T>` in payloads (`FUSE:docs/editor.md`). The viewport shows the engine frame zero-copy through a `ui_vk` external-texture slot on the adopted device (`FUSE_EDITOR_QT_FIRST_PLAN.md` §11). Nothing is re-implemented in ForgeScript.

Where Studio departs from PLATFORM.md and where it does not:

| Rule | Studio |
|---|---|
| App logic in ForgeScript (section 2) | **Exception.** Logic and UI are native C++23. Allowed because the whole app is a wrapped existing engine (ground (d)), by owner decision. |
| Shared app host (section 4) | The host verifies the bundle and builds the policy as usual, then starts the native entry instead of a script engine (CR04-GAP-5). |
| Default-deny capabilities, portals, prompts by the shell (section 6) | Unchanged. |
| Signed, read-only bundle (section 3) | Unchanged; the binary is in the tree hash. |
| No JIT, no W+X (section 4) | Unchanged; checked on the binary (CR04-W35). |
| Native code under the safe subset and PRISM (section 2, row 3) | Applies to all of Studio, with the gates the FUSE build plan sets for FUSE code (CR04-W37). |
| ForgeScript for extension | Used for project scripting and editor extensions (CR04-W12, CR04-W32). |

### 6.2 Data model

Project folder (Verified parts marked V; the rest Proposed):

```
<project>/
  project.json                 V  schema 1 (name, dimensions, modules, default worlds)
  worlds/*.fuselevel           V  world files (native target format; conversion upstream is incremental)
  assets/…                        source assets (glTF, PNG, WAV, …) and *.import.json
  scripts/*.fs | *.lua            gameplay scripts (+ .sig for signed ForgeScript)
  ui/*.fuseui, style/*.css        game UI documents (edited in DV02)
  prefabs/*.prefab.json
  materials/*.fusemat          V  cooked material format exists; a text source form is Proposed
  sequences/*.seq.json            cinematics timelines
  graphs/*.graph.json             visual scripts, behaviour trees, material graphs
  app/manifest.json               the game's own manifest draft (capabilities, id, version)
  export/presets.json
  .studio/                        cache: cooked assets, thumbnails, journal, autosave (not versioned)
```

| Entity | Fields |
|---|---|
| `ProjectManifest` (V, `FUSE:docs/projects.md`) | `schemaVersion: 1`, `name: string`, `dimensions: {enable3D, enable2D, enableUI: bool}`, `modules: {ai, cinematics, fx, mechanics, adventure: bool}`, `defaultWorld3D: string`, `defaultWorld2D: string` |
| `StudioProject` (Proposed, in `app/` and `.studio/`) | `trust: enum{trusted,restricted}`, `origin: string`, `script_backend: enum{forgescript,lua,none}`, `declared_caps: [string]`, `granted_caps: [string]`, `layouts`, `recent_worlds` |
| `Entity` | `id: EntityID`, `name: string`, `parent: EntityID?`, `components: [Component]`, `prefab: {asset: AssetId, overrides: [...]}?`, `tags: [string]`, `layer: u8`, editor-only `hidden`, `locked` |
| `Component` | `type: string`, `props: map<string, Value>`; schemas come from the engine's component schema |
| `Asset` | `id: AssetId`, `path: string` (VFS), `kind: enum{mesh,texture,material,audio,script,world,prefab,ui,sequence,graph,pack,unknown}`, `source_blake3: hex`, `import: json`, `cooked: path?`, `licence: string`, `author: string`, `deps: [AssetId]` |
| `EditCommand` (journal) | `seq: u64`, `time: i64`, `type: string`, `payload: json` with handles, `undo_group: u64` |
| `Sequence` | `duration_ms: u32`, `groups: [{name, tracks: [{kind: enum{camera,sprite,property,audio,event}, target, keys: [{t_ms, value, interp}]}]}]` |
| `ExportPreset` | `name`, `bundle_id`, `version`, `variant: enum{debug,release}`, `entry_world`, `quality`, `caps: [string]`, `key_id: string`, `compress: bool` |
| `SimBatch` | `world`, `ticks: u64`, `dt_ms: f64`, `seeds: [u64]`, `params: [{name, from, to, steps}]`, `recorders: [{entity, property, every}]`, `output_token` |

Cooked formats (Verified names): FMSH `.fusemesh`, `.fusetex`, `.fusemat`, `.fuseaudio`, `.fusecol` (`FUSE:docs/modules.md` row `fuse_asset`; `FUSE:Tools/FUSE/fuse_cook.cpp` usage text). The cook tool also has `--project`, `--manifest`, `--dry-run`, `--lenient` and texture block formats BC1, BC4, BC5, BC6H, BC7 (same file).

Schema versions: `project.json` carries `schemaVersion`; every Proposed file carries `schema`. Migration is forward only with the old file copied to `.studio/pre-migration/`.

### 6.3 File formats and protocols

| Format | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `project.json` | yes | yes | `fuse_project` loader (Verified) and writer | app process |
| `.fuselevel` worlds | yes | yes | FUSE | app process; `cook` first for a check of foreign projects |
| glTF 2.0 / GLB, OBJ | yes | glTF export (phase 3) | FUSE importers; which source formats the mesh cooker accepts today is Unverified beyond the cook tool's usage text | `cook` |
| FBX | optional | no | Only if a reader with an acceptable licence is chosen | `cook` |
| PNG, TGA, JPEG, HDR, KTX2 | yes | PNG, EXR (captures) | FUSE cook (Verified list in `fuse_cook.cpp`) | `cook` |
| WAV, FLAC, Ogg | yes | no | FUSE cook (Verified list) | `cook` |
| FMSH, `.fusetex`, `.fusemat`, `.fuseaudio`, `.fusecol` | yes | yes | FUSE asset readers and cookers | `cook` writes; the app process and the runtime read |
| Lua scripts | run | edit | Lua VM (optional backend, Verified) | `play-runner` only; never in the Studio process |
| ForgeScript project scripts | run | edit | FORGE engine embedded as a script backend (Proposed) | `play-runner` |
| ForgeScript editor extensions (signed) | run | no | Embedded `forge::script::engine` (Proposed) | app process, narrowed policy |
| `.fuseui`, CSS | preview | no (DV02 edits) | FUSE UI | app process |
| NeuralVFX `.nvfxp` | yes | no | `libnvfx` (optional) | `cook` (check), runtime (play) |
| CSV, Parquet (simulation output) | no | yes | Own CSV; Parquet writer shared with OF09 | `play-runner` writes to the passed handle |
| App bundle (`.app/`, `SIGNATURES`) | lint | yes | Platform bundle tools (FORGE-WP-09, FORGE-WP-38) | `export` |
| Helper IPC | — | — | Session-bus schemas `org.pbsd.studio.cook.v1` and `.export.v1` (jobs, progress, reports) | both ends validate |
| Debug and profile channel | — | — | FORGE debugger protocol (FORGE-WP-36) plus an engine profile stream | `play-runner` ↔ app process, opt-in per run |

### 6.4 Integration

- **Intents handled:** `open` (a project folder, `project.json`, a `.fuselevel`, a project archive), `new`, `edit`. **Emitted:** `open` and `edit` to DV01 (scripts), DV02 (`.fuseui`), DV03 (project folder), CR01 (textures), CR03 (audio), OF09 and AI04 (simulation data); `share` (bundle or archive).
- **MIME:** edit `application/x-fuse-project+json`, `application/x-fuse-level`; import `model/gltf+json`, `model/gltf-binary`, `model/obj`, image and audio types of 6.3; export `application/x-pbsd-app-bundle`, `application/x-fuse-project-archive`, `text/csv`.
- **Services:** `svc.portal.file`, `svc.settings`, `svc.notify`, `svc.intent`, `svc.keyring` (signing through the key service), `svc.thumbnail` (provides thumbnailers for cooked meshes and materials), `svc.power`, `svc.clipboard`, `svc.backup` (excludes `.studio/`), `svc.search` (optional), `svc.audit` (play-mode capability log is read from it), `svc.a11y`, `svc.update` is not used (SY05 installs bundles).
- **Other apps:** DV01, DV02, DV03 as above; SY05 installs exports; SE04 holds developer keys; GM01's games are Studio projects; CR02 takes captures; AI02 for any models a project ships.

**The FUSE editor today (Verified):**

| Fact | Source |
|---|---|
| The editor is documented as a desktop Qt 6 application; `fuse_editor_api` is the Qt-free boundary (`CommandQueue`, `EditorHost`); `fuse_editor` is the Qt 6 Widgets shell | `FUSE:docs/editor.md` |
| A Qt-free shell builds with `-DFUSE_EDITOR_SHELL=fuse` as `fuse_editor_fuse` on the shared Qt-free `EditorController`; the default shell stays `qt` | `FUSE:docs/ui/README.md` (Editor shell, UI9); `FUSE:docs/plans/FUSE_EDITOR_QT_FIRST_PLAN.md` §11 |
| All ten panels are ported: Hierarchy, Inspector, Console, Project Hub, Asset Browser, Material Editor, SDF Sculpt, Profiler, Sequencer, Timeline; plus the New Project wizard | same |
| Also ported: Navigation and Terrain panels, Preferences with shortcut rebinding, Recent Projects, grouping, collider handles, Open Script, view modes, multi-viewport (single, two, quad), play-mode frame and badge, Maximize on Play, transform gizmo, live zero-copy 3D viewport (`--gpu-viewport`) | same |
| 45 gates under `ctest -L fuse_editor_ui`; panel models are unit-tested without UI (`fuse_editor_panel_models`) | same |
| Chrome is built from a stable action table; three docks with tabs, a viewport, a status bar | `FUSE_EDITOR_QT_FIRST_PLAN.md` §11 |
| Controller, shell and scene host live in `Source/FUSE/EditorShell/{Controller,Fuse,SceneHost}` | repo tree |
| Project format: a directory with `project.json` schema 1; loader `fuse_project` | `FUSE:docs/projects.md` |
| Worlds are `*.fuselevel`; importers register placeholder worlds; conversion is incremental | `FUSE:docs/projects.md` |
| Tools: `fuse_cook`, `fuse_import`, `fuse_convert`, plus `AssetCheck`, `AssetGen`, `Cook`, `Lint`, `Relight`, `WorldExtract` directories | `FUSE:Tools/FUSE/` |
| Scripting: a game-thread `ScriptHost`; `ScriptVM` has a Null backend and an optional Lua backend; hot reload and a per-entity script component are not in the current milestone; a headless `ScriptConsole` | `FUSE:docs/scripting.md` |
| Engine layers L0–L4; modules `fuse_ai`, `fuse_fx`, `fuse_cinematics`, `fuse_mechanics`, `fuse_adventure`; libraries for VFX (CPU particles), terrain (heightfield, LOD grid), world partition, audio, ECS, physics, net (stubs), animation, assets, navigation (grid navmesh with A*), compute | `FUSE:docs/architecture.md`, `FUSE:docs/modules.md` |
| Roadmap status: in-viewport gizmos, native world files and Lua as the default shipped VM are listed as "still scaffolding / later" in the roadmap, while the editor plan reports a working gizmo in both shells. The two documents differ; the editor plan is newer in detail. | `FUSE:docs/roadmap.md`, `FUSE_EDITOR_QT_FIRST_PLAN.md` §11 |
| The UI app shell has X11 and Win32 native windows; no Wayland backend | PLATFORM.md V5 |

**What remains Qt-only (Verified unless marked):**

| Item | Source |
|---|---|
| The default build and the documented product are the Qt shell; roadmap Track A states "Editor chrome is Qt 6" and gate U6 is "Qt editor shell, in progress" | `FUSE:docs/editor.md`, `FUSE:docs/roadmap.md` |
| The Qt embedded Vulkan child-window viewport (`ViewportVulkanWindow`, `QVulkanInstance`) and its live-present gate `fuse_editor_qt_live_present` | `FUSE:docs/editor.md` |
| The Qt widgets have not moved onto the shared `EditorPanels` models ("can move later; the Qt shell is unchanged") | `FUSE_EDITOR_QT_FIRST_PLAN.md` §11 |
| Qt-specific sources under `Source/FUSE/Editor/src/qt` and `tests/qt` | repo tree |
| Whether any panel or command exists only in the Qt shell | Unverified: the plan claims parity item by item; the two shells were not compared line by line here |

**NeuralVFX (Verified):** `NeuralVFX:engines/fuse/` is a sample integration written against a stand-in for FUSE ("There is no FUSE source in this repository", `fuse_engine.hpp`). It defines the interface FUSE would implement (frame clock, job system, view, texture upload, render hooks) and an adapter that plays an effect pack as sprites. Studio's effect-library import (feature F060) is Proposed on top of that.

**FORGE (Verified):** `FORGE:examples/integrations/fuse_runtime.cpp` is a 22-line sample using `forge::task::pool` and SIMD dispatch to blend rows; its comment names task, memory and profiling as runtime services for the engine. It is not an integration with FUSE's source.

## 7. Security and privacy

Studio is one native process (decision in section 13). It is confined exactly like a ForgeScript app: the app host verifies the bundle signature, compiles the manifest to a policy, and starts the binary already in Capsicum capability mode with only the handles the manifest allows. The difference is inside the process: there is no memory-safe interpreter between hostile data and the app's handles, so the safe-subset and PRISM gates of section 11 and the restricted-project rules below carry that weight.

Capabilities requested by Studio itself:

| Capability | Reason |
|---|---|
| `fs.app` | Settings, templates, hub list, caches |
| `fs.portal` | Project folders (remembered grants), imports, export destinations |
| `net.none` | Studio has no network use |
| `proc.helper:cook` | Sandboxed import and cook |
| `proc.helper:export` | Bundle writing |
| `dev.gpu.compute` | Viewport rendering and compute kernels, in the app process |
| `svc.intent` | Hand-offs, and the request to the shell to launch a project as a development app |
| `svc.keyring` | Ask the key service to sign an export; Studio never sees the private key |
| `svc.audit` | Read the capability log of the project instance Studio launched (only that instance) |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.thumbnail`, `svc.power`, `svc.clipboard`, `svc.backup`, `svc.search`, `svc.a11y` | As listed in 6.4 |

Not requested: `proc.exec`, any `net.*` beyond `net.none`, `dev.camera`, `dev.mic`, `dev.screen`, `dev.location`, `dev.usb`, `bg.*`, `ai.local`. A game that needs a microphone or a socket gets it under its own identity, not Studio's.

**Sandboxing of running projects.** Play mode never runs project code with Studio's authority.

1. The project declares capabilities in `app/manifest.json`, in the same vocabulary as PLATFORM.md section 6.
2. Play starts `play-runner` as a **separate confined app instance of the project**, launched by the shell at Studio's request (CR04-GAP-3). Its policy is compiled from the project's manifest by the same manifest-to-policy compiler as any app (FORGE-WP-08). Rule 2 of PLATFORM.md section 6 ("a helper never gets more than its app") is why this is not a plain helper: Studio holds no network or microphone grant to pass down.
3. Prompts for the project's capabilities are drawn by the shell and name the project, marked "development build".
4. The play instance gets read access to the cooked content and scripts of the project and write access to its own `fs.app` area only. It cannot write the project sources.
5. Its frames come back to the Studio viewport as a surface handed over by the shell; input is forwarded while the viewport has focus.
6. Everything it uses or is denied lands in `svc.audit`; Studio shows that as the capability log and reuses it in the export review.
7. A restricted project cannot play at all. Project scripts never run inside the Studio process: editor-time script callbacks are off on this OS.
8. In-process play (upstream's behaviour) is kept only for trusted projects with no scripts, as a fast path. It is a setting, default off.

**Assets:** project sources and unreleased content, the developer's signing key, the user's other files, the user's trust in exported bundles.

**Adversaries:** a downloaded project or template (scripts, crafted assets, crafted world files, a manifest that asks for too much); a crafted asset file; a hostile editor extension; a compromised dependency inside a project; a game under test that tries to reach beyond its declaration.

**What a compromise of the Studio process could reach:** the project folders the user granted (read and write), the GPU, Studio's own data, a request to the key service, a request to launch a development app. It could not reach other files, the network, devices or other apps.

| Surface | Mitigation |
|---|---|
| Source asset parsers (meshes, images, audio) | Only in `cook`: no capabilities, one job per process, time and memory limits. The app process reads only cooked formats written by `cook`. |
| World, prefab and project files from elsewhere | Parsed in the app process, so this is the main surface. Projects from elsewhere open Restricted; their files are first checked in `cook` (structure, sizes, references) before the app loads them; the loaders are T2 targets and fuzzed. |
| Memory-safety bugs in native editor and engine code | Safe-subset check and PRISM gates before release (CR04-W37); handles instead of raw pointers across threads (Verified upstream rule); W^X, ASLR and the hardened allocator of the OS; capability mode limits the damage. |
| Signing misuse by a compromised Studio | The key service shows its own confirmation with the bundle id and hash for every signature; Studio cannot sign silently. |
| Project scripts | Run only in the play instance under the project's policy. ForgeScript must be signed for export; bytecode only, no native code (Verified: `FORGE:script/src/runtime.cpp:729-730`, `script/src/vm.cpp:220-229`). Lua projects are allowed for development; export of Lua projects needs an owner decision (13.4). |
| Editor extensions | Signed ForgeScript only, run by an embedded FORGE engine in the app with a policy narrowed to the editor API: no `fs`, `net`, `proc` or `env`. |
| Native plugins | None. The engine's quarantined compatibility script VMs (`fuse_compat_ts*`) are never built into Studio or exports; upstream forces them off in shipping builds (Verified, `FUSE:docs/modules.md`). |
| Export | Capability review step; linter; signature by the key service; reproducible manifest with hashes. An exported bundle contains no editor code. |
| GPU | Engine shaders come from the signed Studio bundle or the project's cooked materials; user shader source (material graphs) is compiled inside `cook`. |
| Samples and templates | Shipped signed and read-only. |

**Stored:** projects where the user keeps them; hub list as portal tokens; no personal data in exports unless typed. **Leaves the machine:** nothing from Studio. A game under test may use the network only if it declared it and the user granted it.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `general.open_last_project` | bool | `true` | Skip the hub when a project was open |
| `general.restore_layout` | bool | `true` | Restore docks per project (upstream preference) |
| `general.ui_scale` | float | `1.0` | 1.0 to 2.0 |
| `viewport.camera_speed` | float | upstream default | Fly speed (upstream preference) |
| `viewport.look_sensitivity` | float | upstream default | Mouse look (upstream preference) |
| `viewport.preview_sun` | bool | `true` | Use the preview sun (upstream preference) |
| `viewport.orbit_style` | enum | `turntable` | turntable, trackball |
| `viewport.gpu` | enum | `auto` | auto, on, software |
| `viewport.render_when_idle` | bool | `false` | Keep rendering with no change |
| `snap.grid` | float | `0.5` | Metres |
| `snap.angle` | float | `15` | Degrees |
| `play.maximize_on_play` | bool | `false` | Hide docks on Play (upstream key) |
| `play.sandbox` | enum | `separate_instance` | separate_instance, in_engine (trusted, no-capability scripts only) |
| `play.keep_grants_between_runs` | bool | `true` | Remember "this session" grants while the project is open |
| `play.debug_channel` | bool | `true` | Attach profiler and debugger to play runs |
| `assets.auto_reimport` | bool | `true` | Watch and re-cook |
| `assets.cook_jobs` | int | `0` | Parallel cook helpers; 0 = cores minus two |
| `assets.default_texture_format` | enum | `BC7` | BC1, BC4, BC5, BC6H, BC7 |
| `scripts.backend_default` | enum | `forgescript` | forgescript, lua, none |
| `scripts.external_editor` | enum | `ide` | built_in, ide |
| `project.trust_new_from_templates` | bool | `true` | Projects made from shipped templates start trusted |
| `autosave.snapshot_minutes` | int | `2` | Snapshot interval |
| `export.default_variant` | enum | `release` | debug, release |
| `export.key_id` | string | empty | Keyring item for signing |
| `vcs.show_status` | bool | `true` | File status badges |
| `sim.default_dt_ms` | float | `16.6667` | Fixed step for simulation runs |
| `shortcut.<stableId>` | string | from the action table | Per-action binding; `""` = unbound (upstream scheme) |

## 9. Accessibility and languages

- The hierarchy, inspector, asset browser, console and timeline are ordinary trees, grids and lists with full a11y roles. The viewport exposes a text summary: selected entities, camera position and facing, play state.
- **Keyboard-only world editing.** Select through the hierarchy; move, rotate and scale with numeric fields or with arrow keys in the active gizmo mode (step = snap value); "Place at camera" and "Snap to ground" commands.
- A list view of "entities in view" with distance and bearing lets a screen-reader user navigate a scene.
- Play-mode state uses a frame, a badge and words (PLAYING, PAUSED), not colour alone.
- UI scale is tested at 1, 1.25, 1.5 and 2 upstream (Verified gates `font_1` … `font_2`); this spec extends the target to 3.
- Studio offers an "accessibility check" for the project's own UI documents by handing them to DV02.
- All strings in tables; entity and asset names accept any script (G-03).

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to hub | ≤ 400 ms |
| Open a project to first viewport frame | ≤ 2 s for a 10 000-entity world (cooked cache warm) |
| UI frame while hovering | ≤ 16.7 ms (upstream reports 1.5 ms on Linux for its hover-frame gate; not measured on this OS) |
| Idle | 0 UI frames; the engine renders no frame when nothing changes (upstream behaviour) |
| Viewport frame, edit mode | ≤ 16.7 ms on the reference GPU for the reference world |
| Viewport frame hand-off | Editor: 0 copies (same process, same device). Play instance: 0 copies with CR04-GAP-1; ≤ 2 ms at 1080p with the shared-memory fallback |
| Edit command applied (UI post → game-thread drain → panel update) | ≤ 1 frame |
| Hierarchy | 100 000 entities virtualised; filter results in ≤ 100 ms |
| Import and cook of a 100 MiB glTF | Progress within 200 ms; UI never blocks |
| Play start | ≤ 1.5 s from key press to first game frame |
| Restart after a crash to restored world | ≤ 5 s for the reference world |
| Memory | ≤ 400 MiB with an empty project; beyond that it depends on the world |
| Export of a 1 GiB project | ≤ 2 min when the cook cache is warm |

## 11. Testing and proof

Nothing below has run for this app. Upstream gates are cited as existing, not as results obtained here.

- **Upstream gates, used directly:** the 45 `fuse_editor_ui` gates and the `fuse_editor_panel_models` and `fuse_editor_controller_api` suites (Verified to exist) test the very binary Studio ships. They must first run on HardenedBSD (G-18).
- **Confinement tests:** start Studio under its manifest policy and check that it opens no path, socket or device outside its handles; every file action goes through the portal; no writable-executable mapping exists at any time.
- **Unit (C++):** hub list, trust state machine, journal and replay, export preset handling, capability diff (declared against used).
- **Golden renders:** each dock and dialog at scale 1, 1.5, 2 in the system themes; viewport overlays (gizmo, bounds, navmesh).
- **Scripted UI through the a11y tree:** the 14 top tasks, including Play and Stop restoring the world byte for byte.
- **Fuzz:** every importer in `cook`; the `.fuselevel` and `project.json` loaders (these also run in the app process, so they get the longest fuzz budget); prefab, sequence and graph JSON; the play-instance debug channel decoder.
- **Sandbox tests:** a test project whose scripts try to read outside `fs.app`, open sockets, start processes and use devices without declaring them; each attempt must be denied and logged. A crafted asset set must not escape `cook`.
- **Crash tests:** kill Studio during edits and during save; on restart no edit older than the last journal flush is lost.
- **Export tests:** the bundle passes the linter, verifies its signatures, runs with only declared capabilities, and two exports of the same inputs are byte-identical.
- **Determinism:** a simulation run with single-thread jobs and a fixed seed gives identical recorder output twice.

Because Studio is a native app, its own code carries the proof burden that ForgeScript would otherwise remove. All Studio and FUSE code in the app process must pass the MathScript safe-subset check and the PRISM gates that the FUSE build plan sets for FUSE code before a release is signed.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Editor shell and controller (`EditorShell/Fuse`, `Controller`) | Safe subset; no runtime errors in command handling | T3, T2 for the command queue and undo stack | Subset check, PRISM job, sanitizers, the UI gates | Size of the shell; UI code is hard to specify |
| `EditorHost` command drain | Every command is applied once, in order, on the game thread | T2 | PRISM on the queue; model check of the post/drain protocol | Whole-engine concurrency |
| `.fuselevel` and `project.json` loaders (in the app process) | No out-of-bounds read or unchecked index on any input | T2 | PRISM plus fuzzing | — |
| Edit journal and replay | Replay reproduces the same world hash | T3 | Property tests | Engine state is too large for proof now |
| Manifest draft → policy for play runs | Policy never exceeds the manifest | T1 | Covered by the platform compiler's own proof target (FORGE-WP-08) | Depends on that work |
| Capability diff (declared vs used) | Sound: never reports unused when used | T2 | PRISM on the small pure module | — |
| `cook` parsers | Contained; own parsers free of out-of-bounds reads | T2 for own code, T4 for third-party readers | PRISM, fuzzing, sandbox tests | Third-party code outside the subset |
| Handle maps (`fuse::Handle<T>`, generation checks) | No use of a stale handle | T2 | PRISM on the handle module | Upstream code must first pass the safe-subset check (Unverified) |
| Navigation grid A* | Path is shortest in steps; deterministic | T1 | Proof on the small pure module | — |
| Engine at large (renderer, physics, jobs) | Safe subset; no sanitizer findings on the gate suite | T3 | Subset check, ASan/UBSan runs (upstream sets this bar in its roadmap) | Size and concurrency |
| Embedded ForgeScript engine (extensions) and its host bindings | Bindings validate every argument; scripts cannot reach beyond the editor API | T3, T2 for the binding layer | PRISM on bindings; language guarantees | FORGE's own tiers |
| Lua VM | Contained in the play instance; never in the Studio process | T4 | Sandbox only | C code outside the subset |
| Bundle writer | Output matches the bundle schema; signatures verify | T2 | PRISM plus round-trip tests | — |

## 12. Build plan

IDs are stable. CR04-W03, W05 and W06 (a `studio-engine` helper and a ForgeScript re-implementation of the editor chrome) are retired by the decision in section 13 and are not reused. New items start at W34.

### Phase 1 — the upstream shell, bundled and confined as an app on this OS

| ID | Work item | Depends on |
|---|---|---|
| CR04-W01 | Build FUSE engine, editor API, controller, scene host and `fuse_editor_fuse` on HardenedBSD (clang, libc++), offline | G-18, FUSE-WP-01, FUSE-WP-02, FUSE-WP-03 |
| CR04-W02 | Engine shaders and Vulkan path on HardenedBSD; decide the shader compiler question | CR04-GAP-4, FUSE-WP-24 |
| CR04-W34 | Bundle: package `fuse_editor_fuse` as `org.pbsd.studio` with a native entry, manifest, signed files; app host starts it under the manifest policy | CR04-GAP-5, G-07, FORGE-WP-08, FORGE-WP-09, FORGE-WP-10 |
| CR04-W35 | Confinement: run in Capsicum capability mode from start; replace direct file access and the built-in `fileDialog` with portal handles; settings through `svc.settings`; no ambient paths; W^X and no-JIT audit of the binary | G-06, G-09, FORGE-WP-04, FUSE-WP-03, W34 |
| CR04-W36 | Platform integration: Wayland window, system theme tokens and header bar, command palette and shortcut settings fed from the action table, a11y bridge, notifications, intents | G-01, G-04, FUSE-WP-05, FUSE-WP-21, FUSE-WP-29, FUSE-WP-30, W34 |
| CR04-W37 | Safe-subset and PRISM gates for the editor shell, controller, scene host and the engine libraries Studio links, as the FUSE build plan sets for FUSE code; record tiers per module | FUSE-WP-04, PRISM, MathScript plugin |
| CR04-W07 | Run the upstream UI gates (`fuse_editor_ui`, panel models, controller) on this OS against the shipped binary; add gates for portal file access | W01, W35 |
| CR04-W08 | `cook` helper around the FUSE tools; import by drag and drop; placeholders on failure | G-06, FORGE-WP-27, FORGE-WP-28, W01 |
| CR04-W09 | Edit journal, autosave, restore after a crash of the app | W34 |
| CR04-W10 | Project trust states and restricted mode; pre-check of foreign worlds in `cook` | W08, W34 |
| CR04-W11 | In-process play for trusted, script-free projects (interim, upstream behaviour) | W34 |

### Phase 2 — the complete editor

| ID | Work item | Depends on |
|---|---|---|
| CR04-W12 | ForgeScript as the project scripting language: a `ScriptVM` backend, engine API bindings, signed scripts | FORGE-WP-10, FORGE-WP-12, FORGE-WP-14, FORGE-WP-33 |
| CR04-W13 | Sandboxed play as a separate confined instance; capability log dock | CR04-GAP-3, FORGE-WP-08, G-09 (`svc.audit`), W12 |
| CR04-W04 | Play-instance frames in the Studio viewport: surface handed over by the shell, shared-memory path first, then zero-copy; input forwarding | CR04-GAP-1, W13 |
| CR04-W14 | Export wizard, bundle writer, signing through the key service, linter, install hand-off | FORGE-WP-09, FORGE-WP-38, SE04, SY05, W13 |
| CR04-W15 | Prefabs, layers, tags, multi-edit, typed property editors | W34 |
| CR04-W16 | Import settings, auto re-import, dependencies, provenance, thumbnails | W08 |
| CR04-W17 | 2D viewport and tools; hybrid view | W34 |
| CR04-W18 | Timeline tracks, auto-key, curve dock; skeletal and sprite animation views | FUSE-WP-50 |
| CR04-W19 | Physics set-up and debug views; audio sources; particle editor | W34 |
| CR04-W20 | Quick script editor, Problems dock, API reference; DV01 hand-off | FUSE-WP-37, DV01 |
| CR04-W21 | DV02 and DV03 hand-offs; status badges; text-friendly world, prefab and material files | DV02, DV03 |
| CR04-W22 | Templates, samples browser, project archive, health check | W10 |
| CR04-W23 | Profiler detail: flame graph, GPU passes, memory, capture | CR04-GAP-2 |
| CR04-W24 | Lighting, GI and post panels; terrain brushes and heightmap I/O | W34 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CR04-W25 | Graph editors: material graph, visual scripting, behaviour trees, animation states, effect composer | FUSE-WP-50, W12 |
| CR04-W26 | Script debugger and hot reload | FORGE-WP-36, W12 |
| CR04-W27 | Simulation: deterministic mode, headless runs, sweeps, recorders, plots, checkpoints | W13, OF09, AI04 |
| CR04-W28 | Reproducible export, SBOM, headless build command | FORGE-WP-34, W14 |
| CR04-W29 | World partition tools, foliage, terrain painting, navigation volumes | W24 |
| CR04-W30 | Cinematic render to image sequence; CR02 hand-off | W18, CR02 |
| CR04-W31 | NeuralVFX effect packs as assets | NeuralVFX port (Unverified), W16 |
| CR04-W32 | ForgeScript as the editor extension language: an embedded engine in the app, the editor API as host bindings, signed tools with a narrowed policy | W12, FORGE-WP-10 |
| CR04-W33 | Input record and replay; joints; quality presets; environment | W19 |

## 13. Open questions and risks

**Decided (owner, 12 Nov 2026):** Studio ships FUSE's existing Qt-free C++ editor shell (`fuse_editor_fuse` on `EditorController`) directly as the app. There is no ForgeScript front end and no `studio-engine` helper. Reason: functionality and simplicity; the shell already has the panels and gates. Studio is a documented native-app exception to PLATFORM.md section 2 (section 2 of this spec). Retired by this decision: work items CR04-W03, W05 and W06, and the earlier open question about process-split cost.

1. **Exception must stay narrow.** Studio's own UI and logic are native C++. Every other rule still applies: signed bundle, manifest-derived confinement, no JIT, no W+X, portals for files, safe-subset and PRISM gates. The platform needs a native-entry path in the app host for this (CR04-GAP-5). No other app should use it without its own owner decision.
2. **One process holds the editor and the engine.** A memory-safety bug in the engine while it loads a hostile world is a bug in the whole app. Mitigations are in section 7 (restricted projects, pre-check in `cook`, capability mode, gates). There is no automatic engine restart; a crash closes the window and the journal restores the work.
3. **Development launch (CR04-GAP-3)** is new platform work: the shell must run an unsigned or developer-signed bundle as a confined app, clearly marked, for the developer only. Without it, sandboxed play cannot honour "the project's declared capabilities".
4. **Lua.** It is the only real script backend upstream today and is C code. For development it runs confined in the play instance. Whether exported bundles may contain a Lua VM and unsigned Lua needs a decision. Proposal: ForgeScript only for export on this OS.
5. **ForgeScript as the project scripting and extension language** is Proposed. Per-frame script cost without JIT may be too high for heavy game logic; the upstream rule "script is glue, not the kernel" helps, and heavy logic belongs in engine modules.
6. **Engine on HardenedBSD** is Unverified end to end: Vulkan drivers, the shader toolchain (CR04-GAP-4), fibers under hardening; CUDA is not expected. The software viewport fallback exists upstream.
7. **Games as app bundles.** PLATFORM.md section 3 expects `main.fs` plus helpers. A game is mostly a native runtime plus content. The bundle format must allow a large signed content pack and a runtime per game, or a shared system runtime. Decide with FORGE-WP-09 and CR04-GAP-5.
8. **Upstream documents disagree** on editor status (roadmap against editor plan). Before planning from the feature table, run the upstream gates and record which pass on this OS.
9. **Scope.** The feature table describes a complete engine editor. Many rows marked N depend on engine features that are scaffolding upstream (physics detail, animation, net). Editor work must follow the engine, not lead it.
10. **Safe-subset distance.** How much of the editor shell, controller and engine already passes MathScript's subset check and PRISM is Unverified. The gate work (CR04-W37) may be large; it decides when Studio can ship.
11. **Look and behaviour.** The upstream shell has its own theme tokens and builds chrome from C++, not `.fuseui`. It must adopt the system tokens, header bar, command palette and shortcut settings (CR04-W36) or it will not match the suite.
12. **Third-party asset formats.** FBX has no open specification; glTF is the supported interchange format. Which source formats the mesh importer accepts today was not established here.
