export module pbsd.kde.layershell.interfaces_region;

import pbsd.core;

/// Burst 16 wave 2 — layer-shell region interface stub (string constants).
export namespace pbsd::kde::frameworks::layershell::interfaces_region {

inline constexpr const char kInterfaceId[] = "org.kde.layershell.region";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";
inline constexpr const char kIntegrationPlugin[] = "layer-shell-qt";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/interfaces/region.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::interfaces_region
