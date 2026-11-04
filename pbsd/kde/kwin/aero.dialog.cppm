export module pbsd.kde.kwin.aero.dialog;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.compositing;
import pbsd.kde.kwin.compositing.theme;

/// Burst 12 — KWin dialog blur ↔ Aero theme surface dispatch.
/// Upstream: kde/kwin/src/compositing.cpp + pbsd/theme/plasma/dialogs/
export namespace pbsd::kde::kwin::aero::dialog {

struct DialogBlur {
    ::pbsd::kde::plasma::aero::BlurParams params{};
    float shadow_opacity{0.40f};
    int corner_radius{8};
    const char* messagebox_svg{::pbsd::kde::plasma::aero::kMessageBoxSvg};
};

[[nodiscard]] inline DialogBlur default_blur() noexcept {
    DialogBlur d{};
    d.params = compositing::theme::blur_for(compositing::theme::Surface::Dialog);
    d.corner_radius = d.params.corner_radius;
    return d;
}

[[nodiscard]] inline DialogBlur for_surface(
    ::pbsd::kde::plasma::aero::CompositingSurface s) noexcept {
    DialogBlur d{};
    switch (s) {
    case ::pbsd::kde::plasma::aero::CompositingSurface::StartMenu:
        d.params = compositing::start_menu_blur();
        break;
    case ::pbsd::kde::plasma::aero::CompositingSurface::Decoration:
        d.params = compositing::decoration_blur();
        d.shadow_opacity = 0.35f;
        break;
    default:
        d.params = compositing::dialog_blur();
        break;
    }
    d.corner_radius = d.params.corner_radius;
    return d;
}

[[nodiscard]] inline Status validate_opacity(float o) noexcept {
    return compositing::theme::validate_opacity(o);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return compositing::theme::upstream_path();
}

} // namespace pbsd::kde::kwin::aero::dialog
