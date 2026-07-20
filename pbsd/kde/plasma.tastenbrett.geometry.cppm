export module pbsd.kde.plasma.tastenbrett.geometry;

import pbsd.core;

/// Wave 3 pass 4 — Keyboard layout geometry constants.
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/geometry.cpp
export namespace pbsd::kde::plasma::tastenbrett::geometry {

    inline constexpr int kDefaultKeyWidth = 48;
    inline constexpr int kDefaultKeyHeight = 48;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/keyboard/tastenbrett/geometry.cpp";
}

} // namespace pbsd::kde::plasma::tastenbrett::geometry
