export module pbsd.kde.kwin.compositing.theme;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.compositing;

/// Burst 11 — KWin compositor blur presets ↔ Aero theme surface roles.
/// Upstream: kde/kwin/src/compositing.cpp
export namespace pbsd::kde::kwin::compositing::theme {

enum class Surface : unsigned char {
    Panel,
    Dialog,
    StartMenu,
    Decoration,
    Glow,
    SystemLink,
};

[[nodiscard]] inline plasma::aero::BlurParams blur_for(Surface s) noexcept {
    switch (s) {
    case Surface::Panel:
        return compositing::panel_blur();
    case Surface::Dialog:
        return compositing::dialog_blur();
    case Surface::StartMenu:
        return compositing::start_menu_blur();
    case Surface::Decoration:
        return compositing::decoration_blur();
    case Surface::Glow:
        return compositing::glow_blur();
    case Surface::SystemLink:
        return compositing::system_link_blur();
    }
    return plasma::aero::default_blur();
}

[[nodiscard]] inline plasma::aero::CompositingSurface to_aero_surface(Surface s) noexcept {
    switch (s) {
    case Surface::Panel:
        return plasma::aero::CompositingSurface::Panel;
    case Surface::Dialog:
        return plasma::aero::CompositingSurface::Dialog;
    case Surface::StartMenu:
        return plasma::aero::CompositingSurface::StartMenu;
    case Surface::Decoration:
        return plasma::aero::CompositingSurface::Decoration;
    case Surface::Glow:
        return plasma::aero::CompositingSurface::Glow;
    case Surface::SystemLink:
        return plasma::aero::CompositingSurface::SystemLink;
    }
    return plasma::aero::CompositingSurface::Panel;
}

[[nodiscard]] inline Status validate_opacity(float o) noexcept {
    if (o < 0.0f || o > 1.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/compositing.cpp";
}

} // namespace pbsd::kde::kwin::compositing::theme
