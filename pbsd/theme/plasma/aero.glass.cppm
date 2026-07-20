export module pbsd.theme.plasma.aero.glass;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 5 — PBSD Aero theme constants (aero.glass.cppm).
export namespace pbsd::theme::plasma::aero::glass {

    /// Aero glass material tints (PBSD brand blue, no purple).
    struct Material {
        float r{0.12f};
        float g{0.22f};
        float b{0.40f};
        float a{0.65f};
    };
    inline constexpr Material kPanelGlass{0.10f, 0.20f, 0.38f, 0.72f};
    inline constexpr Material kDialogGlass{0.12f, 0.22f, 0.40f, 0.78f};
    inline constexpr Material kStartMenuGlass{0.11f, 0.21f, 0.39f, 0.78f};
    inline constexpr Material kTitleBarActive{0.14f, 0.26f, 0.46f, 0.85f};
    inline constexpr Material kTitleBarInactive{0.10f, 0.18f, 0.32f, 0.70f};
    [[nodiscard]] inline Material panel() noexcept { return kPanelGlass; }
    [[nodiscard]] inline Material from_blur_params(const ::pbsd::kde::plasma::aero::BlurParams& p) noexcept {
        return Material{p.tint.r, p.tint.g, p.tint.b, p.panel_opacity};
    }

} // namespace pbsd::theme::plasma::aero::glass
