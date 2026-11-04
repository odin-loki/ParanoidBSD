module;
#include <cstdint>

export module pbsd.kernel.epoch;

export import pbsd.core;

/// Wave 4/5 — epoch-based reclamation from sys/epoch.h, kern/subr_epoch.c.
export namespace pbsd::kernel::epoch {

inline constexpr unsigned kEpochPreemptMin = 1;
inline constexpr unsigned kEpochPreemptMax = 4;

enum class EpochState : unsigned char {
    Uninitialized = 0,
    Active = 1,
    Draining = 2,
};

struct EpochTracker {
    unsigned    epoch{};
    EpochState  state{EpochState::Uninitialized};
    unsigned    preempt_count{};
};

[[nodiscard]] constexpr Status validate_preempt(unsigned count) noexcept {
    if (count < kEpochPreemptMin || count > kEpochPreemptMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool is_active(EpochState s) noexcept {
    return s == EpochState::Active;
}

[[nodiscard]] constexpr bool can_drain(EpochState s) noexcept {
    return s == EpochState::Active || s == EpochState::Draining;
}

[[nodiscard]] constexpr unsigned advance_epoch(unsigned cur) noexcept {
    return cur + 1;
}

} // namespace pbsd::kernel::epoch
