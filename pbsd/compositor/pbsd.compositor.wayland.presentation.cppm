module;
#include <cstdint>

export module pbsd.compositor.wayland.presentation;

import pbsd.core;

/// Burst 11 — wp_presentation frame timing (wayland-protocols stable).
export namespace pbsd::compositor::wayland::presentation {

inline constexpr const char kInterface[] = "wp_presentation";
inline constexpr int kVersion = 2;

enum class FeedbackEvent : unsigned int {
    SyncOutput  = 0,
    Present     = 1,
    Refresh     = 2,
    ClockId     = 3,
    Done        = 4,
};

enum class Kind : unsigned int {
    Vsync         = 0,
    Desktop       = 1,
    Idle          = 2,
    Commit        = 3,
};

struct PresentEvent {
    std::uint32_t tv_sec_hi{};
    std::uint32_t tv_sec_lo{};
    std::uint32_t tv_nsec{};
    std::uint32_t refresh{};
    std::uint32_t seq_hi{};
    std::uint32_t seq_lo{};
    unsigned flags{};
};

[[nodiscard]] inline bool is_feedback_event(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(FeedbackEvent::Done);
}

[[nodiscard]] inline Status validate_refresh(unsigned refresh_ns) noexcept {
    if (refresh_ns == 0 || refresh_ns > 100'000'000) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline int frame_budget_us(unsigned refresh_ns) noexcept {
    if (refresh_ns == 0) {
        return 16667;
    }
    return static_cast<int>(refresh_ns / 1000);
}

} // namespace pbsd::compositor::wayland::presentation
