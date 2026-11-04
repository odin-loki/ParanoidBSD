export module pbsd.kde.plasma.tastenbrett.application.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.tastenbrett.application;

/// Burst 16 wave 2 — Plasma tastenbrett.application ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/application.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::tastenbrett::application::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct LayoutStyle {
    const char* section_key{"application"};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline LayoutStyle default_style() noexcept { return LayoutStyle{}; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::tastenbrett::application::upstream_path();
}

} // namespace pbsd::kde::plasma::tastenbrett::application::glue
