export module pbsd.theme.plasma.aero.battery_indicator;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 15 — Aero battery indicator visual tokens.
export namespace pbsd::theme::plasma::aero::battery_indicator {

struct BatteryTokens {
    unsigned icon_size_px{16};
    float low_threshold{0.15f};
    const char* battery_svg{"plasma/panel/battery.svg"};
};

[[nodiscard]] inline BatteryTokens default_tokens() noexcept {
    return BatteryTokens{};
}

} // namespace pbsd::theme::plasma::aero::battery_indicator
