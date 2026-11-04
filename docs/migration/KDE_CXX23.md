# KDE / Plasma C++23 Migration (Wave 3)

Conversion of `kde/` sources into C++23 modules under `pbsd/kde/` and
Capsicum-aware theme assets under `pbsd/theme/`. Inventory assigns all
`kde/**` paths to wave3.

## Progress (Wave 3 — 2026-07-19, MAX throughput pass 4)

| Milestone | Status | Artifact |
|-----------|--------|----------|
| Aero Plasma theme package (colors, SVGs, plasmarc, panel/) | **Done** | `pbsd/theme/plasma/` (v0.6.0) |
| Theme color manifest `theme-colors.json` | **Done** | 46 new SVG assets + expanded manifest |
| System color scheme `PBSDAero.colors` | **Done** | `pbsd/theme/plasma/color-schemes/` |
| KWin effect package + Aero shaders | **Done** | `pbsd/theme/kwin-effects/pbsd_aero_blur/` |
| KWin decoration bridge module | **Done** | `pbsd/kde/kwin.decorations.cppm` |
| C++23 module `pbsd.kde.plasma.aero` | **Done** | 34 new asset path constants |
| KWin logging partitions (13 + common) | **Done** | `pbsd/kde/kwin/*.logging.cppm` (+ scene/core/backends) |
| KWin core/plugin/wayland modules | **Done** | 31 modules under `pbsd/kde/kwin/` |
| KF6 hand ports | **Done** | `pbsd/kde/frameworks/*.cppm` (91 modules) |
| Plasma modules | **Done** | 30 modules (`plasma.*.cppm`) |
| Batch port stubs (500 cumulative wave3) | **Done** | `pbsd/ports/wave3/kde/` |
| CMake targets | **Done** | auto-regenerated via `tools/update_kde_cmake.py` |
| Conversion log | **Done** | `docs/migration/CONVERTED.md` |

## Progress (Wave 3 — 2026-07-19, MAX throughput pass 3)

| Milestone | Status | Artifact |
|-----------|--------|----------|
| Aero Plasma theme package (colors, SVGs, plasmarc, panel/) | **Done** | `pbsd/theme/plasma/` (v0.5.0) |
| Theme color manifest `theme-colors.json` | **Done** | Start menu + panel glass asset manifest |
| System color scheme `PBSDAero.colors` | **Done** | `pbsd/theme/plasma/color-schemes/` |
| KWin effect package + Aero shaders | **Done** | `pbsd/theme/kwin-effects/pbsd_aero_blur/` |
| KWin decoration bridge module | **Done** | `pbsd/kde/kwin.decorations.cppm` (+ start_menu/decoration blur) |
| C++23 module `pbsd.kde.plasma.aero` | **Done** | 11 new Start/panel asset paths |
| C++23 module `pbsd.kde.kwin.pbsd_aero` | **Done** | `params_for_start_menu()`, `params_for_decoration()` |
| KWin logging partitions (9 + common) | **Done** | `pbsd/kde/kwin/*.logging.cppm` |
| KWin effect factory modules | **Done** | `pbsd/kde/kwin/effects/` (47 modules) |
| KWin core bridge modules | **Done** | plugin, graphicsbufferallocator, textinput, vsyncmonitor, rootitem, screencast |
| KF6 hand ports | **Done** | `pbsd/kde/frameworks/*.cppm` (81 modules) |
| Plasma modules | **Done** | 15 modules (`plasma.*.cppm`) |
| Batch port stubs (163 cumulative wave3) | **Done** | `pbsd/ports/wave3/kde/` |
| CMake targets | **Done** | `pbsd_kde_kwin_effects` + updated install rules |
| Conversion log | **Done** | `docs/migration/CONVERTED.md` |

## Scope

| Upstream tree | PBSD target | Approach |
|---------------|-------------|----------|
| `kde/plasma-framework/` | `pbsd.kde.plasma.*` | Module partitions per framework tier |
| `kde/plasma-desktop/` | `pbsd.kde.plasma.aero` | Theme + shell layout hooks |
| `kde/plasma-workspace/` | `pbsd.kde.plasma.workspace` | Panel/shell C++23 façade |
| `kde/kwin/` | `pbsd.kde.kwin.pbsd_aero` | Blur effect + decoration bridge |
| `kde/frameworks/` (KF6) | `pbsd.kde.frameworks.*` | Import std modules; `-fno-exceptions` where linked to nucleus |

## Conversion strategy

### Phase A — Theme package (complete)

- Ship data-only Plasma theme (`pbsd/theme/plasma/`) via CMake install.
- C++23 stubs (`plasma.aero.cppm`, `kwin.pbsd_aero.cppm`) expose tint/blur
  constants to hosted tools without pulling Qt headers into nucleus builds.
- Widget/dialog/decoration/panel SVGs, `plasmarc`, `theme-colors.json`, and
  `PBSDAero.colors` installed.

### Phase B — KWin effects + decorations (Wave 3 scaffold complete)

1. Port `kwin-effects/pbsd_aero_blur` with metadata, config JSON, shader stubs.
2. `PbsdAeroBlurEffect` reads `pbsd_aero_blur.json` glass tint / saturation.
3. `kwin.decorations.cppm` bridges KDecoration3 asset paths + metrics.
4. Optional `PBSD_BUILD_KWIN_PLUGINS=ON` builds plugin when KWin/KF6 present.
5. Wire `pbsd.compositor` glass parameters as fallback when KWin is absent (Wave 8).

### Phase C — Frameworks (Wave 3 throughput)

Convert bottom-up:

1. **KCoreAddons procstat** — `pbsd.kde.frameworks.kcoreaddons.procstat` (**done**, FreeBSD)
2. **KCoreAddons runtimeplatform** — `pbsd.kde.frameworks.kcoreaddons.runtimeplatform` (**done**, PLASMA_PLATFORM)
3. **KCoreAddons processlist / klistopenfiles** — procstat façade + lsof PID parse (**done**)
4. **KCoreAddons util** — kshell, kjsonutils, kfileutils, kbackup, krandom, klibexec, ksandbox, ksignalhandler, kstaticplugin, kmacroexpander, ksdclock, knetworkmounts, filesystemtype (**done**)
5. **KIO metadata keys** — `pbsd.kde.frameworks.kio.metadata` (**done**)
6. **KConfig** — gui_static, types, kconfigbase, ksharedconfig, kreadconfig, kwriteconfig (**done**, hosted)
7. **KWin logging categories** — `pbsd.kde.kwin.*.logging` (**done**, 9 partitions + common)
8. **Plasma Desktop logging** — `pbsd.kde.plasma.desktop.logging` (**done**)
9. **Batch stubs** — 112 smallest `kde/**` TUs under `pbsd/ports/wave3/` (**done**)
10. **KConfig hand ports** — kdesktopfileaction, kconfigskeleton, kconfigwatcher, kemailsettings, kauthorized, … (**done**, 11 modules)
11. **KCoreAddons jobs/plugin** — kcompositejob, kjob, kpluginfactory, kformat, kstringhandler, … (**done**, 12 modules)
12. **Plasma start menu** — `pbsd.kde.plasma.startmenu` + `panel/start-menu.md` (**done**)
13. **KWin blur bridge** — blur, compositing, effects.registry (**done**)
14. **KWindowSystem / KWayland** — Wayland protocol wrappers; keep FFI boundary thin.
15. **PlasmaFramework** — applet API as modules; QML remains `.qml` until Wave 9.
16. **PlasmaDesktop / Workspace** — shell binaries import `pbsd.kde.plasma.aero`.

### Phase D — Purge

Delete converted `.cpp` rows from `c_inventory.csv`; run `wave_purge_c_check.py`.

## Module naming

```
pbsd.kde.plasma.aero                        — theme constants + install paths
pbsd.kde.plasma.desktop.logging             — Plasma Desktop KCM log category
pbsd.kde.kwin.pbsd_aero                     — blur/decoration bridge to KWin
pbsd.kde.kwin.decorations                   — KDecoration3 asset paths + metrics
pbsd.kde.kwin.logging.common                — shared log category descriptors
pbsd.kde.kwin.{effect,drm,libinput,...}.logging — Q_LOGGING_CATEGORY mirrors
pbsd.kde.frameworks.kcoreaddons.procstat      — FreeBSD process list (no Qt)
pbsd.kde.frameworks.kcoreaddons.{random,kshell,kjsonutils,...} — util partitions
pbsd.kde.frameworks.kio.metadata            — KIO metadata key constants
pbsd.kde.plasma.startmenu                     — Start menu layout + asset paths
pbsd.kde.plasma.packagestructure              — Plasma/Generic package plugin id
pbsd.kde.plasma.{touchpad,mouse}.logging      — KCM logging categories
pbsd.kde.kwin.blur                            — upstream BlurEffect factory constants
pbsd.kde.kwin.compositing                     — panel/dialog blur bridge
pbsd.kde.kwin.effects.registry                — built-in effect plugin IDs
pbsd.kde.kwin.input_event                     — input event type tags
pbsd.kde.frameworks.kconfig.*                 — kconfig hand ports (11 modules)
pbsd.kde.frameworks.kcoreaddons.*             — kcoreaddons hand ports (27 modules)
pbsd.kde.frameworks.kio.{metadata,workerfactory,kpasswdserver}
pbsd.port.wave3.kde.<path>                    — auto-generated migration stubs (112)
```

## Build flags

- Theme stubs linked to nucleus: `-fno-exceptions -fno-rtti` (match `pbsd_handles`).
- Full Plasma/KF6 ports: hosted, exceptions allowed at QApplication boundary only.
- KWin plugin build: `-DPBSD_BUILD_KWIN_PLUGINS=ON` (requires ECM, KWin, KF6::Config).

## Install layout

```
share/plasma/desktoptheme/pbsd-aero/          ← metadata.json, colors, theme-colors.json, widgets/, dialogs/, panel/
share/color-schemes/PBSDAero.colors           ← system-wide color scheme
share/kwin/effects/pbsd_aero_blur/            ← metadata, config, shaders, qrc
share/kwin/decoration/pbsd_aero/              ← decoration metadata + SVG chrome + config.json
share/pbsd/kde/modules/                       ← 72 C++23 module sources (kde + logging + frameworks)
share/doc/pbsd-theme/plasma/panel/start-menu.md ← Start menu design notes
share/doc/pbsd-theme/kwin-effects/.../code/   ← effect C++ sources for packagers
pbsd/ports/wave3/kde/                         ← 112 auto-generated KF6 migration stubs
```

## Remaining for full port

- QML applets, runners, and KCMs in `kde/plasma-desktop/`
- Full KWin compositor path vs `pbsd.compositor` native path decision
- KF6 framework module partitions (~30 libraries)
- CI job building Plasma 6 against PBSD theme package
