export module pbsd.kde.plasma.mediacontroller.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.mediacontroller;

/// Burst 13 — Media controller applet ↔ Aero widget chrome glue.
export namespace pbsd::kde::plasma::mediacontroller::glue {

struct ControllerStyle {
    unsigned poll_ms{mediacontroller::kDefaultPollMs};
    int icon_size{22};
    float popup_opacity{0.80f};
    const char* progress_svg{aero::kProgressSvg};
};

[[nodiscard]] inline ControllerStyle default_style() noexcept {
    return ControllerStyle{};
}

[[nodiscard]] inline float opacity_for_status(mediacontroller::PlaybackStatus st) noexcept {
    switch (st) {
    case mediacontroller::PlaybackStatus::Playing:
        return 0.88f;
    case mediacontroller::PlaybackStatus::Paused:
        return 0.72f;
    default:
        return 0.60f;
    }
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return mediacontroller::upstream_path();
}

} // namespace pbsd::kde::plasma::mediacontroller::glue
