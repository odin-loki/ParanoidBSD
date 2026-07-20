export module pbsd.theme.plasma.aero.dwm_blur;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 5 — PBSD Aero theme constants (aero.dwm_blur.cppm).
export namespace pbsd::theme::plasma::aero::dwm_blur {

    /// DWM-style blur constants (Windows 7 Aero reference).
    inline constexpr int kBlurRadius{24};
    inline constexpr float kNoiseStrength{0.04f};
    inline constexpr float kSaturationBoost{1.15f};
    inline constexpr float kContrastBoost{1.05f};
    inline constexpr float kPanelOpacity{0.72f};
    inline constexpr float kDialogOpacity{0.78f};
    inline constexpr float kStartMenuOpacity{0.78f};
    inline constexpr bool kBlurBehindEnabled{true};
    inline constexpr bool kExtendFrameIntoClientArea{true};
    enum class Region : unsigned char { Panel, Dialog, StartMenu, TitleBar, Thumbnail };
    [[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams defaults() noexcept {
        return ::pbsd::kde::plasma::aero::default_blur();
    }

} // namespace pbsd::theme::plasma::aero::dwm_blur
