export module pbsd.kde.realtime;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (realtime.cpp).
/// Upstream: kde/kwin/src/utils/realtime.cpp
export namespace pbsd::kde::realtime {

inline constexpr int kSchedPolicy = 2;
inline constexpr int kMinPriority = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/utils/realtime.cpp";
}

} // namespace pbsd::kde::realtime
