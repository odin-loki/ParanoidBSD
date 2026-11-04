export module pbsd.kde.plasma.systemsettings.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.systemsettings;

/// Burst 14 — Plasma systemsettings ↔ Aero glue.
/// Upstream: kde/systemsettings/app/main.cpp
export namespace pbsd::kde::plasma::systemsettings::glue {

struct SettingsStyle {
    const char* kcm_root{"systemsettings5"};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline SettingsStyle default_style() noexcept {
    return SettingsStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return systemsettings::upstream_path();
}

} // namespace pbsd::kde::plasma::systemsettings::glue
