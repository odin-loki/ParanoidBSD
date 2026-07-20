export module pbsd.kde.plasma.keyboard.layoutnames.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.keyboard.layoutnames;

/// Burst 16 wave 2 — Plasma keyboard.layoutnames ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/keyboard/layoutnames.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::keyboard::layoutnames::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct LayoutNameStyle {
    const char* settings_svg{aero::kSettingsSvg};
    unsigned max_layouts{8};
};

[[nodiscard]] inline Status validate_layout_count(unsigned n) noexcept {
    return n <= 8 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::keyboard::layoutnames::upstream_path();
}

} // namespace pbsd::kde::plasma::keyboard::layoutnames::glue
