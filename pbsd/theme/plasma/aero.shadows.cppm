export module pbsd.theme.plasma.aero.shadows;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 11 — Aero drop-shadow tokens for panels, dialogs, and popups.
export namespace pbsd::theme::plasma::aero::shadows {

struct Shadow {
    float offset_x{0.0f};
    float offset_y{2.0f};
    float blur{12.0f};
    float spread{0.0f};
    float opacity{0.35f};
};

inline constexpr Shadow kPanelShadow{0.0f, 4.0f, 16.0f, 0.0f, 0.28f};
inline constexpr Shadow kDialogShadow{0.0f, 8.0f, 24.0f, 0.0f, 0.40f};
inline constexpr Shadow kStartMenuShadow{0.0f, 12.0f, 32.0f, 0.0f, 0.45f};
inline constexpr Shadow kTooltipShadow{0.0f, 2.0f, 8.0f, 0.0f, 0.25f};

[[nodiscard]] inline Shadow for_surface(
    ::pbsd::kde::plasma::aero::CompositingSurface s) noexcept {
    switch (s) {
    case ::pbsd::kde::plasma::aero::CompositingSurface::Panel:
        return kPanelShadow;
    case ::pbsd::kde::plasma::aero::CompositingSurface::Dialog:
        return kDialogShadow;
    case ::pbsd::kde::plasma::aero::CompositingSurface::StartMenu:
        return kStartMenuShadow;
    default:
        return kTooltipShadow;
    }
}

[[nodiscard]] inline Status validate_blur(float blur) noexcept {
    if (blur < 0.0f || blur > 64.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::shadows
