# PBSD Aero Theme (Wave 3)

Windows 7 Aero aesthetic for Plasma 6 + KWin Wayland.

## Layout

- `plasma/` — Plasma desktop theme package (`pbsd-aero`)
  - `metadata.json`, `colors`, `plasmarc`, `blur-effect.json`, `theme-colors.json` (v0.6.0)
  - `color-schemes/PBSDAero.colors` — system color scheme
  - `widgets/` — panel, button, tasks, tooltip, menuitem, progress, spinner, hover-highlight, combobox, listbox, treeview, volume, battery, wifi, bluetooth, …
  - `panel/` — taskbar, start-button, start-menu, system links (documents/music/games/…), power actions (lock/sleep/restart/logoff)
  - `dialogs/` — background, messagebox, filedialog, progress, wizard, properties, about
  - `decoration/` — KDecoration3 chrome SVGs + glow/border assets + `metadata.json`
- `kwin-effects/pbsd_aero_blur/` — full KWin effect package
  - `metadata.json`, `contents/config/pbsd_aero_blur.json`
  - `contents/shaders/` — Aero-tinted blur pass stubs
  - `contents/code/` — `PbsdAeroBlurEffect` (built with `-DPBSD_BUILD_KWIN_PLUGINS=ON`)
- `../kde/` — C++23 modules (224 hand ports):
  - `pbsd.kde.plasma.aero` — theme constants + 80+ asset paths
  - `pbsd.kde.plasma.startmenu` — Start menu layout (480×560)
  - `pbsd.kde.kwin.pbsd_aero` — KWin blur/decoration bridge
  - `pbsd.kde.kwin.effects.*` — 47 built-in effect factory constants
  - `pbsd.kde.kwin.{core,wayland,plugins}.*` — 31 KWin bridge modules
  - `pbsd.kde.frameworks.*` — KF6 hand ports (91 modules)

## Install

```bash
cmake -S pbsd -B build -G Ninja
cmake --build build
cmake --install build --prefix ~/.local
```

Plasma picks up `~/.local/share/plasma/desktoptheme/pbsd-aero`.
Color scheme: `~/.local/share/color-schemes/PBSDAero.colors`.
KWin effect: `~/.local/share/kwin/effects/pbsd_aero_blur/`.
Decoration assets: `~/.local/share/kwin/decoration/pbsd_aero/`.

Optional KWin plugin build (FreeBSD/Plasma host with KWin dev packages):

```bash
cmake -S pbsd -B build -DPBSD_BUILD_KWIN_PLUGINS=ON
cmake --build build --target pbsd_aero_blur
```
