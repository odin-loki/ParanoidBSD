export module pbsd.theme.plasma.aero.volume_mixer;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 15 — Aero volume mixer visual tokens.
export namespace pbsd::theme::plasma::aero::volume_mixer {

struct VolumeTokens {
    unsigned slider_width_px{120};
    float mute_opacity{0.50f};
    const char* volume_svg{"plasma/panel/volume.svg"};
};

[[nodiscard]] inline VolumeTokens default_tokens() noexcept {
    return VolumeTokens{};
}

} // namespace pbsd::theme::plasma::aero::volume_mixer
