export module pbsd.theme.plasma.aero.task_preview;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 14 — Aero task preview visual tokens.
export namespace pbsd::theme::plasma::aero::task_preview {

struct PreviewTokens {
    unsigned thumbnail_width_px{240};
    unsigned animation_ms{180};
    float shadow_opacity{0.35f};
};

[[nodiscard]] inline PreviewTokens default_tokens() noexcept {
    return PreviewTokens{};
}

} // namespace pbsd::theme::plasma::aero::task_preview
