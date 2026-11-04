export module pbsd.kde.plasma.taskmanager.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.taskmanager;

/// Burst 15 — Plasma taskmanager ↔ Aero glue.
/// Upstream: kde/plasma-desktop/applets/taskmanager/package/contents/config/main.xml
export namespace pbsd::kde::plasma::taskmanager::glue {

struct TaskbarStyle {
    int icon_size{taskmanager::kDefaultIconSize};
    int max_tasks{taskmanager::kMaxTasks};
    const char* tasks_svg{taskmanager::kTasksSvg};
    float glass_opacity{0.72f};
};

[[nodiscard]] inline TaskbarStyle default_style() noexcept {
    return TaskbarStyle{};
}

[[nodiscard]] inline Status validate_icon_size(int px) noexcept {
    return px >= 16 && px <= 64 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::taskmanager::upstream_path();
}

} // namespace pbsd::kde::plasma::taskmanager::glue
