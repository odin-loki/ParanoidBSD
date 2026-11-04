export module pbsd.theme.plasma.aero.task_switcher;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 wave 2 — Aero task switcher visual tokens.
export namespace pbsd::theme::plasma::aero::task_switcher {

struct TaskSwitcherTokens {
    unsigned thumbnail_width_px{240};
    unsigned animation_ms{180};
    float dim_opacity{0.40f};
};

[[nodiscard]] inline TaskSwitcherTokens default_tokens() noexcept {
    return TaskSwitcherTokens{};
}

} // namespace pbsd::theme::plasma::aero::task_switcher
