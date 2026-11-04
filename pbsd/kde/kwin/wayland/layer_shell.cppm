module;

export module pbsd.kde.kwin.wayland.layer_shell;

import pbsd.core;
import pbsd.kde.plasma.panel;
import pbsd.kde.kwin.aero.panel;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 7 — zwlr_layer_shell_v1 panel placement glue.
/// Upstream: kde/kwin/src/wayland/layershellv1.cpp
export namespace pbsd::kde::kwin::wayland::layer_shell {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_LAYER_SHELL",
    "kwin_layer_shell",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/layershellv1.cpp",
};

inline constexpr const char kInterface[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";
inline constexpr int kVersion = 4;

enum class Layer : unsigned char {
    Background = 0,
    Bottom     = 1,
    Top        = 2,
    Overlay    = 3,
};

enum class Anchor : unsigned char {
    None   = 0,
    Top    = 1,
    Bottom = 2,
    Left   = 4,
    Right  = 8,
};

struct PanelLayerConfig {
    Layer layer{Layer::Top};
    Anchor anchor{Anchor::Bottom};
    int exclusive_zone{aero::panel::kPanelHeight};
    int margin{0};
    float opacity{aero::panel::kPanelOpacity};
};

[[nodiscard]] inline Anchor anchor_from_edge(aero::panel::Edge edge) noexcept {
    switch (edge) {
    case aero::panel::Edge::Top:    return Anchor::Top;
    case aero::panel::Edge::Bottom: return Anchor::Bottom;
    case aero::panel::Edge::Left:   return Anchor::Left;
    case aero::panel::Edge::Right:  return Anchor::Right;
    }
    return Anchor::Bottom;
}

[[nodiscard]] inline PanelLayerConfig default_panel_config() noexcept {
    PanelLayerConfig cfg{};
    cfg.anchor = anchor_from_edge(aero::panel::kDefaultEdge);
    cfg.exclusive_zone = plasma::panel::kDefaultHeight;
    cfg.opacity = plasma::panel::kDefaultOpacity;
    return cfg;
}

[[nodiscard]] inline const char* shell_surface_name() noexcept {
    return kShellSurface;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/layershellv1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::layer_shell
