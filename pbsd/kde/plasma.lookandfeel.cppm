export module pbsd.kde.plasma.lookandfeel;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 — look-and-feel package keys (from landingpage KCM).
/// Upstream: kde/plasma-desktop/kcms/landingpage/lookandfeelmetadata.cpp
export namespace pbsd::kde::plasma::lookandfeel {

inline constexpr const char kPackageId[] = "org.kde.lookandfeel";
inline constexpr const char kThemeColorsKey[] = "theme-colors.json";
inline constexpr const char kColorSchemeKey[] = "colors";

[[nodiscard]] inline const char* aero_theme_id() noexcept { return aero::kThemeId; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/landingpage/lookandfeelmetadata.cpp";
}

} // namespace pbsd::kde::plasma::lookandfeel
