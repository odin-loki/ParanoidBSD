export module pbsd.kde.plasma.containmentactions.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.containmentactions;

/// Burst 16 — Plasma containmentactions ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-framework/containmentactions.cpp
export namespace pbsd::kde::plasma::containmentactions::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct ActionsStyle {
    const char* plugin_prefix{containmentactions::kPluginPrefix};
    unsigned max_actions{containmentactions::kMaxActions};
    const char* layer_shell{kLayerShell};
};

[[nodiscard]] inline ActionsStyle default_style() noexcept {
    return ActionsStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::containmentactions::upstream_path();
}

} // namespace pbsd::kde::plasma::containmentactions::glue
