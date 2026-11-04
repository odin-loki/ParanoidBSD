export module pbsd.theme.plasma.aero.media_controls;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 14 — Aero media controls visual tokens.
export namespace pbsd::theme::plasma::aero::media_controls {

struct MediaTokens {
    unsigned button_size_px{32};
    float inactive_opacity{0.55f};
    const char* play_svg{"plasma/panel/media-play.svg"};
};

[[nodiscard]] inline MediaTokens default_tokens() noexcept {
    return MediaTokens{};
}

} // namespace pbsd::theme::plasma::aero::media_controls
