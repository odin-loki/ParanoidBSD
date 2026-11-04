export module pbsd.kde.plasma.notifications;

import pbsd.core;

/// Wave 3 pass 5 — Plasma notification manager limits.
/// Upstream: kde/plasma-workspace/libnotificationmanager/notificationsmodel.cpp
export namespace pbsd::kde::plasma::notifications {

    inline constexpr const char kNotificationSvg[] = "plasma/widgets/notification.svg";
    inline constexpr unsigned kMaxVisible{5};
    inline constexpr unsigned kDefaultTimeoutMs{5000};
    enum class Urgency : unsigned char { Low, Normal, Critical };

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/libnotificationmanager/notificationsmodel.cpp";
}

} // namespace pbsd::kde::plasma::notifications
