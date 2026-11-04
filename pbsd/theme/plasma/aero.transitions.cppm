export module pbsd.theme.plasma.aero.transitions;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 11 — Aero animation timing curves for panel/dialog transitions.
export namespace pbsd::theme::plasma::aero::transitions {

enum class Easing : unsigned char {
    Linear,
    EaseOut,
    EaseInOut,
    Snap,
};

struct Transition {
    float duration_ms{220.0f};
    Easing easing{Easing::EaseOut};
};

inline constexpr Transition kPanelShow{180.0f, Easing::EaseOut};
inline constexpr Transition kPanelHide{140.0f, Easing::EaseInOut};
inline constexpr Transition kStartMenuOpen{220.0f, Easing::Snap};
inline constexpr Transition kDialogFade{160.0f, Easing::EaseOut};
inline constexpr Transition kGlowPulse{400.0f, Easing::EaseInOut};

[[nodiscard]] inline Transition for_surface(
    ::pbsd::kde::plasma::aero::CompositingSurface s) noexcept {
    switch (s) {
    case ::pbsd::kde::plasma::aero::CompositingSurface::StartMenu:
        return kStartMenuOpen;
    case ::pbsd::kde::plasma::aero::CompositingSurface::Dialog:
        return kDialogFade;
    case ::pbsd::kde::plasma::aero::CompositingSurface::Glow:
        return kGlowPulse;
    default:
        return kPanelShow;
    }
}

[[nodiscard]] inline float apply_easing(Easing e, float t) noexcept {
    if (t <= 0.0f) {
        return 0.0f;
    }
    if (t >= 1.0f) {
        return 1.0f;
    }
    switch (e) {
    case Easing::Linear:
        return t;
    case Easing::EaseOut:
        return 1.0f - (1.0f - t) * (1.0f - t);
    case Easing::EaseInOut:
        return t < 0.5f ? 2.0f * t * t : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    case Easing::Snap:
        return t > 0.5f ? 1.0f : 0.0f;
    }
    return t;
}

[[nodiscard]] inline Status validate_duration(float ms) noexcept {
    if (ms < 0.0f || ms > 5000.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::transitions
