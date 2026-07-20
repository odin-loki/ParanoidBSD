export module pbsd.kde.plasma.touchscreen.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.touchscreen;

/// Burst 15 — Plasma touchscreen ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp
export namespace pbsd::kde::plasma::touchscreen::glue {

struct TouchscreenStyle {
    const char* kcm_id{touchscreen::kKcmId};
    const char* settings_svg{aero::kSettingsSvg};
    float panel_opacity{0.75f};
};

[[nodiscard]] inline TouchscreenStyle default_style() noexcept {
    return TouchscreenStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::touchscreen::upstream_path();
}

} // namespace pbsd::kde::plasma::touchscreen::glue
