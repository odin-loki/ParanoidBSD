export module pbsd.theme.plasma.aero.immersive;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 wave 2 — Aero immersive visual tokens.
export namespace pbsd::theme::plasma::aero::immersive {

struct ImmersiveTokens {
    float chrome_opacity{0.0f};
    unsigned hide_delay_ms{3000};
    bool auto_hide_panel{true};
};

[[nodiscard]] inline ImmersiveTokens default_tokens() noexcept {
    return ImmersiveTokens{};
}

} // namespace pbsd::theme::plasma::aero::immersive
