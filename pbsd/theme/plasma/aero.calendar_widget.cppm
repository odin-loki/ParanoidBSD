export module pbsd.theme.plasma.aero.calendar_widget;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Aero calendar widget visual tokens.
export namespace pbsd::theme::plasma::aero::calendar_widget {

struct CalendarTokens {
    unsigned cell_size_px{32};
    unsigned header_font_px{14};
    float weekend_opacity{0.75f};
};

[[nodiscard]] inline CalendarTokens default_tokens() noexcept {
    return CalendarTokens{};
}

} // namespace pbsd::theme::plasma::aero::calendar_widget
