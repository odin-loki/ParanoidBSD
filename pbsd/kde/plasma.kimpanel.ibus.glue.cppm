export module pbsd.kde.plasma.kimpanel.ibus.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.kimpanel.ibus;

/// Burst 15 — Plasma kimpanel.ibus ↔ Aero glue.
/// Upstream: kde/plasma-desktop/applets/kimpanel/backend/ibus/ibus15/main.cpp
export namespace pbsd::kde::plasma::kimpanel::ibus::glue {

struct InputPanelStyle {
    const char* ibus_service{kimpanel::ibus::kIbusService};
    const char* ibus_path{kimpanel::ibus::kIbusPath};
    float popup_opacity{0.82f};
};

[[nodiscard]] inline InputPanelStyle default_style() noexcept {
    return InputPanelStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::kimpanel::ibus::upstream_path();
}

} // namespace pbsd::kde::plasma::kimpanel::ibus::glue
