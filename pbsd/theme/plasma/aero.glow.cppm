export module pbsd.theme.plasma.aero.glow;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 6 — PBSD Aero window glow constants.
export namespace pbsd::theme::plasma::aero::glow {

enum class State : unsigned char { Active, Inactive };

inline constexpr int kBlurRadius{20};
inline constexpr float kSaturation{1.20f};
inline constexpr const char kActiveSvg[] = "plasma/decoration/glow-active.svg";
inline constexpr const char kInactiveSvg[] = "plasma/decoration/glow-inactive.svg";

[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams params(State st) noexcept {
    auto p = ::pbsd::kde::plasma::aero::default_blur();
    p.blur_radius = kBlurRadius;
    p.saturation = kSaturation;
    if (st == State::Inactive) {
        p.panel_opacity = 0.70f;
    }
    return p;
}

} // namespace pbsd::theme::plasma::aero::glow
