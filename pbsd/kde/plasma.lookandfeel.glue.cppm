export module pbsd.kde.plasma.lookandfeel.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.lookandfeel;

/// Wave 3 pass 7 — Look-and-feel package ↔ Aero theme-colors.json glue.
/// Upstream: kde/plasma-desktop/kcms/landingpage/lookandfeelmetadata.cpp
export namespace pbsd::kde::plasma::lookandfeel::glue {

struct ThemeBinding {
    const char* theme_id{aero::kThemeId};
    const char* color_scheme{aero::kColorScheme};
    const char* colors_path{aero::kThemeColorsPath};
    const char* blur_config{aero::kBlurEffectPath};
    const char* decoration_id{aero::kDecorationId};
};

[[nodiscard]] inline ThemeBinding aero_binding() noexcept {
    return ThemeBinding{};
}

[[nodiscard]] inline bool package_matches(const char* package_id) noexcept {
    if (package_id == nullptr) {
        return false;
    }
    return package_id[0] == lookandfeel::kPackageId[0]
        && aero::theme_id_matches(lookandfeel::aero_theme_id());
}

[[nodiscard]] inline Status validate_color_scheme(const char* scheme) noexcept {
    if (scheme == nullptr || scheme[0] == '\0') {
        return Status::Invalid;
    }
    if (scheme[0] == 'P' && scheme[1] == 'B' && scheme[2] == 'S'
        && scheme[3] == 'D') {
        return Status::Ok;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* accent_primary_hex() noexcept {
    return aero::kAccentPrimaryHex;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return lookandfeel::upstream_path();
}

} // namespace pbsd::kde::plasma::lookandfeel::glue
