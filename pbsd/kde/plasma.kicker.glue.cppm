export module pbsd.kde.plasma.kicker.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.kicker;

/// Burst 13 — Kicker start button ↔ Aero panel asset glue.
export namespace pbsd::kde::plasma::kicker::glue {

struct KickerStyle {
    const char* start_button_svg{kicker::kStartButtonSvg};
    const char* applications_svg{kicker::kApplicationsSvg};
    unsigned favorite_slots{kicker::kFavoriteSlots};
    float glass_opacity{0.72f};
};

[[nodiscard]] inline KickerStyle default_style() noexcept {
    KickerStyle s{};
    s.start_button_svg = aero::kStartButtonSvg;
    s.applications_svg = aero::kApplicationsSvg;
    return s;
}

[[nodiscard]] inline Status validate_favorite_slots(unsigned slots) noexcept {
    return slots <= kicker::kFavoriteSlots ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return kicker::upstream_path();
}

} // namespace pbsd::kde::plasma::kicker::glue
