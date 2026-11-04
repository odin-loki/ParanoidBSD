export module pbsd.kde.plasma.widgets.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.widgets;

/// Burst 14 — Plasma widgets ↔ Aero glue.
/// Upstream: kde/plasma-framework/src/plasmaquick/plasmoid/plasmoid.cpp
export namespace pbsd::kde::plasma::widgets::glue {

struct WidgetStyle {
    const char* containment_svg{aero::kHoverHighlightSvg};
    float panel_opacity{aero::default_blur().panel_opacity};
};

[[nodiscard]] inline WidgetStyle default_style() noexcept {
    return WidgetStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return widgets::upstream_path();
}

} // namespace pbsd::kde::plasma::widgets::glue
