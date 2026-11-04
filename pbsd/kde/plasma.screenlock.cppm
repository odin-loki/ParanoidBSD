export module pbsd.kde.plasma.screenlock;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Plasma screenlock constants.
/// Upstream: kde/plasma-workspace/screenlocker/main.cpp
export namespace pbsd::kde::plasma::screenlock {

inline constexpr const char kGreeterId[] = "org.kde.plasma.screenlocker";
inline constexpr unsigned kGraceMs{5000};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/screenlocker/main.cpp";
}

} // namespace pbsd::kde::plasma::screenlock
