export module pbsd.kde.plasma.touchscreen.moduledata.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.touchscreen.moduledata;

/// Burst 16 wave 2 — Plasma touchscreen.moduledata ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::touchscreen::moduledata::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct TouchscreenModuleStyle {
    const char* group{moduledata::kTouchscreenGroup};
    unsigned max_devices{moduledata::kMaxDevices};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline TouchscreenModuleStyle default_style() noexcept {
    return TouchscreenModuleStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::touchscreen::moduledata::upstream_path();
}

} // namespace pbsd::kde::plasma::touchscreen::moduledata::glue
