export module pbsd.kde.kwin.wayland.xdg_decoration;

import pbsd.core;
import pbsd.kde.kwin.decorations;
import pbsd.kde.server_decoration;
import pbsd.kde.kwin.wayland.xdg_shell;

/// Wave 3 pass 7 — xdg-decoration ↔ pbsd_aero KDecoration3 bridge.
/// Upstream: kde/kwin/src/wayland/server_decoration.cpp
export namespace pbsd::kde::kwin::wayland::xdg_decoration {

inline constexpr const char kManagerInterface[] = "zxdg_decoration_manager_v1";
inline constexpr const char kToplevelDecoration[] = "zxdg_toplevel_decoration_v1";

[[nodiscard]] inline xdg_shell::DecorationMode preferred_mode() noexcept {
    return xdg_shell::DecorationMode::ServerSide;
}

[[nodiscard]] inline const char* decoration_plugin_id() noexcept {
    return decorations::kPluginId;
}

[[nodiscard]] inline decorations::DecorationMetrics default_metrics() noexcept {
    return decorations::default_metrics();
}

[[nodiscard]] inline bool plugin_matches(const char* id) noexcept {
    return decorations::plugin_id_matches(id);
}

[[nodiscard]] inline const char* server_decoration_interface() noexcept {
    return server_decoration::kServerDecoration;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return server_decoration::upstream_path();
}

} // namespace pbsd::kde::kwin::wayland::xdg_decoration
