export module pbsd.theme.plasma.aero.panel;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 9 burst — Aero panel chrome metrics.
export namespace pbsd::theme::plasma::aero::panel {

inline constexpr int kHeight = 40;
inline constexpr int kIconSize = 32;
inline constexpr int kSpacing = 4;
inline constexpr float kOpacity = 0.72f;

[[nodiscard]] inline int height_from_blur(const ::pbsd::kde::plasma::aero::BlurParams& p) noexcept {
    return p.title_bar_height + kSpacing;
}

} // namespace pbsd::theme::plasma::aero::panel
