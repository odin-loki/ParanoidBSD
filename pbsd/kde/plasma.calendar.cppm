export module pbsd.kde.plasma.calendar;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Plasma calendar constants.
/// Upstream: kde/plasma-desktop/applets/calendar/calendar.cpp
export namespace pbsd::kde::plasma::calendar {

inline constexpr const char kAppletId[] = "org.kde.plasma.calendar";
inline constexpr unsigned kWeekRows{6};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/applets/calendar/calendar.cpp";
}

} // namespace pbsd::kde::plasma::calendar
