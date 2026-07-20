export module pbsd.kde.plasma.tastenbrett.geometry.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.tastenbrett.geometry;

/// Burst 16 wave 2 — Plasma tastenbrett.geometry ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/geometry.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::tastenbrett::geometry::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct GeometryStyle {
    unsigned row_height_px{32};
    unsigned key_spacing_px{4};
};

[[nodiscard]] inline Status validate_row_height(unsigned px) noexcept {
    return px >= 16 && px <= 64 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::tastenbrett::geometry::upstream_path();
}

} // namespace pbsd::kde::plasma::tastenbrett::geometry::glue
