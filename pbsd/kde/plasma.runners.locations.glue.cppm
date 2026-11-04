export module pbsd.kde.plasma.runners.locations.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.runners.locations;

/// Burst 18 — Plasma locations runner ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-workspace/runners/locations/locations.cpp
export namespace pbsd::kde::plasma::runners::locations::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct LocationsStyle {
    const char* runner_id{locations::kRunnerId};
    const char* home_prefix{locations::kHomePrefix};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline LocationsStyle default_style() noexcept {
    return LocationsStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::runners::locations::upstream_path();
}

} // namespace pbsd::kde::plasma::runners::locations::glue
