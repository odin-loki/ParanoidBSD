export module pbsd.theme.plasma.aero.notifications;

import pbsd.core;
import pbsd.kde.plasma.notifications;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 6 — PBSD Aero theme constants (aero.notifications.cppm).
export namespace pbsd::theme::plasma::aero::notifications {

    inline constexpr float kGlassOpacity{0.78f};
    inline constexpr unsigned kMaxVisible{5};
    inline constexpr unsigned kTimeoutMs{5000};
    inline constexpr const char kNotificationSvg[] = "plasma/widgets/notification.svg";
    enum class Placement : unsigned char { TopRight, BottomRight, TopLeft, BottomLeft };
    inline constexpr Placement kDefaultPlacement{Placement::BottomRight};

} // namespace pbsd::theme::plasma::aero::notifications
