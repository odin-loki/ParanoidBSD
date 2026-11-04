module;

export module pbsd.kde.plasma.wayland.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.panel;
import pbsd.kde.plasma.shell;

/// Wave 3 pass 7 — Plasma panel ↔ zwlr_layer_shell_v1 anchor glue.
/// Upstream: kde/plasma-workspace/shell/shell.cpp + layer-shell-qt integration.
export namespace pbsd::kde::plasma::wayland::glue {

inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

enum class PanelAnchor : unsigned char {
    Top    = 1,
    Bottom = 2,
    Left   = 4,
    Right  = 8,
};

[[nodiscard]] inline PanelAnchor anchor_from_panel_position(
    panel::Position pos) noexcept {
    switch (pos) {
    case panel::Position::Top:    return PanelAnchor::Top;
    case panel::Position::Bottom: return PanelAnchor::Bottom;
    case panel::Position::Left:   return PanelAnchor::Left;
    case panel::Position::Right:  return PanelAnchor::Right;
    }
    return PanelAnchor::Bottom;
}

struct LayerSurfaceConfig {
    PanelAnchor anchor{PanelAnchor::Bottom};
    int exclusive_zone{panel::kDefaultHeight};
    int margin{0};
    bool keyboard_interactivity{false};
    const char* shell_surface{kShellSurface};
};

[[nodiscard]] inline LayerSurfaceConfig default_panel_layer() noexcept {
    LayerSurfaceConfig cfg{};
    cfg.anchor = PanelAnchor::Bottom;
    cfg.exclusive_zone = panel::kDefaultHeight;
    cfg.margin = 0;
    return cfg;
}

[[nodiscard]] inline float panel_opacity() noexcept {
    return panel::kDefaultOpacity;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/shell/shell.cpp";
}

} // namespace pbsd::kde::plasma::wayland::glue
