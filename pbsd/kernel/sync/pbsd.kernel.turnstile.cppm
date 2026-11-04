module;
#include <cstdint>

export module pbsd.kernel.turnstile;

import pbsd.core;

/// Freestanding stubs of `sys/turnstile.h` and `kern/subr_turnstile.c`.
export namespace pbsd::kernel::turnstile {

inline constexpr int kTsExclusiveQueue = 0;
inline constexpr int kTsSharedQueue     = 1;
inline constexpr int kTsQueues            = 2;

inline constexpr unsigned kTcTableSize = 128;
inline constexpr unsigned kTcMask      = kTcTableSize - 1;
inline constexpr unsigned kTcShift     = 8;

[[nodiscard]] constexpr unsigned chain_hash(std::uintptr_t lock_addr) noexcept {
    return (lock_addr >> kTcShift) & kTcMask;
}

enum class TurnstileOp : std::uint8_t {
    ChainLock = 0,
    Wait      = 1,
    TryWait   = 2,
    Cancel    = 3,
    Lookup    = 4,
    Signal    = 5,
    Broadcast = 6,
    Unpend    = 7,
    Disown    = 8,
    Claim     = 9,
};

struct TurnstileStub {
    unsigned owner_tid{0};
    unsigned blocked_exclusive{0};
    unsigned blocked_shared{0};
    unsigned pending{0};
    bool     pending_wakeup{false};
    bool     owned{false};
    bool     chain_locked{false};
    std::uintptr_t lock_addr{0};
};

struct ChainStub {
    bool locked{false};
    unsigned depth{0};
};

[[nodiscard]] inline bool queue_empty(const TurnstileStub& ts, int queue) noexcept {
    if (queue == kTsExclusiveQueue) {
        return ts.blocked_exclusive == 0;
    }
    if (queue == kTsSharedQueue) {
        return ts.blocked_shared == 0;
    }
    return true;
}

[[nodiscard]] inline bool empty(const TurnstileStub& ts) noexcept {
    return queue_empty(ts, kTsExclusiveQueue) && queue_empty(ts, kTsSharedQueue);
}

[[nodiscard]] inline Status chain_lock(TurnstileStub& ts, unsigned tid) noexcept {
    if (ts.owned && ts.owner_tid != tid) {
        return Status::Denied;
    }
    ts.owned = true;
    ts.owner_tid = tid;
    ts.chain_locked = true;
    return Status::Ok;
}

[[nodiscard]] inline Status chain_unlock(TurnstileStub& ts) noexcept {
    if (!ts.chain_locked) {
        return Status::Invalid;
    }
    ts.chain_locked = false;
    return Status::Ok;
}

[[nodiscard]] inline Status wait(TurnstileStub& ts, int queue) noexcept {
    if (!ts.chain_locked) {
        return Status::Denied;
    }
    if (queue == kTsExclusiveQueue) {
        ++ts.blocked_exclusive;
    } else if (queue == kTsSharedQueue) {
        ++ts.blocked_shared;
    } else {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status try_wait(TurnstileStub& ts, int queue) noexcept {
    if (!ts.chain_locked) {
        return Status::Denied;
    }
    if (queue != kTsExclusiveQueue && queue != kTsSharedQueue) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool signal(TurnstileStub& ts, int queue) noexcept {
    if (queue == kTsExclusiveQueue && ts.blocked_exclusive > 0) {
        --ts.blocked_exclusive;
        ++ts.pending;
        ts.pending_wakeup = true;
        return empty(ts);
    }
    if (queue == kTsSharedQueue && ts.blocked_shared > 0) {
        --ts.blocked_shared;
        ++ts.pending;
        ts.pending_wakeup = true;
        return empty(ts);
    }
    return false;
}

[[nodiscard]] inline Status broadcast(TurnstileStub& ts, int queue) noexcept {
    if (queue == kTsExclusiveQueue) {
        ts.pending += ts.blocked_exclusive;
        ts.blocked_exclusive = 0;
    } else if (queue == kTsSharedQueue) {
        ts.pending += ts.blocked_shared;
        ts.blocked_shared = 0;
    } else {
        return Status::Invalid;
    }
    if (ts.pending > 0) {
        ts.pending_wakeup = true;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status unpend(TurnstileStub& ts) noexcept {
    ts.pending = 0;
    ts.pending_wakeup = false;
    ts.owned = false;
    ts.owner_tid = 0;
    ts.chain_locked = false;
    return Status::Ok;
}

[[nodiscard]] inline Status cancel(TurnstileStub& ts, int queue) noexcept {
    if (queue == kTsExclusiveQueue && ts.blocked_exclusive > 0) {
        --ts.blocked_exclusive;
    } else if (queue == kTsSharedQueue && ts.blocked_shared > 0) {
        --ts.blocked_shared;
    } else {
        return Status::Invalid;
    }
    return chain_unlock(ts);
}

[[nodiscard]] inline Status disown(TurnstileStub& ts) noexcept {
    ts.owned = false;
    ts.owner_tid = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status claim(TurnstileStub& ts, unsigned tid) noexcept {
    if (ts.owned && ts.owner_tid != tid) {
        return Status::Denied;
    }
    ts.owned = true;
    ts.owner_tid = tid;
    return Status::Ok;
}

[[nodiscard]] inline Status lookup(TurnstileStub& ts, std::uintptr_t lock_addr) noexcept {
    ts.lock_addr = lock_addr;
    return Status::Ok;
}

[[nodiscard]] inline Status chain_lock(ChainStub& chain, TurnstileStub& ts,
                                     unsigned tid, std::uintptr_t lock_addr) noexcept {
    if (chain.locked) {
        return Status::Busy;
    }
    chain.locked = true;
    ++chain.depth;
    (void)lookup(ts, lock_addr);
    return chain_lock(ts, tid);
}

struct TurnstileOpEntry {
    TurnstileOp op;
    const char* name;
};

inline constexpr TurnstileOpEntry kOpTable[] = {
    {TurnstileOp::ChainLock, "chain_lock"},
    {TurnstileOp::Wait,      "wait"},
    {TurnstileOp::TryWait,   "try_wait"},
    {TurnstileOp::Signal,    "signal"},
    {TurnstileOp::Broadcast, "broadcast"},
    {TurnstileOp::Unpend,    "unpend"},
    {TurnstileOp::Disown,    "disown"},
    {TurnstileOp::Claim,     "claim"},
};

[[nodiscard]] inline unsigned op_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kOpTable) / sizeof(kOpTable[0]));
}

} // namespace pbsd::kernel::turnstile
