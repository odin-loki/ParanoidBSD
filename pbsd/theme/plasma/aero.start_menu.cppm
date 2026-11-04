export module pbsd.theme.plasma.aero.start_menu;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.startmenu;

/// Wave 3 pass 5 — PBSD Aero theme constants (aero.start_menu.cppm).
export namespace pbsd::theme::plasma::aero::start_menu {

    inline constexpr int kWidth{480};
    inline constexpr int kHeight{560};
    inline constexpr int kSidebarWidth{240};
    inline constexpr int kSearchHeight{36};
    inline constexpr int kItemHeight{32};
    inline constexpr float kOpacity{0.78f};
    inline constexpr const char kFrameSvg[] = "plasma/panel/start-menu-frame.svg";
    inline constexpr const char kBackgroundSvg[] = "plasma/panel/start-menu.svg";
    [[nodiscard]] inline ::pbsd::kde::plasma::startmenu::Layout layout() noexcept {
        return ::pbsd::kde::plasma::startmenu::default_layout();
    }

} // namespace pbsd::theme::plasma::aero::start_menu
