export module pbsd.kde.layershell.layershellintegration;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (qwaylandlayershellintegration.cpp).
/// Upstream: kde/frameworks/layer-shell-qt/src/qwaylandlayershellintegration.cpp
export namespace pbsd::kde::frameworks::layershell::layershellintegration {

inline constexpr const char kIntegrationPlugin[] = "layer-shell-qt";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/qwaylandlayershellintegration.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::layershellintegration
