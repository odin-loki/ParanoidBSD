export module pbsd.kde.plasma.qtquicksettings.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.qtquicksettings;

/// Burst 14 — Plasma qtquicksettings ↔ Aero glue.
/// Upstream: kde/plasma-framework/src/plasmaquick/plasmoid/plasmoid.cpp
export namespace pbsd::kde::plasma::qtquicksettings::glue {

struct QtQuickStyle {
    const char* import_uri{"org.kde.plasma.core"};
    unsigned animation_ms{220};
};

[[nodiscard]] inline QtQuickStyle default_style() noexcept {
    return QtQuickStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return qtquicksettings::upstream_path();
}

} // namespace pbsd::kde::plasma::qtquicksettings::glue
