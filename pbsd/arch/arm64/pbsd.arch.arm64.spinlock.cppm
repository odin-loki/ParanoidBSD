module;
#include <cstdint>

export module pbsd.arch.arm64.spinlock;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/spinlock.c
export namespace pbsd::arch::arm64::spinlock {

enum class State : unsigned char {
    Unlocked = 0,
    Locked = 1,
};

[[nodiscard]] inline bool is_locked(State s) noexcept {
    return s == State::Locked;
}

} // namespace pbsd::arch::arm64::spinlock
