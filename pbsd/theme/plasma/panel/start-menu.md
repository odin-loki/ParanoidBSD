# PBSD Aero — Start Menu Notes (Wave 3)

Windows 7 Aero Start menu layout for Plasma 6 kickoff / fullscreen launcher.

## Dimensions

| Element | Size | Asset |
|---------|------|-------|
| Menu panel | 480 × 560 px | `start-menu.svg` |
| Search box | 36 px height | `search-box.svg` |
| User tile | 64 px height | `user-tile.svg` |
| Sidebar | 240 px width | `sidebar.svg` |
| Shutdown button | 120 × 32 px | `shutdown.svg` |
| Menu item row | 32 px height | `widgets/menuitem.svg` |
| Hover highlight | 240 × 32 px | `widgets/hover-highlight.svg` |
| Applications column | 240 px width | `applications.svg` |
| System links | 20 × 20 icons | `documents.svg`, `pictures.svg`, `control-panel.svg` |

## Visual targets

- **Background**: dual Kawase blur @ radius 24, glass tint `0.12,0.22,0.40 @ 0.78 α`
- **Left column**: pinned + frequent apps (Aero blue hover `#78a8e8`)
- **Right column**: system links (Documents, Pictures, Control Panel)
- **Search**: inset glass field with `#5082c8` focus ring
- **Shutdown**: split button bottom-right (orange hover on power icon)

## Module bridge

- C++23 constants: `pbsd.kde.plasma.startmenu` (`plasma.startmenu.cppm`)
- Theme paths: `pbsd.kde.plasma.aero` (`kStartMenuSvg`, `kStartMenuNotesPath`)
- Blur params: `pbsd.kde.kwin.compositing::dialog_blur()`

## KWin integration

1. Enable `pbsd_aero_blur` effect (reads `blur-effect.json`).
2. Decoration plugin `pbsd_aero` for popup windows spawned from Start menu.
3. Panel `start-button.svg` opens menu anchored to bottom-left taskbar slot.

## Remaining

- QML kickoff applet importing `pbsd.kde.plasma.startmenu`
- Search provider wiring (Plasma runners)
- Per-monitor DPI scaling on multi-head setups
