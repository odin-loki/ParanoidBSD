export module pbsd.theme.plasma.aero.window_chrome;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 5 — PBSD Aero theme constants (aero.window_chrome.cppm).
export namespace pbsd::theme::plasma::aero::window_chrome {

    /// KDecoration3 / Aero window chrome metrics (decoration/config.json).
    struct Metrics {
        int title_bar_height{36};
        int button_size{28};
        int border_width{1};
        int corner_radius{8};
        bool blur_enabled{true};
        float glass_opacity{0.78f};
    };
    inline constexpr const char kDecorationId[] = "pbsd_aero";
    inline constexpr const char kColorScheme[] = "PBSDAero";
    inline constexpr const char kTitleBarActiveSvg[] = "plasma/decoration/titlebar-active.svg";
    inline constexpr const char kTitleBarInactiveSvg[] = "plasma/decoration/titlebar-inactive.svg";
    inline constexpr const char kCloseSvg[] = "plasma/decoration/close.svg";
    inline constexpr const char kMaximizeSvg[] = "plasma/decoration/maximize.svg";
    inline constexpr const char kMinimizeSvg[] = "plasma/decoration/minimize.svg";
    [[nodiscard]] inline Metrics defaults() noexcept {
        Metrics m{};
        m.title_bar_height = ::pbsd::kde::plasma::aero::default_blur().title_bar_height;
        m.corner_radius = ::pbsd::kde::plasma::aero::default_blur().corner_radius;
        return m;
    }

} // namespace pbsd::theme::plasma::aero::window_chrome
