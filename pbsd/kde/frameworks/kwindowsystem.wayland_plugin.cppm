export module pbsd.kde.frameworks.kwindowsystem.wayland_plugin;

import pbsd.core;

/// Wave 3 pass 4 — Wayland platform plugin id.
/// Upstream: kde/frameworks/kwindowsystem/src/platforms/wayland/plugin.cpp
export namespace pbsd::kde::frameworks::kwindowsystem::wayland_plugin {

    inline constexpr const char kPluginId[] = "wayland";
    inline constexpr const char kPlatformName[] = "wayland";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/platforms/wayland/plugin.cpp";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::wayland_plugin
