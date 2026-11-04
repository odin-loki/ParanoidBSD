export module pbsd.kde.plasma.baloo.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.baloo;

/// Burst 14 — Plasma baloo ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/kcm_baloo/main.cpp
export namespace pbsd::kde::plasma::baloo::glue {

struct BalooStyle {
    const char* kcm_id{"kcm_baloo"};
    bool indexing_enabled{true};
};

[[nodiscard]] inline BalooStyle default_style() noexcept {
    return BalooStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return baloo::upstream_path();
}

} // namespace pbsd::kde::plasma::baloo::glue
