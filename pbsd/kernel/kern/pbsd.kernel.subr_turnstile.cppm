module;
#include <cstdint>

export module pbsd.kernel.subr_turnstile;

import pbsd.core;
import pbsd.kernel.turnstile;

/// Freestanding port of `kern/subr_turnstile.c` helpers.
export namespace pbsd::kernel::subr_turnstile {

[[nodiscard]] inline Status queue_wait(turnstile::TurnstileStub& ts, int queue) noexcept {
    if (queue < turnstile::kTsExclusiveQueue || queue >= turnstile::kTsQueues) {
        return Status::Invalid;
    }
    if (queue == turnstile::kTsExclusiveQueue) {
        ++ts.blocked_exclusive;
    } else {
        ++ts.blocked_shared;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status signal_one(turnstile::TurnstileStub& ts, int queue) noexcept {
    if (turnstile::queue_empty(ts, queue)) {
        return Status::NotFound;
    }
    if (queue == turnstile::kTsExclusiveQueue) {
        --ts.blocked_exclusive;
    } else {
        --ts.blocked_shared;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_turnstile
