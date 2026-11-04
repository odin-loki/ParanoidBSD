export module pbsd.kde.plasma.taskbar.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.panel;
import pbsd.kde.plasma.taskbar;
import pbsd.kde.plasma.taskmanager;

/// Burst 11 — Taskbar applet ↔ Aero panel asset glue.
/// Upstream: kde/plasma-desktop/applets/taskmanager/
export namespace pbsd::kde::plasma::taskbar::glue {

struct TaskbarLayout {
    int icon_size{taskmanager::kDefaultIconSize};
    int max_tasks{taskmanager::kMaxTasks};
    int panel_height{panel::kDefaultHeight};
    float panel_opacity{panel::kDefaultOpacity};
    const char* taskbar_svg{aero::kTaskbarSvg};
    const char* tasks_svg{taskmanager::kTasksSvg};
};

[[nodiscard]] inline TaskbarLayout default_layout() noexcept {
    TaskbarLayout l{};
    l.icon_size = taskbar::kTaskIconSize;
    return l;
}

[[nodiscard]] inline int task_slot_width(int icon_size) noexcept {
    if (icon_size <= 0) {
        return taskmanager::kDefaultIconSize + 8;
    }
    return icon_size + 8;
}

[[nodiscard]] inline Status validate_icon_size(int size) noexcept {
    if (size < 16 || size > 64) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return taskmanager::upstream_path();
}

} // namespace pbsd::kde::plasma::taskbar::glue
