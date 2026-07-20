export module pbsd.theme.plasma.aero.magnifier;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 wave 2 — Aero magnifier visual tokens.
export namespace pbsd::theme::plasma::aero::magnifier {

struct MagnifierTokens {
    unsigned zoom_percent{200};
    unsigned lens_size_px{128};
    float border_opacity{0.70f};
};

[[nodiscard]] inline MagnifierTokens default_tokens() noexcept {
    return MagnifierTokens{};
}

} // namespace pbsd::theme::plasma::aero::magnifier
