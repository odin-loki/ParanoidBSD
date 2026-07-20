export module pbsd.kde.plasma.quickaccess;

import pbsd.core;

/// Wave 3 pass 6 — Start menu quick access links (layout.json quickAccess).
/// Upstream: pbsd/theme/plasma/panel/layout.json
export namespace pbsd::kde::plasma::quickaccess {

    enum class Entry : unsigned char {
        Favorites, Recent, AllPrograms, Help, Settings, Run, Search
    };
    inline constexpr unsigned kCount{7};
    inline constexpr const char kFavoritesSvg[] = "plasma/panel/favorites.svg";
    inline constexpr const char kRecentSvg[] = "plasma/panel/recent.svg";
    inline constexpr const char kAllProgramsSvg[] = "plasma/panel/all-programs.svg";
    inline constexpr const char kHelpSvg[] = "plasma/panel/help.svg";
    inline constexpr const char kSettingsSvg[] = "plasma/panel/settings.svg";
    inline constexpr const char kRunSvg[] = "plasma/panel/run.svg";
    inline constexpr const char kSearchSvg[] = "plasma/panel/search.svg";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/plasma/panel/layout.json";
}

} // namespace pbsd::kde::plasma::quickaccess
