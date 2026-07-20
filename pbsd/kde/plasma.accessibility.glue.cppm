export module pbsd.kde.plasma.accessibility.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.accessibility;

/// Burst 16 — Plasma accessibility ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-desktop/kcms/accessibility/accessibility.cpp
export namespace pbsd::kde::plasma::accessibility::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct AccessibilityStyle {
    const char* kcm_id{accessibility::kKcmId};
    float min_contrast{accessibility::kMinContrast};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline AccessibilityStyle default_style() noexcept {
    return AccessibilityStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::accessibility::upstream_path();
}

} // namespace pbsd::kde::plasma::accessibility::glue
