export module pbsd.kde.plasma.tastenbrett.row.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.tastenbrett.row;

/// Burst 16 wave 2 — Plasma tastenbrett.row ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/row.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::tastenbrett::row::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct RowStyle {
    unsigned max_keys{20};
    float label_opacity{0.90f};
};

[[nodiscard]] inline Status validate_max_keys(unsigned n) noexcept {
    return n > 0 && n <= 32 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::tastenbrett::row::upstream_path();
}

} // namespace pbsd::kde::plasma::tastenbrett::row::glue
