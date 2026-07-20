module;
#include <cstdint>

export module pbsd.compositor.frame.pacing;

import pbsd.core;
import pbsd.compositor.wayland.presentation;

/// Burst 12 — Frame pacing budget from wp_presentation refresh hints.
export namespace pbsd::compositor::frame::pacing {

struct FrameBudget {
    int budget_us{16667};
    int refresh_ns{16'666'666};
    unsigned max_late_frames{2};
    wayland::presentation::Kind kind{wayland::presentation::Kind::Vsync};
};

[[nodiscard]] inline FrameBudget from_refresh(unsigned refresh_ns) noexcept {
    FrameBudget b{};
    if (wayland::presentation::validate_refresh(refresh_ns) != Status::Ok) {
        b.refresh_ns = 16'666'666;
        b.budget_us = wayland::presentation::frame_budget_us(b.refresh_ns);
        return b;
    }
    b.refresh_ns = refresh_ns;
    b.budget_us = wayland::presentation::frame_budget_us(refresh_ns);
    return b;
}

[[nodiscard]] inline FrameBudget default_60hz() noexcept {
    return from_refresh(16'666'666);
}

[[nodiscard]] inline bool is_late(std::uint32_t frame_time_us, int budget_us) noexcept {
    return static_cast<int>(frame_time_us) > budget_us;
}

[[nodiscard]] inline unsigned late_frame_count(
    std::uint32_t frame_time_us, int budget_us, unsigned prior_late) noexcept {
    if (!is_late(frame_time_us, budget_us)) {
        return 0;
    }
    return prior_late + 1;
}

[[nodiscard]] inline Status validate_budget(int budget_us) noexcept {
    if (budget_us < 1000 || budget_us > 100'000) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::frame::pacing
