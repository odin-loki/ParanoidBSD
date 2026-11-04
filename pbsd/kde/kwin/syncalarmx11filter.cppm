export module pbsd.kde.syncalarmx11filter;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (syncalarmx11filter.cpp).
/// Upstream: kde/kwin/src/syncalarmx11filter.cpp
export namespace pbsd::kde::syncalarmx11filter {

inline constexpr const char kSyncCounterAtom[] = "SYNC_COUNTER";
inline constexpr unsigned kAlarmMask = 3;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/syncalarmx11filter.cpp";
}

} // namespace pbsd::kde::syncalarmx11filter
