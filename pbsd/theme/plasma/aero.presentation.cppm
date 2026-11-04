export module pbsd.theme.plasma.aero.presentation;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.presentation;

/// Wave 3 pass 7 — Aero frame pacing tokens ↔ wp_presentation feedback.
export namespace pbsd::theme::plasma::aero::presentation {

inline constexpr int kTargetFps{60};
inline constexpr int kFrameBudgetUs{16667};
inline constexpr float kSnapAnimationMs{220.0f};

struct FramePacing {
    int target_fps{kTargetFps};
    int frame_budget_us{kFrameBudgetUs};
    float snap_ms{kSnapAnimationMs};
};

[[nodiscard]] inline FramePacing default_pacing() noexcept {
    FramePacing p{};
    p.frame_budget_us = static_cast<int>(::pbsd::kde::presentation::kRefreshNs / 1000);
    return p;
}

[[nodiscard]] inline int frame_budget_for_fps(int fps) noexcept {
    if (fps <= 0) {
        return kFrameBudgetUs;
    }
    return 1'000'000 / fps;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::presentation::upstream_path();
}

} // namespace pbsd::theme::plasma::aero::presentation
