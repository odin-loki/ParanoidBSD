export module pbsd.theme.plasma.aero.sidebar;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero start menu sidebar chrome tokens.
export namespace pbsd::theme::plasma::aero::sidebar {

inline constexpr const char kSidebarSvg[] = "plasma/panel/sidebar.svg";
inline constexpr int kWidth{180};
inline constexpr float kGlassOpacity{0.68f};

[[nodiscard]] inline Status validate_width(int w) noexcept {
    if (w < 120 || w > 240) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* asset_path() noexcept {
    return ::pbsd::kde::plasma::aero::kSidebarSvg;
}

} // namespace pbsd::theme::plasma::aero::sidebar
