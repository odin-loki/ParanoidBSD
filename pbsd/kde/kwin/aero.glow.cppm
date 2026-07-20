export module pbsd.kde.kwin.aero.glow;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.compositing;

/// Wave 3 pass 6 — Aero window glow blur constants (glowActive/glowInactive assets).
/// Upstream: pbsd/theme/plasma/blur-effect.json
export namespace pbsd::kde::kwin::aero::glow {

    inline constexpr const char kGlowActiveSvg[] = "plasma/decoration/glow-active.svg";
    inline constexpr const char kGlowInactiveSvg[] = "plasma/decoration/glow-inactive.svg";
    inline constexpr int kGlowBlurRadius{20};
    inline constexpr float kGlowSaturation{1.20f};
    inline constexpr float kGlowContrast{1.08f};
    inline constexpr float kGlowNoise{0.03f};
    enum class State : unsigned char { Active, Inactive };
    [[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams params(State state) noexcept {
        auto p = ::pbsd::kde::kwin::compositing::glow_blur();
        if (state == State::Inactive) {
            p.saturation = 1.05f;
            p.panel_opacity = 0.60f;
        }
        return p;
    }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/plasma/blur-effect.json";
}

} // namespace pbsd::kde::kwin::aero::glow
