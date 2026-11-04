export module pbsd.theme.plasma.aero.notification;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 9 burst — Aero notification popup metrics.
export namespace pbsd::theme::plasma::aero::notification {

inline constexpr int kWidth = 360;
inline constexpr int kMinHeight = 80;
inline constexpr int kCornerRadius = 8;
inline constexpr float kOpacity = 0.85f;

[[nodiscard]] inline float opacity_from_blur(const ::pbsd::kde::plasma::aero::BlurParams& p) noexcept {
    return p.dialog_opacity;
}

} // namespace pbsd::theme::plasma::aero::notification
