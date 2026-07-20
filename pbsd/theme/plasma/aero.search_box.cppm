export module pbsd.theme.plasma.aero.search_box;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero start menu search box tokens.
export namespace pbsd::theme::plasma::aero::search_box {

inline constexpr const char kSearchBoxSvg[] = "plasma/panel/search-box.svg";
inline constexpr const char kPlaceholder[] = "Search programs and files";
inline constexpr int kHeight{28};
inline constexpr int kCornerRadius{4};
inline constexpr float kGlassOpacity{0.72f};

[[nodiscard]] inline Status validate_height(int h) noexcept {
    if (h < 20 || h > 40) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* panel_dir() noexcept {
    return ::pbsd::kde::plasma::aero::kPanelDir;
}

} // namespace pbsd::theme::plasma::aero::search_box
