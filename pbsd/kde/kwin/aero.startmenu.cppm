module;

export module pbsd.kde.kwin.aero.startmenu;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.startmenu;
import pbsd.kde.kwin.compositing;
import pbsd.kde.kwin.aero.glow;

/// Wave 3 pass 7 — KWin start-menu popup blur/frame glue.
/// Upstream: pbsd/theme/plasma/panel/start-menu.md + blur-effect.json
export namespace pbsd::kde::kwin::aero::startmenu {

inline constexpr int kWidth{plasma::startmenu::default_width};
inline constexpr int kHeight{plasma::startmenu::default_height};
inline constexpr int kSearchHeight{plasma::startmenu::search_height};
inline constexpr float kOpacity{0.78f};
inline constexpr const char kFrameSvg[] = "plasma/panel/start-menu-frame.svg";
inline constexpr const char kSearchBoxSvg[] = "plasma/panel/search-box.svg";

struct PopupConfig {
    int width{kWidth};
    int height{kHeight};
    int search_height{kSearchHeight};
    float opacity{kOpacity};
    plasma::aero::BlurParams blur{};
};

[[nodiscard]] inline PopupConfig default_popup() noexcept {
    PopupConfig cfg{};
    cfg.blur = compositing::start_menu_blur();
    cfg.opacity = cfg.blur.dialog_opacity;
    return cfg;
}

[[nodiscard]] inline plasma::aero::BlurParams blur_with_glow(bool active) noexcept {
    auto p = compositing::start_menu_blur();
    if (active) {
        auto g = glow::params(glow::State::Active);
        p.saturation = g.saturation;
    }
    return p;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return plasma::startmenu::kNotesPath;
}

} // namespace pbsd::kde::kwin::aero::startmenu
