export module pbsd.theme.plasma.aero.brightness_slider;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Aero brightness slider visual tokens.
export namespace pbsd::theme::plasma::aero::brightness_slider {

struct BrightnessTokens {
    unsigned slider_width_px{140};
    float dim_opacity{0.40f};
    const char* brightness_svg{"plasma/panel/brightness.svg"};
};

[[nodiscard]] inline BrightnessTokens default_tokens() noexcept {
    return BrightnessTokens{};
}

} // namespace pbsd::theme::plasma::aero::brightness_slider
