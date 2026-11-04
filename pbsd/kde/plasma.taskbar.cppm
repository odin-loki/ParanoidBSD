export module pbsd.kde.plasma.taskbar;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.taskmanager;

/// Wave 3 pass 5 — Taskbar strip metrics (layout.json + taskmanager).
/// Upstream: kde/plasma-desktop/containments/panel/Panel.qml
export namespace pbsd::kde::plasma::taskbar {

    inline constexpr int kDefaultHeight{40};
    inline constexpr int kTaskIconSize{::pbsd::kde::plasma::taskmanager::kDefaultIconSize};
    inline constexpr const char kTaskbarSvg[] = "plasma/panel/taskbar.svg";
    inline constexpr const char kTasksWidgetSvg[] = "plasma/widgets/tasks.svg";
    inline constexpr float kGlassOpacity{0.72f};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/containments/panel/Panel.qml";
}

} // namespace pbsd::kde::plasma::taskbar
