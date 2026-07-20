export module pbsd.theme.plasma.aero.wallpaper;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 11 — Aero desktop wallpaper blur/dim overlay tokens.
export namespace pbsd::theme::plasma::aero::wallpaper {

struct WallpaperOverlay {
    float dim_opacity{0.15f};
    float blur_radius{8.0f};
    float saturation{1.05f};
    const char* color_scheme{::pbsd::kde::plasma::aero::kColorScheme};
};

inline constexpr WallpaperOverlay kDefault{};
inline constexpr WallpaperOverlay kLockScreen{0.35f, 16.0f, 0.95f,
    ::pbsd::kde::plasma::aero::kColorScheme};
inline constexpr const char kWallpaperDir[] = "plasma/wallpapers";
inline constexpr const char kDefaultWallpaper[] = "plasma/wallpapers/pbsd-aero-default.svg";

[[nodiscard]] inline WallpaperOverlay for_lock_screen() noexcept {
    return kLockScreen;
}

[[nodiscard]] inline float effective_dim(float user_dim) noexcept {
    if (user_dim < 0.0f) {
        return kDefault.dim_opacity;
    }
    if (user_dim > 0.8f) {
        return 0.8f;
    }
    return user_dim;
}

[[nodiscard]] inline Status validate_blur(float radius) noexcept {
    if (radius < 0.0f || radius > 48.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::wallpaper
