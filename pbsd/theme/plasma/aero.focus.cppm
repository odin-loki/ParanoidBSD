export module pbsd.theme.plasma.aero.focus;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 12 — Aero keyboard focus ring tokens.
export namespace pbsd::theme::plasma::aero::focus {

struct FocusRing {
    float width{2.0f};
    float opacity{0.85f};
    const char* color_hex{::pbsd::kde::plasma::aero::kAccentHighlightHex};
    float dash_on{4.0f};
    float dash_off{2.0f};
};

inline constexpr FocusRing kDefault{};
inline constexpr FocusRing kHighContrast{3.0f, 1.0f, "#ffffff", 0.0f, 0.0f};
inline constexpr FocusRing kPanelItem{1.5f, 0.70f, ::pbsd::kde::plasma::aero::kAccentPrimaryHex, 0.0f, 0.0f};

[[nodiscard]] inline FocusRing for_surface(
    ::pbsd::kde::plasma::aero::CompositingSurface s) noexcept {
    switch (s) {
    case ::pbsd::kde::plasma::aero::CompositingSurface::Panel:
        return kPanelItem;
    case ::pbsd::kde::plasma::aero::CompositingSurface::Dialog:
    case ::pbsd::kde::plasma::aero::CompositingSurface::StartMenu:
        return kDefault;
    default:
        return kPanelItem;
    }
}

[[nodiscard]] inline Status validate_width(float w) noexcept {
    if (w < 0.0f || w > 8.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::focus
