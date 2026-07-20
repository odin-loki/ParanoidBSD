export module pbsd.kde.plasma.packagestructure_wallpaper;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (plasma_wallpaper_packagestructure.cpp).
/// Upstream: kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_wallpaper_packagestructure.cpp
export namespace pbsd::kde::frameworks::plasma::packagestructure_wallpaper {

inline constexpr const char kPluginId[] = "Plasma/Wallpaper";
inline constexpr const char kWallpaperQml[] = "contents/ui/main.qml";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_wallpaper_packagestructure.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::packagestructure_wallpaper
