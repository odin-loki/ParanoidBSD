export module pbsd.kde.plasma.menuhelper.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.menuhelper;

/// Burst 14 — Plasma menuhelper ↔ Aero glue.
/// Upstream: kde/plasma-desktop/shell/menuhelper.cpp
export namespace pbsd::kde::plasma::menuhelper::glue {

struct MenuStyle {
    const char* context_menu_id{"plasma-desktop"};
    unsigned max_depth{8};
};

[[nodiscard]] inline MenuStyle default_style() noexcept {
    return MenuStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return menuhelper::upstream_path();
}

} // namespace pbsd::kde::plasma::menuhelper::glue
