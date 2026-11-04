export module pbsd.theme.plasma.aero.user_tile;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero start menu user tile tokens.
export namespace pbsd::theme::plasma::aero::user_tile {

inline constexpr const char kUserTileSvg[] = "plasma/panel/user-tile.svg";
inline constexpr int kTileHeight{48};
inline constexpr int kAvatarSize{32};
inline constexpr float kOpacity{0.82f};

[[nodiscard]] inline Status validate_tile_height(int h) noexcept {
    if (h < 32 || h > 64) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* asset_path() noexcept {
    return ::pbsd::kde::plasma::aero::kUserTileSvg;
}

} // namespace pbsd::theme::plasma::aero::user_tile
