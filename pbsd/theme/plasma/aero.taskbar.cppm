export module pbsd.theme.plasma.aero.taskbar;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.panel;

/// Wave 3 pass 5 — PBSD Aero theme constants (aero.taskbar.cppm).
export namespace pbsd::theme::plasma::aero::taskbar {

    inline constexpr int kHeight{40};
    inline constexpr int kIconSize{32};
    inline constexpr float kOpacity{0.72f};
    inline constexpr int kBlurRadius{24};
    inline constexpr const char kTaskbarSvg[] = "plasma/panel/taskbar.svg";
    inline constexpr const char kStartButtonSvg[] = "plasma/panel/start-button.svg";
    inline constexpr const char kShowDesktopSvg[] = "plasma/panel/show-desktop.svg";
    inline constexpr const char kClockSvg[] = "plasma/panel/clock.svg";
    enum class Alignment : unsigned char { Left, Center, Right };
    inline constexpr Alignment kDefaultAlignment{Alignment::Center};

} // namespace pbsd::theme::plasma::aero::taskbar
