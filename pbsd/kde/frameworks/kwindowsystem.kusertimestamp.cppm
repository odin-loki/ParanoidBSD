export module pbsd.kde.kwindowsystem.kusertimestamp;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kusertimestamp.cpp).
/// Upstream: kde/frameworks/kwindowsystem/src/kusertimestamp.cpp
export namespace pbsd::kde::frameworks::kwindowsystem::kusertimestamp {

inline constexpr const char kTimestampProperty[] = "_NET_WM_USER_TIME";
inline constexpr unsigned kInvalidTimestamp = 0;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/kusertimestamp.cpp";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::kusertimestamp
