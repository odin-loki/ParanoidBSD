export module pbsd.theme.plasma.aero.caption_buttons;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero window caption button asset tokens.
export namespace pbsd::theme::plasma::aero::caption_buttons {

inline constexpr const char kMinimizeSvg[] = "plasma/decoration/minimize.svg";
inline constexpr const char kMaximizeSvg[] = "plasma/decoration/maximize.svg";
inline constexpr const char kRestoreSvg[] = "plasma/decoration/restore.svg";
inline constexpr const char kCloseSvg[] = "plasma/decoration/close.svg";
inline constexpr const char kShadeSvg[] = "plasma/decoration/shade.svg";
inline constexpr const char kCaptionIconSvg[] = "plasma/decoration/caption-icon.svg";

inline constexpr int kButtonWidth{30};
inline constexpr int kButtonHeight{22};
inline constexpr float kHoverOpacity{0.85f};

[[nodiscard]] inline Status validate_button_size(int w, int h) noexcept {
    if (w < 16 || h < 16 || w > 48 || h > 32) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* decoration_dir() noexcept {
    return ::pbsd::kde::plasma::aero::kDecorationDir;
}

} // namespace pbsd::theme::plasma::aero::caption_buttons
