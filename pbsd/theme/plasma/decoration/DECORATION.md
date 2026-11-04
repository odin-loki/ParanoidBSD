# PBSD Aero — KDecoration3 Notes (Wave 3)

Windows 7 Aero window chrome for KWin / KDecoration3. Shader plugin and QML
follow in Wave 3 continuation; this document captures decoration parameters.

## Visual targets

- **Title bar height**: 30 px (compact), 36 px (touch)
- **Border width**: 1 px outer glow, 4 px resize hit region
- **Corner radius**: 6 px on maximized-unframe, 8 px floating
- **Button layout**: `[icon] title … [min] [max] [close]` (LTR)

## Color tokens (from `colors`)

| Token | RGB | Use |
|-------|-----|-----|
| `decoration.focus` | 80,130,200 | Active title gradient top |
| `decoration.hover` | 100,150,220 | Button hover |
| `decoration.inactive` | 60,90,130 | Inactive title bar |
| `glass.tint` | 0.12,0.22,0.40 @ 0.65 α | KWin blur pass |

## KDecoration3 integration

1. Install as `org.kde.kdecoration3` plugin `pbsd_aero` (metadata + SVG chrome in
   `share/kwin/decoration/pbsd_aero/`; C++23 factory stub: `pbsd.kde.kwin.pbsd_aero`).
2. Read `blur-effect.json` for glass parameters; forward to KWin effect
   `pbsd_aero_blur` (see `kwin-effects/pbsd_aero_blur/`).
3. Window button SVGs: `close.svg`, `maximize.svg`, `minimize.svg`.
4. Title bars: `titlebar-active.svg`, `titlebar-inactive.svg`.

## Remaining

- QML decoration plugin + C++23 KDecoration3 factory
- Per-monitor DPI scaling tests on Wayland
- Snap preview and Aero Shake (optional, Wave 8 compositor)
