export module pbsd.theme.plasma.aero.bifrost_tokens;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 7 — Bifrost hypervisor ↔ Aero glass visual token bridge.
/// Maps PBSD Bifrost VM identity colors into Aero theme glass/accent slots.
export namespace pbsd::theme::plasma::aero::bifrost_tokens {

/// Bifrost frost-blue accent (distinct from Aero primary but harmonized).
struct FrostAccent {
    float r{0.18f};
    float g{0.38f};
    float b{0.62f};
    float a{0.85f};
};

inline constexpr FrostAccent kHypervisorStrip{0.18f, 0.38f, 0.62f, 0.85f};
inline constexpr FrostAccent kVmConsole{0.10f, 0.16f, 0.24f, 0.92f};
inline constexpr FrostAccent kGuestOverlay{0.12f, 0.22f, 0.40f, 0.65f};

[[nodiscard]] inline ::pbsd::kde::plasma::aero::GlassTint to_glass_tint(FrostAccent f) noexcept {
    return ::pbsd::kde::plasma::aero::GlassTint{f.r, f.g, f.b, f.a};
}

[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams guest_panel_blur() noexcept {
    auto p = ::pbsd::kde::plasma::aero::default_blur();
    p.tint = to_glass_tint(kGuestOverlay);
    p.panel_opacity = 0.72f;
    return p;
}

[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams console_dialog_blur() noexcept {
    auto p = guest_panel_blur();
    p.tint = to_glass_tint(kVmConsole);
    p.dialog_opacity = 0.88f;
    return p;
}

} // namespace pbsd::theme::plasma::aero::bifrost_tokens
