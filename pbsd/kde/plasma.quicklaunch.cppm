export module pbsd.kde.plasma.quicklaunch;

import pbsd.core;

/// Wave 3 pass 5 — Quick launch strip constants.
/// Upstream: kde/plasma-desktop/containments/panel/plugins/quicklaunch/quicklaunch.cpp
export namespace pbsd::kde::plasma::quicklaunch {

    inline constexpr const char kQuickLaunchSvg[] = "plasma/panel/quick-launch.svg";
    inline constexpr unsigned kMaxIcons{10};
    inline constexpr int kIconSize{32};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/containments/panel/plugins/quicklaunch/quicklaunch.cpp";
}

} // namespace pbsd::kde::plasma::quicklaunch
