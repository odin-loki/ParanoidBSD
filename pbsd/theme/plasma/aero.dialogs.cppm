export module pbsd.theme.plasma.aero.dialogs;

import pbsd.core;
import pbsd.kde.plasma.dialogs;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 6 — PBSD Aero theme constants (aero.dialogs.cppm).
export namespace pbsd::theme::plasma::aero::dialogs {

    using Kind = ::pbsd::kde::plasma::dialogs::Kind;
    inline constexpr float kOpacity{0.78f};
    inline constexpr int kCornerRadius{8};
    inline constexpr int kTitleBarHeight{36};
    enum class Backdrop : unsigned char { Glass, Solid, Mica };
    inline constexpr Backdrop kDefaultBackdrop{Backdrop::Glass};

} // namespace pbsd::theme::plasma::aero::dialogs
