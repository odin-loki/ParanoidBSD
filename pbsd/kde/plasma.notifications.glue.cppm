export module pbsd.kde.plasma.notifications.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.notifications;

/// Burst 11 — Notification popup ↔ Aero glass blur glue.
/// Upstream: kde/plasma-workspace/libnotificationmanager/
export namespace pbsd::kde::plasma::notifications::glue {

struct NotificationStyle {
    unsigned max_visible{notifications::kMaxVisible};
    unsigned timeout_ms{notifications::kDefaultTimeoutMs};
    float opacity{0.82f};
    int blur_radius{aero::default_blur().blur_radius};
    const char* icon_svg{notifications::kNotificationSvg};
    const char* widget_svg{aero::kNotificationSvg};
};

[[nodiscard]] inline NotificationStyle default_style() noexcept {
    return NotificationStyle{};
}

[[nodiscard]] inline NotificationStyle for_urgency(notifications::Urgency u) noexcept {
    NotificationStyle s = default_style();
    switch (u) {
    case notifications::Urgency::Low:
        s.opacity = 0.72f;
        s.timeout_ms = 3000;
        break;
    case notifications::Urgency::Critical:
        s.opacity = 0.92f;
        s.timeout_ms = 0;
        s.blur_radius = 28;
        break;
    default:
        break;
    }
    return s;
}

[[nodiscard]] inline Status validate_timeout(unsigned ms) noexcept {
    if (ms > 600'000) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return notifications::upstream_path();
}

} // namespace pbsd::kde::plasma::notifications::glue
