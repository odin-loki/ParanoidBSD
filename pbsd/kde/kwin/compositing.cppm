export module pbsd.kde.kwin.compositing;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 — compositor glass/blur bridge constants.
export namespace pbsd::kde::kwin::compositing {

inline constexpr const char kCompositorBackend[] = "wayland";
inline constexpr bool kBlurBehindPanels{true};

[[nodiscard]] inline plasma::aero::BlurParams panel_blur() noexcept {
    auto p = plasma::aero::default_blur();
    p.panel_opacity = 0.72f;
    return p;
}

[[nodiscard]] inline plasma::aero::BlurParams dialog_blur() noexcept {
    auto p = plasma::aero::default_blur();
    p.dialog_opacity = 0.78f;
    return p;
}

[[nodiscard]] inline plasma::aero::BlurParams start_menu_blur() noexcept {
    auto p = dialog_blur();
    p.dialog_opacity = 0.78f;
    p.corner_radius = 8;
    return p;
}

[[nodiscard]] inline plasma::aero::BlurParams decoration_blur() noexcept {
    auto p = plasma::aero::default_blur();
    p.panel_opacity = 0.78f;
    p.title_bar_height = 36;
    return p;
}

[[nodiscard]] inline plasma::aero::BlurParams glow_blur() noexcept {
    auto p = decoration_blur();
    p.saturation = 1.20f;
    p.contrast = 1.08f;
    p.noise = 0.03f;
    return p;
}

[[nodiscard]] inline plasma::aero::BlurParams system_link_blur() noexcept {
    auto p = panel_blur();
    p.panel_opacity = 0.68f;
    return p;
}

} // namespace pbsd::kde::kwin::compositing
