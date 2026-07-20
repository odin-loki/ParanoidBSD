export module pbsd.theme.plasma.aero.action_center;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 wave 2 — Aero action center visual tokens.
export namespace pbsd::theme::plasma::aero::action_center {

struct ActionCenterTokens {
    unsigned panel_width_px{360};
    float backdrop_opacity{0.55f};
    const char* panel_svg{"plasma/panel/action-center.svg"};
};

[[nodiscard]] inline ActionCenterTokens default_tokens() noexcept {
    return ActionCenterTokens{};
}

} // namespace pbsd::theme::plasma::aero::action_center
