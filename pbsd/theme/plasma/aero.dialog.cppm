export module pbsd.theme.plasma.aero.dialog;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 9 burst — Aero dialog frame metrics.
export namespace pbsd::theme::plasma::aero::dialog {

inline constexpr int kTitleHeight = 32;
inline constexpr int kButtonSize = 24;
inline constexpr int kPadding = 12;
inline constexpr int kCornerRadius = 8;

[[nodiscard]] inline int corner_from_blur(const ::pbsd::kde::plasma::aero::BlurParams& p) noexcept {
    return p.corner_radius;
}

} // namespace pbsd::theme::plasma::aero::dialog
