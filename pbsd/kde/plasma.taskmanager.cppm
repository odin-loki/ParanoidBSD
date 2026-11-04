export module pbsd.kde.plasma.taskmanager;

import pbsd.core;

/// Wave 3 — task manager layout constants.
/// Upstream: kde/plasma-desktop/applets/taskmanager/package/contents/config/main.xml
export namespace pbsd::kde::plasma::taskmanager {

inline constexpr int kDefaultIconSize{32};
inline constexpr int kGroupMode{0};
inline constexpr int kMaxTasks{64};
inline constexpr const char kTasksSvg[] = "plasma/widgets/tasks.svg";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/applets/taskmanager/";
}

} // namespace pbsd::kde::plasma::taskmanager
