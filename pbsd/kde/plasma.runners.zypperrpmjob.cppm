export module pbsd.kde.plasma.runners.zypperrpmjob;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (ZypperRPMJob.cpp).
/// Upstream: kde/plasma-desktop/kcms/runners/plugininstaller/ZypperRPMJob.cpp
export namespace pbsd::kde::plasma::runners::zypperrpmjob {

inline constexpr const char kPackageManager[] = "zypper";
inline constexpr const char kRpmExtension[] = ".rpm";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/runners/plugininstaller/ZypperRPMJob.cpp";
}

} // namespace pbsd::kde::plasma::runners::zypperrpmjob
