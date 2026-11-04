export module pbsd.theme.plasma.aero.kwin_bridge;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.pbsd_aero;
import pbsd.kde.kwin.decorations;
import pbsd.kde.kwin.effects.pbsd_aero_blur;

/// Wave 3 pass 7 — Theme package ↔ KWin pbsd_aero blur/decoration bridge.
export namespace pbsd::theme::plasma::aero::kwin_bridge {

struct PackagePaths {
    const char* theme_id{::pbsd::kde::plasma::aero::kThemeId};
    const char* blur_effect{::pbsd::kde::kwin::effects::pbsd_aero_blur::kEffectId};
    const char* blur_metadata{::pbsd::kde::kwin::effects::pbsd_aero_blur::kMetadataPath};
    const char* decoration_plugin{::pbsd::kde::kwin::decorations::kPluginId};
    const char* decoration_metadata{::pbsd::kde::kwin::decorations::kMetadataPath};
};

[[nodiscard]] inline PackagePaths default_paths() noexcept {
    return PackagePaths{};
}

[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams panel_blur() noexcept {
    return ::pbsd::kde::kwin::effects::pbsd_aero_blur::from_aero_defaults().panel;
}

[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams dialog_blur() noexcept {
    return ::pbsd::kde::kwin::effects::pbsd_aero_blur::from_aero_defaults().dialog;
}

[[nodiscard]] inline const char* effect_plugin_id() noexcept {
    return ::pbsd::kde::kwin::effect_plugin_id();
}

[[nodiscard]] inline bool theme_matches(const char* id) noexcept {
    return ::pbsd::kde::plasma::aero::theme_id_matches(id);
}

} // namespace pbsd::theme::plasma::aero::kwin_bridge
