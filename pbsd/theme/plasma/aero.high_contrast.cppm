export module pbsd.theme.plasma.aero.high_contrast;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.theme.plasma.aero.focus;
import pbsd.theme.plasma.aero.borders;

/// Burst 12 — Aero high-contrast accessibility overrides.
export namespace pbsd::theme::plasma::aero::high_contrast {

struct Palette {
    const char* foreground{"#ffffff"};
    const char* background{"#000000"};
    const char* accent{"#ffff00"};
    float min_contrast{4.5f};
    float panel_opacity{0.95f};
    focus::FocusRing focus{focus::kHighContrast};
    borders::Border border{borders::kDialogBorder.width, 1.0f, 0, "#ffffff"};
};

inline constexpr Palette kDefault{};

[[nodiscard]] inline Palette for_forced_colors(bool enabled) noexcept {
    if (!enabled) {
        Palette p{};
        p.foreground = ::pbsd::kde::plasma::aero::kAccentPrimaryHex;
        p.background = "#1a2233";
        p.accent = ::pbsd::kde::plasma::aero::kAccentHighlightHex;
        p.min_contrast = 3.0f;
        p.panel_opacity = 0.85f;
        p.focus = focus::kDefault;
        return p;
    }
    return kDefault;
}

[[nodiscard]] inline float effective_panel_opacity(bool forced) noexcept {
    return forced ? kDefault.panel_opacity : 0.72f;
}

[[nodiscard]] inline Status validate_contrast(float ratio) noexcept {
    if (ratio < 1.0f || ratio > 21.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::high_contrast
