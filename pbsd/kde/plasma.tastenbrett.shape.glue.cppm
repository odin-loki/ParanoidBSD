export module pbsd.kde.plasma.tastenbrett.shape.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.tastenbrett.shape;

/// Burst 16 wave 2 — Plasma tastenbrett.shape ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/shape.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::tastenbrett::shape::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct ShapeStyle {
    unsigned min_width_px{24};
    unsigned min_height_px{24};
};

[[nodiscard]] inline ShapeStyle default_style() noexcept { return ShapeStyle{}; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::tastenbrett::shape::upstream_path();
}

} // namespace pbsd::kde::plasma::tastenbrett::shape::glue
