export module pbsd.compositor.kde.bridge;

import pbsd.core;
import pbsd.compositor.wayland;
import pbsd.compositor.wayland.layer;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.compositing;
import pbsd.kde.kwin.wayland.xdg_shell;
import pbsd.kde.kwin.wayland.layer_shell;

/// Wave 3 pass 7 — Native compositor ↔ KDE/KWin Aero glue bridge.
export namespace pbsd::compositor::kde::bridge {

struct AeroSync {
    wayland::AeroDefaults wire{};
    ::pbsd::kde::plasma::aero::BlurParams panel{};
    ::pbsd::kde::plasma::aero::BlurParams dialog{};
};

[[nodiscard]] inline AeroSync from_kde_defaults() noexcept {
    AeroSync s{};
    s.panel = ::pbsd::kde::kwin::compositing::panel_blur();
    s.dialog = ::pbsd::kde::kwin::compositing::dialog_blur();
    s.wire.blur_radius_px = static_cast<float>(s.panel.blur_radius);
    s.wire.glass_tint[0] = s.panel.tint.r;
    s.wire.glass_tint[1] = s.panel.tint.g;
    s.wire.glass_tint[2] = s.panel.tint.b;
    s.wire.glass_tint[3] = s.panel.tint.a;
    s.wire.panel_opacity = s.panel.panel_opacity;
    s.wire.dialog_opacity = s.dialog.dialog_opacity;
    s.wire.noise_strength = s.panel.noise;
    s.wire.saturation_boost = s.panel.saturation;
    return s;
}

[[nodiscard]] inline bool xdg_prefers_server_decoration() noexcept {
    return ::pbsd::kde::kwin::wayland::xdg_shell::prefers_server_decoration(
        ::pbsd::kde::kwin::wayland::xdg_shell::DecorationMode::ServerSide);
}

[[nodiscard]] inline wayland::layer::Anchor panel_anchor_from_kwin() noexcept {
    auto cfg = ::pbsd::kde::kwin::wayland::layer_shell::default_panel_config();
    switch (cfg.anchor) {
    case ::pbsd::kde::kwin::wayland::layer_shell::Anchor::Top:
        return wayland::layer::Anchor::Top;
    case ::pbsd::kde::kwin::wayland::layer_shell::Anchor::Left:
        return wayland::layer::Anchor::Left;
    case ::pbsd::kde::kwin::wayland::layer_shell::Anchor::Right:
        return wayland::layer::Anchor::Right;
    default:
        return wayland::layer::Anchor::Bottom;
    }
}

[[nodiscard]] inline Status validate_layer_opcode(unsigned opcode) noexcept {
    if (wayland::layer::is_layer_shell_request(opcode)) {
        return Status::Ok;
    }
    if (wayland::layer::is_layer_surface_request(opcode)) {
        return Status::Ok;
    }
    return Status::Protocol;
}

} // namespace pbsd::compositor::kde::bridge
