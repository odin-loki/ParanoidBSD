export module pbsd.kde.executable_path_sysctl;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (executable_path_sysctl.cpp).
/// Upstream: kde/kwin/src/utils/executable_path_sysctl.cpp
export namespace pbsd::kde::executable_path_sysctl {

inline constexpr int kCtlKern = 1;
inline constexpr int kKernProc = 14;
inline constexpr int kKernProcPathname = 12;
inline constexpr unsigned kMaxPathLen = 1024;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/utils/executable_path_sysctl.cpp";
}

} // namespace pbsd::kde::executable_path_sysctl
