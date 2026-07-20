export module pbsd.kde.plasma.panel.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.panel;

/// Burst 14 — Plasma panel ↔ Aero glue.
/// Upstream: kde/plasma-desktop/containments/panel/Panel.qml
export namespace pbsd::kde::plasma::panel::glue {

struct PanelStyle {
    int height{panel::kDefaultHeight};
    float opacity{panel::kDefaultOpacity};
    int blur_radius{panel::kDefaultBlurRadius};
    const char* layout_svg{aero::kPanelGlassSvg};
};

[[nodiscard]] inline PanelStyle default_style() noexcept {
    return PanelStyle{};
}

[[nodiscard]] inline Status validate_height(int h) noexcept {
    return h >= 24 && h <= 128 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return panel::upstream_path();
}

} // namespace pbsd::kde::plasma::panel::glue
