export module pbsd.kde.plasma.quickaccess.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.quickaccess;

/// Burst 14 — Plasma quickaccess ↔ Aero glue.
/// Upstream: kde/plasma-desktop/containments/panel/quickaccess/QuickAccess.qml
export namespace pbsd::kde::plasma::quickaccess::glue {

struct QuickAccessStyle {
    const char* icon_svg{aero::kQuickLaunchSvg};
    unsigned max_items{12};
};

[[nodiscard]] inline QuickAccessStyle default_style() noexcept {
    return QuickAccessStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return quickaccess::upstream_path();
}

} // namespace pbsd::kde::plasma::quickaccess::glue
