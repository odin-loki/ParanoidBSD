export module pbsd.kde.kwin.decorations;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — KDecoration3 bridge constants (from decorations_logging + theme metadata).
/// Upstream: kde/kwin/src/decorations/decorations_logging.cpp
export namespace pbsd::kde::kwin::decorations {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_DECORATIONS",
    "kwin_decorations",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/decorations/decorations_logging.cpp",
};

inline constexpr const char kPluginId[] = "pbsd_aero";
inline constexpr const char kMetadataPath[] = "plasma/decoration/metadata.json";
inline constexpr const char kTitleBarActiveSvg[] = "plasma/decoration/titlebar-active.svg";
inline constexpr const char kTitleBarInactiveSvg[] = "plasma/decoration/titlebar-inactive.svg";
inline constexpr const char kCloseSvg[] = "plasma/decoration/close.svg";
inline constexpr const char kMaximizeSvg[] = "plasma/decoration/maximize.svg";
inline constexpr const char kMinimizeSvg[] = "plasma/decoration/minimize.svg";
inline constexpr const char kShadeSvg[] = "plasma/decoration/shade.svg";
inline constexpr const char kRestoreSvg[] = "plasma/decoration/restore.svg";
inline constexpr const char kBorderLeftSvg[] = "plasma/decoration/border-left.svg";
inline constexpr const char kBlurEffectId[] = "pbsd_aero_blur";
inline constexpr const char kStartMenuFrameSvg[] = "plasma/panel/start-menu-frame.svg";
inline constexpr const char kPanelGlassSvg[] = "plasma/panel/panel-glass.svg";
inline constexpr const char kGlowActiveSvg[] = "plasma/decoration/glow-active.svg";
inline constexpr const char kGlowInactiveSvg[] = "plasma/decoration/glow-inactive.svg";
inline constexpr const char kBorderTopSvg[] = "plasma/decoration/border-top.svg";
inline constexpr const char kBorderRightSvg[] = "plasma/decoration/border-right.svg";
inline constexpr const char kBorderBottomSvg[] = "plasma/decoration/border-bottom.svg";

struct DecorationMetrics {
    int title_bar_height{36};
    int button_size{28};
    int border_width{1};
    int corner_radius{8};
    bool blur_enabled{true};
    float glass_opacity{0.78f};
    float panel_opacity{0.72f};
};

[[nodiscard]] inline DecorationMetrics default_metrics() noexcept {
    DecorationMetrics m{};
    m.title_bar_height = plasma::aero::default_blur().title_bar_height;
    m.corner_radius = plasma::aero::default_blur().corner_radius;
    return m;
}

[[nodiscard]] inline bool plugin_id_matches(const char* id) noexcept {
    if (id == nullptr) {
        return false;
    }
    return id[0] == 'p' && id[1] == 'b' && id[2] == 's' && id[3] == 'd'
        && id[4] == '_' && id[5] == 'a' && id[6] == 'e' && id[7] == 'r'
        && id[8] == 'o' && id[9] == '\0';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/decorations/decorations_logging.cpp";
}

} // namespace pbsd::kde::kwin::decorations
