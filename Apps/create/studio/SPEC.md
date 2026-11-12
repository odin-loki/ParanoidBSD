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
| Status | Planned. Nothing built as an app for this OS. The FUSE editor exists upstream (see 6.4). |

Labels: **Verified** = read in a repo at the cited path. **Proposed** = design in this plan. **Unverified** = could not be read.

## 1. Purpose

Studio is the FUSE editor shipped as an app: a workbench for building 2D, 3D and hybrid worlds, simulations and games, and for exporting them as signed app bundles for this OS. It is for game developers, simulation authors, teachers and hobbyists. It replaces the Qt 6 FUSE editor on this OS and stands where Godot, Unity or Unreal editors would.

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
| T14 | Recover after a crash of the engine process | 0 (automatic) |

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
| CR04-F011 | Multiple projects | One window per project; each has its own engine process. | N | S | 2 |

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
| CR04-F140 | Engine crash recovery | The engine runs in a helper. If it crashes, Studio keeps its window, restarts the engine and replays the edit journal. | N | M | 1 |
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
7. **Status bar.** Trust state, save state, engine process state, cook queue, version-control summary.

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
| Engine restarting | Viewport shows the last frame dimmed with "Engine restarting…"; docks stay; edits are queued and replayed. |
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
- **CR04-GAP-1**: sharing the engine's rendered frame from the engine helper to the app's UI context without a copy. Upstream does this inside one process through a `ui_vk` external-texture slot on an adopted device (Verified). Across processes it needs an exportable GPU image and a compositor or UI-side import. Not in G-01..G-18.
- **CR04-GAP-2**: flame-graph widget (rectangles with labels, zoom, virtualised).
- **CR04-GAP-3**: development launch of an unsigned or developer-signed bundle as its own confined app instance, started by the shell on Studio's request. Not in G-01..G-18.
- **CR04-GAP-4**: engine shaders on HardenedBSD. The FUSE build plan notes that the shader compiler it depends on has no vendored source and no known FreeBSD binary (FUSE-WP-02 risks, Q-09 there). Studio's lit viewport depends on the answer.
- 2D viewport can use the G-12 canvas or the engine's own 2D render through `viewportWidget`; the second is preferred (one render path).
