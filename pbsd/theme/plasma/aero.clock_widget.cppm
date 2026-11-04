export module pbsd.theme.plasma.aero.clock_widget;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 14 — Aero clock widget visual tokens.
export namespace pbsd::theme::plasma::aero::clock_widget {

struct ClockTokens {
    const char* time_format{"HH:mm"};
    const char* date_format{"ddd, MMM d"};
    unsigned font_px{14};
};

[[nodiscard]] inline ClockTokens default_tokens() noexcept {
    return ClockTokens{};
}

} // namespace pbsd::theme::plasma::aero::clock_widget
