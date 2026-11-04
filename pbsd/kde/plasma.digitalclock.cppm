export module pbsd.kde.plasma.digitalclock;

import pbsd.core;

/// Wave 3 — hand port constants (main.xml).
/// Upstream: kde/plasma-desktop/applets/digital-clock/package/contents/config/main.xml
export namespace pbsd::kde::plasma::digitalclock {

inline constexpr const char kClockSvg[] = "plasma/panel/clock.svg";
inline constexpr const char kDateFormat[] = "ddd MMM d";
inline constexpr const char kTimeFormat[] = "h:mm AP";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/applets/digital-clock/package/contents/config/main.xml";
}

} // namespace pbsd::kde::plasma::digitalclock
