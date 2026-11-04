export module pbsd.kde.plasma.layershell.glue;

import pbsd.core;
import pbsd.kde.plasma.panel;
import pbsd.kde.plasma.wayland.glue;

/// Burst 15 — Plasma panel ↔ zwlr_layer_shell_v1.
/// Upstream: kde/plasma-workspace/shell/shell.cpp + layer-shell-qt
/// Note: layershellintegration lives in pbsd_kde_frameworks; plasma_aero cannot
/// import it without a CMake cycle (frameworks already depends on plasma_aero).
export namespace pbsd::kde::plasma::layershell::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";
inline constexpr int kLayerShellVersion{4};

enum class Layer : unsigned char {
    Background = 0,
    Bottom     = 1,
    Top        = 2,
    Overlay    = 3,
};

struct PanelLayer {
    wayland::glue::PanelAnchor anchor{wayland::glue::PanelAnchor::Bottom};
    Layer layer{Layer::Top};
    int exclusive_zone{panel::kDefaultHeight};
    float opacity{panel::kDefaultOpacity};
    const char* shell_surface{kShellSurface};
};

[[nodiscard]] inline PanelLayer default_panel_layer() noexcept {
    PanelLayer pl{};
    pl.anchor = wayland::glue::PanelAnchor::Bottom;
    pl.exclusive_zone = panel::kDefaultHeight;
    pl.opacity = panel::kDefaultOpacity;
    return pl;
}

[[nodiscard]] inline PanelLayer from_wayland_glue(
    wayland::glue::LayerSurfaceConfig cfg) noexcept {
    PanelLayer pl{};
    pl.anchor = cfg.anchor;
    pl.exclusive_zone = cfg.exclusive_zone;
    pl.shell_surface = cfg.shell_surface;
    return pl;
}

[[nodiscard]] inline Status validate_exclusive_zone(int zone) noexcept {
    if (zone < 0 || zone > 512) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return wayland::glue::upstream_path();
}

} // namespace pbsd::kde::plasma::layershell::glue
