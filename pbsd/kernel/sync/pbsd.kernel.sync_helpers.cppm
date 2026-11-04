module;
#include <cstdint>

export module pbsd.kernel.sync_helpers;

import pbsd.core;
import pbsd.kernel.mutex;
import pbsd.kernel.turnstile;
import pbsd.kernel.subr_turnstile;
import pbsd.kernel.sleepq;

/// Freestanding port of `kern/subr_sleepqueue.c`.
export namespace pbsd::kernel::sync_helpers {

[[nodiscard]] inline Status lock_or_sleep(mutex::MutexClass& m,
                                                   turnstile::TurnstileStub& ts,
                                                   unsigned tid) noexcept {
    const auto st = mutex::try_lock(m, tid);
    if (st == Status::Ok) {
        return Status::Ok;
    }
    return subr_turnstile::queue_wait(ts, turnstile::kTsExclusiveQueue);
}

[[nodiscard]] inline Status unlock_and_wakeup(mutex::MutexClass& m,
                                              sleepq::Channel& ch,
                                              unsigned tid) noexcept {
    if (mutex::unlock(m, tid) != Status::Ok) {
        return Status::Denied;
    }
    return sleepq::wakeup(ch);
}

} // namespace pbsd::kernel::sync_helpers
