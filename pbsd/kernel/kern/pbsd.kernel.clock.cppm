module;
#include <cstdint>

export module pbsd.kernel.clock;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_clock.c — hardclock / softclock scaffold.
export namespace pbsd::kernel::clock {

inline constexpr unsigned kHz = 100;

struct SoftClock {
    std::uint64_t ticks{};
    std::uint64_t hardticks{};
    bool running{false};
};

[[nodiscard]] inline Status start(SoftClock& c) noexcept {
    c.running = true;
    return Status::Ok;
}

[[nodiscard]] inline Status hardclock(SoftClock& c) noexcept {
    if (!c.running) {
        return Status::Invalid;
    }
    ++c.hardticks;
    ++c.ticks;
    return Status::Ok;
}

[[nodiscard]] inline Status softclock(SoftClock& c, unsigned n) noexcept {
    if (!c.running || n == 0) {
        return Status::Invalid;
    }
    c.ticks += n;
    return Status::Ok;
}

} // namespace pbsd::kernel::clock
