export module pbsd.kde.plasma.kicker;

import pbsd.core;

/// Wave 3 — hand port constants (main.xml).
/// Upstream: kde/plasma-desktop/applets/kicker/package/contents/config/main.xml
export namespace pbsd::kde::plasma::kicker {

inline constexpr const char kStartButtonSvg[] = "plasma/panel/start-button.svg";
inline constexpr const char kApplicationsSvg[] = "plasma/panel/applications.svg";
inline constexpr unsigned kFavoriteSlots = 12;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/applets/kicker/package/contents/config/main.xml";
}

} // namespace pbsd::kde::plasma::kicker
