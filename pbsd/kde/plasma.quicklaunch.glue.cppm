export module pbsd.kde.plasma.quicklaunch.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.panel;
import pbsd.kde.plasma.quicklaunch;

/// Burst 12 — Quick launch strip ↔ Aero panel asset glue.
/// Upstream: kde/plasma-desktop/containments/panel/plugins/quicklaunch/
export namespace pbsd::kde::plasma::quicklaunch::glue {

struct QuickLaunchLayout {
    unsigned max_icons{quicklaunch::kMaxIcons};
    int icon_size{quicklaunch::kIconSize};
    int panel_height{panel::kDefaultHeight};
    const char* quick_launch_svg{quicklaunch::kQuickLaunchSvg};
    const char* applications_svg{aero::kApplicationsSvg};
};

[[nodiscard]] inline QuickLaunchLayout default_layout() noexcept {
    return QuickLaunchLayout{};
}

[[nodiscard]] inline int strip_width(unsigned icon_count, int icon_size) noexcept {
    if (icon_count == 0 || icon_size <= 0) {
        return 0;
    }
    const unsigned capped = icon_count > quicklaunch::kMaxIcons ? quicklaunch::kMaxIcons : icon_count;
    return static_cast<int>(capped) * (icon_size + 6) + 12;
}

[[nodiscard]] inline Status validate_icon_count(unsigned count) noexcept {
    if (count > quicklaunch::kMaxIcons) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return quicklaunch::upstream_path();
}

} // namespace pbsd::kde::plasma::quicklaunch::glue
