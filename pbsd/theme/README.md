# PBSD Theme Layer

Windows 7 Aero aesthetic for KDE Plasma 6 — compositor layer only, zero kernel coupling.

## Components
- `plasma/`        — Plasma theme (colours, backgrounds, panel style)
- `kwin-effects/`  — KWin compositor effects (Aero glass blur, shadow depth, transparency)
- `widget-style/`  — Qt/KDE widget style matching Windows 7 control appearance

## Notes
- Aero glass: KWin blur + transparency effects with a specific colour matrix
- Taskbar: bottom-anchored panel, Windows 7 layout (start button, task buttons, clock, tray)
- Start menu: KRunner or custom Plasmoid matching Win7 start menu layout
- Window chrome: custom KWin decoration (title bar gradient, close/min/max button placement)

## References
- KDE Plasma theming documentation: https://develop.kde.org/docs/plasma/theme/
- KWin effect development: https://develop.kde.org/docs/plasma/kwin/
