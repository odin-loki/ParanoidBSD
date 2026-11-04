export module pbsd.kde.plasma.brightness.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.brightness;

/// Burst 16 — Plasma brightness ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-desktop/kcms/brightness/brightness.cpp
export namespace pbsd::kde::plasma::brightness::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct BrightnessStyle {
    const char* kcm_id{brightness::kKcmId};
    const char* settings_svg{aero::kSettingsSvg};
    int min_percent{brightness::kMinPercent};
    int max_percent{brightness::kMaxPercent};
};

[[nodiscard]] inline BrightnessStyle default_style() noexcept {
    return BrightnessStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::brightness::upstream_path();
}

} // namespace pbsd::kde::plasma::brightness::glue
