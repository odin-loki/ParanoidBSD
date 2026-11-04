export module pbsd.theme.plasma.aero.desktop_icons;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 15 — Aero desktop icons visual tokens.
export namespace pbsd::theme::plasma::aero::desktop_icons {

struct DesktopIconTokens {
    unsigned grid_size_px{96};
    unsigned label_font_px{12};
    float selection_opacity{0.45f};
};

[[nodiscard]] inline DesktopIconTokens default_tokens() noexcept {
    return DesktopIconTokens{};
}

} // namespace pbsd::theme::plasma::aero::desktop_icons
