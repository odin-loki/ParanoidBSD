export module pbsd.theme.plasma.aero.decoration_borders;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.theme.plasma.aero.borders;

/// Burst 13 — Aero KDecoration3 border SVG asset tokens.
export namespace pbsd::theme::plasma::aero::decoration_borders {

inline constexpr const char kBorderLeftSvg[] = "plasma/decoration/border-left.svg";
inline constexpr const char kBorderRightSvg[] = "plasma/decoration/border-right.svg";
inline constexpr const char kBorderTopSvg[] = "plasma/decoration/border-top.svg";
inline constexpr const char kBorderBottomSvg[] = "plasma/decoration/border-bottom.svg";
inline constexpr const char kResizeGripSvg[] = "plasma/decoration/resize-grip.svg";

[[nodiscard]] inline borders::Border window_border() noexcept {
    return borders::kWindowBorder;
}

[[nodiscard]] inline Status validate_grip_size(int px) noexcept {
    if (px < 4 || px > 16) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* decoration_id() noexcept {
    return ::pbsd::kde::plasma::aero::kDecorationId;
}

} // namespace pbsd::theme::plasma::aero::decoration_borders
