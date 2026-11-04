module;
#include <cstdint>

export module pbsd.kernel.subr_sleepqueue;

import pbsd.core;
import pbsd.kernel.sleepq;

/// Freestanding port of `kern/subr_sleepqueue.c` helpers.
export namespace pbsd::kernel::subr_sleepqueue {

[[nodiscard]] inline Status sleep_on(sleepq::Channel& ch, const void* wchan) noexcept {
    return sleepq::sleep(ch, wchan, sleepq::kTypeSleep);
}

[[nodiscard]] inline Status timed_sleep(sleepq::Channel& ch, const void* wchan, unsigned waiters) noexcept {
    if (wchan == nullptr) {
        return Status::Invalid;
    }
    ch.waiters = waiters;
    ch.wchan = wchan;
    ch.type = sleepq::kTypeSleep;
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_sleepqueue
