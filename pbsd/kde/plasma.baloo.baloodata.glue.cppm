export module pbsd.kde.plasma.baloo.baloodata.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.baloo.baloodata;

/// Burst 16 wave 2 — Plasma baloo.baloodata ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/kcm_baloo/baloodata.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::baloo::baloodata::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct BalooDataStyle {
    bool file_indexing{true};
    bool email_indexing{false};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline BalooDataStyle default_style() noexcept { return BalooDataStyle{}; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::baloo::baloodata::upstream_path();
}

} // namespace pbsd::kde::plasma::baloo::baloodata::glue
