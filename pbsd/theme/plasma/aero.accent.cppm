export module pbsd.theme.plasma.aero.accent;

import pbsd.core;
import pbsd.kde.plasma.colors;

/// Wave 3 pass 6 — PBSD Aero theme constants (aero.accent.cppm).
export namespace pbsd::theme::plasma::aero::accent {

    /// Windows 7 Aero accent strip — PBSD brand blue only.
    inline constexpr unsigned char kPrimaryR{0x50};
    inline constexpr unsigned char kPrimaryG{0x82};
    inline constexpr unsigned char kPrimaryB{0xc8};
    inline constexpr unsigned char kHighlightR{0xb8};
    inline constexpr unsigned char kHighlightG{0xd8};
    inline constexpr unsigned char kHighlightB{0xff};
    enum class Strip : unsigned char { TitleBar, StartOrb, TaskPreview, Selection };

} // namespace pbsd::theme::plasma::aero::accent
