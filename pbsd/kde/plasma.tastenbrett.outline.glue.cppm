export module pbsd.kde.plasma.tastenbrett.outline.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.tastenbrett.outline;

/// Burst 16 wave 2 — Plasma tastenbrett.outline ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/outline.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::tastenbrett::outline::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct OutlineStyle {
    float stroke_opacity{0.85f};
    unsigned corner_radius_px{3};
};

[[nodiscard]] inline OutlineStyle default_style() noexcept { return OutlineStyle{}; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::tastenbrett::outline::upstream_path();
}

} // namespace pbsd::kde::plasma::tastenbrett::outline::glue
