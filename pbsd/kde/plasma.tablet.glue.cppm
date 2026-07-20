export module pbsd.kde.plasma.tablet.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.tablet;

/// Burst 15 — Plasma tablet ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/tablet/tabletmoduledata.cpp
export namespace pbsd::kde::plasma::tablet::glue {

struct TabletStyle {
    const char* kcm_id{tablet::kKcmId};
    const char* config_group{tablet::kConfigGroup};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline TabletStyle default_style() noexcept {
    return TabletStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::tablet::upstream_path();
}

} // namespace pbsd::kde::plasma::tablet::glue
