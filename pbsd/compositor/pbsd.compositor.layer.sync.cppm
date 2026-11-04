export module pbsd.compositor.layer.sync;

import pbsd.core;
import pbsd.compositor.wayland.layer;
import pbsd.kde.plasma.layershell.glue;
import pbsd.kde.plasma.wayland.glue;
import pbsd.kde.kwin.wayland.layer_shell;

/// Burst 12 — Native compositor layer shell ↔ KDE plasma/kwin glue sync.
export namespace pbsd::compositor::layer::sync {

struct LayerWire {
    wayland::layer::Anchor native_anchor{wayland::layer::Anchor::Bottom};
    wayland::layer::Layer native_layer{wayland::layer::Layer::Top};
    int exclusive_zone{::pbsd::kde::plasma::layershell::glue::default_panel_layer().exclusive_zone};
    float opacity{::pbsd::kde::plasma::layershell::glue::default_panel_layer().opacity};
};

[[nodiscard]] inline wayland::layer::Anchor to_native(
    ::pbsd::kde::plasma::layershell::glue::PanelLayer pl) noexcept {
    switch (pl.anchor) {
    case ::pbsd::kde::plasma::wayland::glue::PanelAnchor::Top:
        return wayland::layer::Anchor::Top;
    case ::pbsd::kde::plasma::wayland::glue::PanelAnchor::Left:
        return wayland::layer::Anchor::Left;
    case ::pbsd::kde::plasma::wayland::glue::PanelAnchor::Right:
        return wayland::layer::Anchor::Right;
    default:
        return wayland::layer::Anchor::Bottom;
    }
}

[[nodiscard]] inline wayland::layer::Layer to_native_layer(
    ::pbsd::kde::plasma::layershell::glue::Layer layer) noexcept {
    switch (layer) {
    case ::pbsd::kde::plasma::layershell::glue::Layer::Background:
        return wayland::layer::Layer::Background;
    case ::pbsd::kde::plasma::layershell::glue::Layer::Bottom:
        return wayland::layer::Layer::Bottom;
    case ::pbsd::kde::plasma::layershell::glue::Layer::Overlay:
        return wayland::layer::Layer::Overlay;
    default:
        return wayland::layer::Layer::Top;
    }
}

[[nodiscard]] inline LayerWire from_plasma_glue() noexcept {
    LayerWire w{};
    const auto pl = ::pbsd::kde::plasma::layershell::glue::default_panel_layer();
    w.native_anchor = to_native(pl);
    w.native_layer = to_native_layer(pl.layer);
    w.exclusive_zone = pl.exclusive_zone;
    w.opacity = pl.opacity;
    return w;
}

[[nodiscard]] inline LayerWire from_kwin_config() noexcept {
    LayerWire w = from_plasma_glue();
    const auto cfg = ::pbsd::kde::kwin::wayland::layer_shell::default_panel_config();
    switch (cfg.anchor) {
    case ::pbsd::kde::kwin::wayland::layer_shell::Anchor::Top:
        w.native_anchor = wayland::layer::Anchor::Top;
        break;
    case ::pbsd::kde::kwin::wayland::layer_shell::Anchor::Left:
        w.native_anchor = wayland::layer::Anchor::Left;
        break;
    case ::pbsd::kde::kwin::wayland::layer_shell::Anchor::Right:
        w.native_anchor = wayland::layer::Anchor::Right;
        break;
    default:
        w.native_anchor = wayland::layer::Anchor::Bottom;
        break;
    }
    w.exclusive_zone = cfg.exclusive_zone;
    w.opacity = cfg.opacity;
    return w;
}

[[nodiscard]] inline Status validate_wire(const LayerWire& w) noexcept {
    if (::pbsd::kde::plasma::layershell::glue::validate_exclusive_zone(w.exclusive_zone) != Status::Ok) {
        return Status::Invalid;
    }
    if (w.opacity < 0.0f || w.opacity > 1.0f) {
        return Status::Invalid;
    }
    return wayland::layer::validate_anchor(static_cast<unsigned>(w.native_anchor));
}

} // namespace pbsd::compositor::layer::sync
