export module pbsd.kde.plasma.tastenbrett.section;

import pbsd.core;

/// Wave 3 pass 4 — Keyboard section labels.
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/section.cpp
export namespace pbsd::kde::plasma::tastenbrett::section {

    inline constexpr const char kAlphanumeric[] = "alphanumeric";
    inline constexpr const char kModifier[] = "modifier";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/keyboard/tastenbrett/section.cpp";
}

} // namespace pbsd::kde::plasma::tastenbrett::section
