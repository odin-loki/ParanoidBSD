export module pbsd.theme.plasma.aero.ribbon;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 wave 2 — Aero ribbon visual tokens.
export namespace pbsd::theme::plasma::aero::ribbon {

struct RibbonTokens {
    unsigned tab_height_px{28};
    unsigned group_spacing_px{8};
    const char* tab_svg{"plasma/widgets/tab.svg"};
};

[[nodiscard]] inline RibbonTokens default_tokens() noexcept {
    return RibbonTokens{};
}

} // namespace pbsd::theme::plasma::aero::ribbon
