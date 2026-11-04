export module pbsd.kde.frameworks.kcoreaddons.kprocesslist_unix_procstat;

import pbsd.core;

/// Wave 3 pass 4 — procstat-backed process list constants.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix_procstat.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kprocesslist_unix_procstat {

    inline constexpr unsigned kMaxPid = 99999;
    inline constexpr const char kProcPath[] = "/proc";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix_procstat.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kprocesslist_unix_procstat
