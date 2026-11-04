export module pbsd.kde.frameworks.kwindowsystem.xcb_plugin;

import pbsd.core;

/// Wave 3 pass 4 — XCB platform plugin id.
/// Upstream: kde/frameworks/kwindowsystem/src/platforms/xcb/plugin.cpp
export namespace pbsd::kde::frameworks::kwindowsystem::xcb_plugin {

    inline constexpr const char kPluginId[] = "xcb";
    inline constexpr const char kPlatformName[] = "xcb";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/platforms/xcb/plugin.cpp";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::xcb_plugin
