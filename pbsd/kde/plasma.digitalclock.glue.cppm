export module pbsd.kde.plasma.digitalclock.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.digitalclock;
import pbsd.kde.plasma.panel;

/// Burst 12 — Digital clock applet ↔ Aero panel typography glue.
/// Upstream: kde/plasma-desktop/applets/digital-clock/
export namespace pbsd::kde::plasma::digitalclock::glue {

struct ClockLayout {
    const char* clock_svg{digitalclock::kClockSvg};
    const char* date_format{digitalclock::kDateFormat};
    const char* time_format{digitalclock::kTimeFormat};
    const char* show_desktop_svg{aero::kShowDesktopSvg};
    int panel_height{panel::kDefaultHeight};
    int font_size{13};
};

[[nodiscard]] inline ClockLayout default_layout() noexcept {
    return ClockLayout{};
}

[[nodiscard]] inline int clock_slot_width(int font_size) noexcept {
    if (font_size <= 0) {
        return 72;
    }
    return font_size * 6;
}

[[nodiscard]] inline Status validate_font_size(int px) noexcept {
    if (px < 10 || px > 24) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return digitalclock::upstream_path();
}

} // namespace pbsd::kde::plasma::digitalclock::glue
