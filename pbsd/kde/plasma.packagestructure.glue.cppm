export module pbsd.kde.plasma.packagestructure.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.packagestructure;

/// Burst 14 — Plasma packagestructure ↔ Aero glue.
/// Upstream: kde/plasma-framework/packagestructure/packagestructure.cpp
export namespace pbsd::kde::plasma::packagestructure::glue {

struct PackageStyle {
    const char* structure_id{"Plasma/Applet"};
    const char* fallback_svg{aero::kHoverHighlightSvg};
};

[[nodiscard]] inline PackageStyle default_style() noexcept {
    return PackageStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return packagestructure::upstream_path();
}

} // namespace pbsd::kde::plasma::packagestructure::glue
