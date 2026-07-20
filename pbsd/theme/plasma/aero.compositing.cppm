export module pbsd.theme.plasma.aero.compositing;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 6 — PBSD Aero theme compositing blur presets.
export namespace pbsd::theme::plasma::aero::compositing {

enum class Surface : unsigned char { Panel, Dialog, StartMenu, Decoration, Glow, SystemLink };

[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams blur(Surface s) noexcept {
    auto p = ::pbsd::kde::plasma::aero::default_blur();
    switch (s) {
    case Surface::Panel:
        p.panel_opacity = 0.72f;
        return p;
    case Surface::Dialog:
        p.dialog_opacity = 0.78f;
        return p;
    case Surface::StartMenu:
        p.dialog_opacity = 0.78f;
        p.corner_radius = 8;
        return p;
    case Surface::Decoration:
        p.title_bar_height = 36;
        return p;
    case Surface::Glow:
        p.blur_radius = 20;
        p.saturation = 1.20f;
        return p;
    case Surface::SystemLink:
        p.panel_opacity = 0.68f;
        return p;
    }
    return p;
}

} // namespace pbsd::theme::plasma::aero::compositing
