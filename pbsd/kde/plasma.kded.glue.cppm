export module pbsd.kde.plasma.kded.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.kded;

/// Burst 14 — Plasma kded ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/kded/kdedconfigdata.cpp
export namespace pbsd::kde::plasma::kded::glue {

struct KdedStyle {
    const char* config_group{kded::kConfigGroup};
    const char* autoload_key{kded::kAutoloadKey};
};

[[nodiscard]] inline KdedStyle default_style() noexcept {
    return KdedStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return kded::upstream_path();
}

} // namespace pbsd::kde::plasma::kded::glue
