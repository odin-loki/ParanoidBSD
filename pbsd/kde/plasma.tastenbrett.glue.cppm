export module pbsd.kde.plasma.tastenbrett.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.tastenbrett.section;

/// Burst 15 — Plasma tastenbrett ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/section.cpp
export namespace pbsd::kde::plasma::tastenbrett::glue {

struct KeyboardLayoutStyle {
    const char* alphanumeric{tastenbrett::section::kAlphanumeric};
    const char* modifier{tastenbrett::section::kModifier};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline KeyboardLayoutStyle default_style() noexcept {
    return KeyboardLayoutStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::tastenbrett::section::upstream_path();
}

} // namespace pbsd::kde::plasma::tastenbrett::glue
