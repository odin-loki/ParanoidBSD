export module pbsd.kde.plasma.startmenu;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 — Aero Start menu layout constants (panel/start-menu.md).
export namespace pbsd::kde::plasma::startmenu {

inline constexpr const char kNotesPath[] = "plasma/panel/start-menu.md";
inline constexpr const char kBackgroundSvg[] = "plasma/panel/start-menu.svg";
inline constexpr const char kApplicationsSvg[] = "plasma/panel/applications.svg";
inline constexpr const char kMusicSvg[] = "plasma/panel/music.svg";
inline constexpr const char kComputerSvg[] = "plasma/panel/computer.svg";
inline constexpr const char kFavoritesSvg[] = "plasma/panel/favorites.svg";
inline constexpr const char kAllProgramsSvg[] = "plasma/panel/all-programs.svg";
inline constexpr const char kPowerButtonSvg[] = "plasma/panel/power-button.svg";
inline constexpr const char kLockSvg[] = "plasma/panel/lock.svg";
inline constexpr unsigned kSystemLinkCount{9};
inline constexpr unsigned kPowerActionCount{5};
inline constexpr int default_width{480};
inline constexpr int default_height{560};
inline constexpr int search_height{36};
inline constexpr int item_height{32};
inline constexpr int sidebar_width{240};
inline constexpr int user_tile_height{64};
inline constexpr const char kSearchBoxSvg[] = "plasma/panel/search-box.svg";
inline constexpr const char kShutdownSvg[] = "plasma/panel/shutdown.svg";
inline constexpr const char kUserTileSvg[] = "plasma/panel/user-tile.svg";
inline constexpr const char kSidebarSvg[] = "plasma/panel/sidebar.svg";
inline constexpr const char kFrameSvg[] = "plasma/panel/start-menu-frame.svg";
inline constexpr const char kDocumentsSvg[] = "plasma/panel/documents.svg";
inline constexpr const char kPicturesSvg[] = "plasma/panel/pictures.svg";
inline constexpr const char kControlPanelSvg[] = "plasma/panel/control-panel.svg";
inline constexpr const char kHoverHighlightSvg[] = "plasma/widgets/hover-highlight.svg";

struct Layout {
    int width{default_width};
    int height{default_height};
    int sidebar{sidebar_width};
    float blur_radius{static_cast<float>(aero::default_blur().blur_radius)};
    float opacity{0.78f};
};

[[nodiscard]] inline Layout default_layout() noexcept { return Layout{}; }

} // namespace pbsd::kde::plasma::startmenu
