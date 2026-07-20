export module pbsd.kde.frameworks.kcoreaddons.kprocesslist_unix;

import pbsd.core;

/// Wave 3 — Unix process list helpers (from kprocesslist_unix.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kprocesslist_unix {

inline constexpr unsigned kMaxPid = 99999;
inline constexpr unsigned kMaxProcesses = 4096;

[[nodiscard]] inline bool pid_valid(unsigned pid) noexcept {
    return pid > 0 && pid <= kMaxPid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kprocesslist_unix
