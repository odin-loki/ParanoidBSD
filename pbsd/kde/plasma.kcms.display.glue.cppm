export module pbsd.kde.plasma.kcms.display.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.kcms.display;

/// Burst 17 — Plasma kcms.display ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-desktop/kcms/display/display.cpp
export namespace pbsd::kde::plasma::kcms::display::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct DisplayKcmStyle {
    const char* kcm_id{display::kKcmId};
    const char* settings_svg{aero::kSettingsSvg};
    const char* layer_shell{kLayerShell};
};

[[nodiscard]] inline DisplayKcmStyle default_style() noexcept {
    return DisplayKcmStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::kcms::display::upstream_path();
}

} // namespace pbsd::kde::plasma::kcms::display::glue
