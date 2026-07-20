export module pbsd.kde.frameworks.kconfig.kstandardshortcutwatcher;

import pbsd.core;
import pbsd.kde.frameworks.kconfig.kconfigwatcher;

/// Wave 3 — standard shortcut watcher config group filter.
/// Upstream: kde/frameworks/kconfig/src/gui/kstandardshortcutwatcher.cpp
export namespace pbsd::kde::frameworks::kconfig::kstandardshortcutwatcher {

inline constexpr const char kConfigFile[] = "kdeglobals";

[[nodiscard]] inline bool should_handle_group(const char* group) noexcept {
    return kconfigwatcher::is_shortcuts_group(group);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kstandardshortcutwatcher.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kstandardshortcutwatcher
