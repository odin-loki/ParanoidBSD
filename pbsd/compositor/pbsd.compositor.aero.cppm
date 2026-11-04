export module pbsd.compositor.aero;

import pbsd.core;
import pbsd.compositor.wayland;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.compositing;
import pbsd.theme.plasma.aero.glass;
import pbsd.theme.plasma.aero.compositing;

/// Wave 3 pass 7 — Compositor AeroTheme from KDE/theme module tokens.
export namespace pbsd::compositor::aero {

struct ThemeBundle {
    wayland::AeroDefaults wire{};
    ::pbsd::kde::plasma::aero::BlurParams panel{};
    ::pbsd::kde::plasma::aero::BlurParams dialog{};
    ::pbsd::theme::plasma::aero::glass::Material panel_glass{};
};

[[nodiscard]] inline ThemeBundle from_theme_modules() noexcept {
    ThemeBundle b{};
    b.panel = ::pbsd::theme::plasma::aero::compositing::blur(
        ::pbsd::theme::plasma::aero::compositing::Surface::Panel);
    b.dialog = ::pbsd::theme::plasma::aero::compositing::blur(
        ::pbsd::theme::plasma::aero::compositing::Surface::Dialog);
    b.panel_glass = ::pbsd::theme::plasma::aero::glass::from_blur_params(b.panel);
    b.wire.blur_radius_px = static_cast<float>(b.panel.blur_radius);
    b.wire.glass_tint[0] = b.panel_glass.r;
    b.wire.glass_tint[1] = b.panel_glass.g;
    b.wire.glass_tint[2] = b.panel_glass.b;
    b.wire.glass_tint[3] = b.panel_glass.a;
    b.wire.panel_opacity = b.panel.panel_opacity;
    b.wire.dialog_opacity = b.dialog.dialog_opacity;
    b.wire.noise_strength = b.panel.noise;
    b.wire.saturation_boost = b.panel.saturation;
    return b;
}

[[nodiscard]] inline ThemeBundle from_kwin_compositing() noexcept {
    ThemeBundle b{};
    b.panel = ::pbsd::kde::kwin::compositing::panel_blur();
    b.dialog = ::pbsd::kde::kwin::compositing::dialog_blur();
    b.panel_glass = ::pbsd::theme::plasma::aero::glass::panel();
    b.wire = wayland::AeroDefaults{};
    return b;
}

[[nodiscard]] inline Status validate_theme_id(const char* id) noexcept {
    return ::pbsd::kde::plasma::aero::validate_theme_id(id);
}

} // namespace pbsd::compositor::aero
