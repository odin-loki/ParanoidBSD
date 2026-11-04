export module pbsd.kde.kpackage_windowswitcher;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (windowswitcher.cpp).
/// Upstream: kde/kwin/src/plugins/kpackage/windowswitcher/windowswitcher.cpp
export namespace pbsd::kde::kpackage_windowswitcher {

inline constexpr const char kTabBoxPackage[] = "kwin/tabbox";
inline constexpr const char kLayoutFile[] = "layout.qml";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/kpackage/windowswitcher/windowswitcher.cpp";
}

} // namespace pbsd::kde::kpackage_windowswitcher
