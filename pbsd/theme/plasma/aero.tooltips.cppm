export module pbsd.theme.plasma.aero.tooltips;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.theme.plasma.aero.shadows;
import pbsd.theme.plasma.aero.typography;

/// Burst 12 — Aero tooltip chrome tokens.
export namespace pbsd::theme::plasma::aero::tooltips {

struct TooltipStyle {
    float opacity{0.92f};
    int corner_radius{4};
    int padding{6};
    int font_size{typography::kDefault.caption};
    shadows::Shadow shadow{shadows::kTooltipShadow};
    const char* background_hex{"#f0f4fa"};
    const char* foreground_hex{"#1a1a1a"};
};

inline constexpr TooltipStyle kDefault{};

[[nodiscard]] inline TooltipStyle for_widget(bool long_text) noexcept {
    TooltipStyle t = kDefault;
    if (long_text) {
        t.font_size = typography::kDefault.body;
        t.padding = 8;
        t.corner_radius = 6;
    }
    return t;
}

[[nodiscard]] inline int max_width(int screen_width) noexcept {
    if (screen_width <= 0) {
        return 320;
    }
    return screen_width / 3;
}

[[nodiscard]] inline Status validate_opacity(float o) noexcept {
    if (o < 0.5f || o > 1.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::tooltips
