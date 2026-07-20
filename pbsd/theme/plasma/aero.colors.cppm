export module pbsd.theme.plasma.aero.colors;

import pbsd.core;
import pbsd.kde.plasma.colors;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 6 — PBSD Aero theme constants (aero.colors.cppm).
export namespace pbsd::theme::plasma::aero::colors {

    using AccentRole = ::pbsd::kde::plasma::colors::AccentRole;
    using Rgb = ::pbsd::kde::plasma::colors::Rgb;
    inline constexpr Rgb kGlassTint{0x1e, 0x38, 0x66};
    inline constexpr float kGlassAlpha{0.65f};
    inline constexpr const char kSchemeName[] = "PBSDAero";
    [[nodiscard]] inline Rgb accent(AccentRole role) noexcept {
        return ::pbsd::kde::plasma::colors::accent(role);
    }
    [[nodiscard]] inline ::pbsd::kde::plasma::aero::GlassTint glass_tint() noexcept {
        return ::pbsd::kde::plasma::aero::default_blur().tint;
    }

} // namespace pbsd::theme::plasma::aero::colors
