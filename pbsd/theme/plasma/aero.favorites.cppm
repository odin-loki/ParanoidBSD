export module pbsd.theme.plasma.aero.favorites;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero start menu favorites pane tokens.
export namespace pbsd::theme::plasma::aero::favorites {

inline constexpr const char kFavoritesSvg[] = "plasma/panel/favorites.svg";
inline constexpr const char kRecentSvg[] = "plasma/panel/recent.svg";
inline constexpr unsigned kMaxPinned{12};
inline constexpr float kLinkOpacity{0.68f};

[[nodiscard]] inline Status validate_pin_count(unsigned count) noexcept {
    return count <= kMaxPinned ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* favorites_path() noexcept {
    return ::pbsd::kde::plasma::aero::kFavoritesSvg;
}

[[nodiscard]] inline const char* recent_path() noexcept {
    return ::pbsd::kde::plasma::aero::kRecentSvg;
}

} // namespace pbsd::theme::plasma::aero::favorites
