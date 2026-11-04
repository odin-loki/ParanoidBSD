export module pbsd.kde.frameworks.kwindowsystem.startupinfo;

import pbsd.core;

/// Wave 3 pass 5 — KStartupInfo tri-state and property keys.
/// Upstream: kde/frameworks/kwindowsystem/src/kstartupinfo.h
export namespace pbsd::kde::frameworks::kwindowsystem::startupinfo {

    enum class TriState : unsigned char { Yes, No, Unknown };
    inline constexpr const char kStartupId[] = "StartupId";
    inline constexpr const char kWMClass[] = "WMClass";
    inline constexpr const char kDesktopFile[] = "DesktopFile";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/kstartupinfo.h";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::startupinfo
