export module pbsd.theme.plasma.aero.screen_reader;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 wave 2 — Aero screen reader visual tokens.
export namespace pbsd::theme::plasma::aero::screen_reader {

struct ScreenReaderTokens {
    unsigned caret_width_px{2};
    float highlight_opacity{0.40f};
    const char* focus_svg{"plasma/widgets/focus-ring.svg"};
};

[[nodiscard]] inline ScreenReaderTokens default_tokens() noexcept {
    return ScreenReaderTokens{};
}

} // namespace pbsd::theme::plasma::aero::screen_reader
