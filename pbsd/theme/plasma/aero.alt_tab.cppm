export module pbsd.theme.plasma.aero.alt_tab;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 15 — Aero alt tab visual tokens.
export namespace pbsd::theme::plasma::aero::alt_tab {

struct AltTabTokens {
    unsigned thumbnail_width_px{280};
    unsigned animation_ms{200};
    float dim_opacity{0.35f};
    const char* switcher_svg{"plasma/panel/alt-tab.svg"};
};

[[nodiscard]] inline AltTabTokens default_tokens() noexcept {
    return AltTabTokens{};
}

} // namespace pbsd::theme::plasma::aero::alt_tab
