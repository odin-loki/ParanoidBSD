export module pbsd.theme.plasma.aero.privacy_indicator;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Aero privacy indicator visual tokens.
export namespace pbsd::theme::plasma::aero::privacy_indicator {

struct PrivacyTokens {
    unsigned icon_size_px{16};
    float active_opacity{1.0f};
    float idle_opacity{0.45f};
};

[[nodiscard]] inline PrivacyTokens default_tokens() noexcept {
    return PrivacyTokens{};
}

} // namespace pbsd::theme::plasma::aero::privacy_indicator
