export module pbsd.theme.plasma.aero.borders;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 12 — Aero chrome border width and corner tokens.
export namespace pbsd::theme::plasma::aero::borders {

struct Border {
    float width{1.0f};
    float opacity{0.55f};
    int corner_radius{8};
    const char* color_hex{::pbsd::kde::plasma::aero::kAccentPrimaryHex};
};

inline constexpr Border kPanelBorder{1.0f, 0.45f, 0, "#406090"};
inline constexpr Border kDialogBorder{1.0f, 0.55f, 8, "#5082c8"};
inline constexpr Border kWindowBorder{1.0f, 0.60f, 0, "#5082c8"};
inline constexpr Border kStartMenuBorder{1.0f, 0.50f, 8, "#5082c8"};

[[nodiscard]] inline Border for_surface(
    ::pbsd::kde::plasma::aero::CompositingSurface s) noexcept {
    switch (s) {
    case ::pbsd::kde::plasma::aero::CompositingSurface::Panel:
        return kPanelBorder;
    case ::pbsd::kde::plasma::aero::CompositingSurface::Dialog:
        return kDialogBorder;
    case ::pbsd::kde::plasma::aero::CompositingSurface::StartMenu:
        return kStartMenuBorder;
    case ::pbsd::kde::plasma::aero::CompositingSurface::Decoration:
        return kWindowBorder;
    default:
        return kPanelBorder;
    }
}

[[nodiscard]] inline Status validate_radius(int r) noexcept {
    if (r < 0 || r > 24) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::borders
