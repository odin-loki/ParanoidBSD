export module pbsd.theme.plasma.aero.frame_assets;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero panel glass + start-menu frame assets (blur-effect.json).
export namespace pbsd::theme::plasma::aero::frame_assets {

inline constexpr const char kPanelGlassSvg[] = "plasma/panel/panel-glass.svg";
inline constexpr const char kStartMenuFrameSvg[] = "plasma/panel/start-menu-frame.svg";
inline constexpr float kStartMenuOpacity{0.78f};
inline constexpr float kPanelOpacity{0.72f};

[[nodiscard]] inline const char* panel_glass_path() noexcept {
    return ::pbsd::kde::plasma::aero::kPanelGlassSvg;
}

[[nodiscard]] inline const char* start_menu_frame_path() noexcept {
    return ::pbsd::kde::plasma::aero::kStartMenuFrameSvg;
}

[[nodiscard]] inline Status validate_opacity(float o) noexcept {
    if (o < 0.0f || o > 1.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::frame_assets
