export module pbsd.theme.plasma.aero.contrast;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero blur/contrast boost tokens (blur-effect.json).
export namespace pbsd::theme::plasma::aero::contrast {

inline constexpr float kPanelContrast{1.05f};
inline constexpr float kDialogContrast{1.08f};
inline constexpr float kHighContrastBoost{1.25f};
inline constexpr float kMinContrast{0.5f};
inline constexpr float kMaxContrast{2.0f};

[[nodiscard]] inline Status validate_contrast(float c) noexcept {
    if (c < kMinContrast || c > kMaxContrast) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline float for_surface(
    ::pbsd::kde::plasma::aero::CompositingSurface s) noexcept {
    switch (s) {
    case ::pbsd::kde::plasma::aero::CompositingSurface::Dialog:
        return kDialogContrast;
    case ::pbsd::kde::plasma::aero::CompositingSurface::StartMenu:
        return kHighContrastBoost;
    default:
        return kPanelContrast;
    }
}

} // namespace pbsd::theme::plasma::aero::contrast
