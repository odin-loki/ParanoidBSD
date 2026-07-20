module;
#include <cstdint>

export module pbsd.kernel.witness;

import pbsd.core;

/// Freestanding witness(4) stubs from sys/lock.h + kern/subr_witness.c.
export namespace pbsd::kernel::witness {

inline constexpr unsigned kLoWitness   = 0x00020000u;
inline constexpr unsigned kLoIsVnode   = 0x00800000u;

inline constexpr int kLaMaskAssert   = 0x000000ff;
inline constexpr int kLaUnlocked     = 0x00000000;
inline constexpr int kLaLocked         = 0x00000001;
inline constexpr int kLaSlocked        = 0x00000002;
inline constexpr int kLaXlocked        = 0x00000004;
inline constexpr int kLaRecursed       = 0x00000008;
inline constexpr int kLaNotrecursed    = 0x00000010;

enum class WarnClass : int {
    LockedAfterSleep = 0x01,
    SpinOnSleepable  = 0x02,
    OrderReversal    = 0x04,
    RecursiveLock    = 0x08,
};

struct LockObjectStub {
    const char* name{nullptr};
    unsigned    class_flags{};
    unsigned    lock_flags{};
    bool        owned{false};
    bool        recursed{false};
    int         assert_state{kLaUnlocked};
};

struct OrderEdge {
    const char* parent;
    const char* child;
};

inline constexpr OrderEdge kKnownOrders[] = {
    {"Giant", "sleepable"},
    {"mutex", "sx"},
};

[[nodiscard]] inline Status init(LockObjectStub& lo, const char* name,
                                 unsigned class_flags) noexcept {
    if (name == nullptr) {
        return Status::Invalid;
    }
    lo.name = name;
    lo.class_flags = class_flags;
    lo.owned = false;
    lo.recursed = false;
    lo.assert_state = kLaUnlocked;
    return Status::Ok;
}

[[nodiscard]] inline Status check_order(const LockObjectStub& held,
                                      const LockObjectStub& want) noexcept {
    if (!held.owned) {
        return Status::Ok;
    }
    if (held.name != nullptr && want.name != nullptr) {
        for (const auto& e : kKnownOrders) {
            if (held.name == e.child && want.name == e.parent) {
                return Status::Denied;
            }
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status lock(LockObjectStub& lo, int assert_want) noexcept {
    if ((assert_want & kLaXlocked) != 0 && lo.recursed) {
        return Status::Denied;
    }
    lo.owned = true;
    lo.assert_state = assert_want;
    return Status::Ok;
}

[[nodiscard]] inline Status unlock(LockObjectStub& lo) noexcept {
    if (!lo.owned) {
        return Status::Invalid;
    }
    lo.owned = false;
    lo.recursed = false;
    lo.assert_state = kLaUnlocked;
    return Status::Ok;
}

[[nodiscard]] inline bool assert_ok(const LockObjectStub& lo, int want) noexcept {
    return (lo.assert_state & kLaMaskAssert) == (want & kLaMaskAssert);
}

[[nodiscard]] inline unsigned order_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kKnownOrders) / sizeof(kKnownOrders[0]));
}

} // namespace pbsd::kernel::witness
