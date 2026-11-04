export module pbsd.kde.plasma.mouse.kapplymousetheme.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.mouse.kapplymousetheme;

/// Burst 15 — Plasma mouse.kapplymousetheme ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/mouse/kapplymousetheme.cpp
export namespace pbsd::kde::plasma::mouse::kapplymousetheme::glue {

struct MouseThemeStyle {
    const char* theme_group{mouse::kapplymousetheme::kCursorThemeGroup};
    const char* theme_key{mouse::kapplymousetheme::kThemeKey};
    const char* settings_svg{aero::kSettingsSvg};
    unsigned cursor_size{24};
};

[[nodiscard]] inline MouseThemeStyle default_style() noexcept {
    return MouseThemeStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::mouse::kapplymousetheme::upstream_path();
}

} // namespace pbsd::kde::plasma::mouse::kapplymousetheme::glue
