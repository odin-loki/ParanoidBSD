export module pbsd.compositor.theme.sync;

import pbsd.core;
import pbsd.compositor.wayland;
import pbsd.kde.plasma.aero;
import pbsd.theme.plasma.aero.compositing;
import pbsd.theme.plasma.aero.glass;
import pbsd.theme.plasma.aero.transitions;

/// Burst 11 — Native compositor wire ↔ Aero theme module sync.
export namespace pbsd::compositor::theme::sync {

struct ThemeWire {
    wayland::AeroDefaults wire{};
    ::pbsd::kde::plasma::aero::BlurParams panel{};
    ::pbsd::theme::plasma::aero::glass::Material panel_glass{};
    float transition_ms{220.0f};
};

[[nodiscard]] inline ThemeWire from_theme_modules() noexcept {
    ThemeWire t{};
    t.panel = ::pbsd::theme::plasma::aero::compositing::blur(
        ::pbsd::theme::plasma::aero::compositing::Surface::Panel);
    t.panel_glass = ::pbsd::theme::plasma::aero::glass::from_blur_params(t.panel);
    t.transition_ms = ::pbsd::theme::plasma::aero::transitions::kPanelShow.duration_ms;
    t.wire.blur_radius_px = static_cast<float>(t.panel.blur_radius);
    t.wire.glass_tint[0] = t.panel_glass.r;
    t.wire.glass_tint[1] = t.panel_glass.g;
    t.wire.glass_tint[2] = t.panel_glass.b;
    t.wire.glass_tint[3] = t.panel_glass.a;
    t.wire.panel_opacity = t.panel.panel_opacity;
    t.wire.noise_strength = t.panel.noise;
    t.wire.saturation_boost = t.panel.saturation;
    return t;
}

[[nodiscard]] inline Status validate_wire(const wayland::AeroDefaults& w) noexcept {
    if (w.panel_opacity < 0.0f || w.panel_opacity > 1.0f) {
        return Status::Invalid;
    }
    if (w.blur_radius_px < 0.0f || w.blur_radius_px > 64.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool theme_id_matches(const char* id) noexcept {
    return ::pbsd::kde::plasma::aero::theme_id_matches(id);
}

} // namespace pbsd::compositor::theme::sync
