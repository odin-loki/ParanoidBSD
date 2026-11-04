export module pbsd.kde.plasma.colors.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.colors;

/// Burst 14 — Plasma colors ↔ Aero glue.
/// Upstream: pbsd/theme/plasma/color-schemes/PBSDAero.colors
export namespace pbsd::kde::plasma::colors::glue {

struct ColorStyle {
    colors::Rgb primary{colors::kPrimary};
    const char* scheme_name{colors::kColorSchemeName};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline ColorStyle default_style() noexcept {
    return ColorStyle{};
}

[[nodiscard]] inline colors::Rgb accent(colors::AccentRole role) noexcept {
    return colors::accent(role);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return colors::upstream_path();
}

} // namespace pbsd::kde::plasma::colors::glue
