# PBSD Native Compositor (Wave 8)

Native Wayland compositor module (`pbsd.compositor`) with Aero fidelity hooks aligned to the Wave 3 theme package.

## KWin retirement path

| Phase | Compositor | Notes |
|-------|------------|-------|
| Wave 3 (now) | KWin Wayland + `pbsd/theme` | Aero blur/glass metadata consumed by KWin effects |
| Wave 8 (scaffold) | `pbsd.compositor` façade | `load_kwin_effect_metadata()` ingests existing JSON; dual-run optional |
| Wave 8+ | Session switch | `startplasma` launches `pbsd-compositor` when `PBSD_NATIVE_COMPOSITOR=1` |
| PBSD 0.1 | KWin removed from default image | KWin tree retained in `kde/kwin/` for reference until parity sign-off |

Parity gate: side-by-side capture diff (blur radius, tint, shadow depth, snap timing) must match KWin baseline within theme tolerance before KWin is dropped from release media.

## Aero hooks

- `AeroTheme` — bundled tint, blur, shadow, snap animation, live-preview flags
- `load_kwin_effect_metadata()` — bridge from `pbsd/theme/kwin-effects/` during migration
- `bind_output()` — maps Desktop / Overlay / Cursor roles to physical outputs
