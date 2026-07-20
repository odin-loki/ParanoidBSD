export module pbsd.theme.plasma.aero.layershell;

import pbsd.core;
import pbsd.kde.plasma.panel;
import pbsd.kde.kwin.wayland.layer_shell;

/// Wave 3 pass 7 — Aero panel layer-shell visual tokens (anchor + glass).
export namespace pbsd::theme::plasma::aero::layershell {

struct PanelTokens {
    int height{::pbsd::kde::plasma::panel::kDefaultHeight};
    float opacity{::pbsd::kde::plasma::panel::kDefaultOpacity};
    int blur_radius{::pbsd::kde::plasma::panel::kDefaultBlurRadius};
    const char* glass_asset{"plasma/panel/panel-glass.svg"};
    ::pbsd::kde::kwin::wayland::layer_shell::Anchor anchor{
        ::pbsd::kde::kwin::wayland::layer_shell::Anchor::Bottom};
};

[[nodiscard]] inline PanelTokens default_bottom_panel() noexcept {
    PanelTokens t{};
    t.height = ::pbsd::kde::plasma::panel::kDefaultHeight;
    t.opacity = ::pbsd::kde::plasma::panel::kDefaultOpacity;
    t.blur_radius = ::pbsd::kde::plasma::panel::kDefaultBlurRadius;
    t.anchor = ::pbsd::kde::kwin::wayland::layer_shell::Anchor::Bottom;
    return t;
}

[[nodiscard]] inline PanelTokens from_kwin_config(
    ::pbsd::kde::kwin::wayland::layer_shell::PanelLayerConfig cfg) noexcept {
    PanelTokens t{};
    t.height = cfg.exclusive_zone;
    t.opacity = cfg.opacity;
    t.anchor = cfg.anchor;
    return t;
}

} // namespace pbsd::theme::plasma::aero::layershell
