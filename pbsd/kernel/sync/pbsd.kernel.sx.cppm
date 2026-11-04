module;
#include <cstdint>

export module pbsd.kernel.sx;

export import pbsd.core;

/// Wave 5 — sx sleepable lock flags from sys/sx.h.
export namespace pbsd::kernel::sx {

inline constexpr unsigned kLockShared            = 0x01;
inline constexpr unsigned kLockSharedWaiters     = 0x02;
inline constexpr unsigned kLockExclusiveWaiters  = 0x04;
inline constexpr unsigned kLockWriteSpinner      = 0x08;
inline constexpr unsigned kLockRecursed          = 0x10;
inline constexpr unsigned kLockFlagMask =
    kLockShared | kLockSharedWaiters | kLockExclusiveWaiters | kLockWriteSpinner
    | kLockRecursed;
inline constexpr unsigned kLockWaiters = kLockSharedWaiters | kLockExclusiveWaiters;

inline constexpr unsigned kSharersShift = 5;

struct SxStub {
    std::uintptr_t lock_word{};
};

[[nodiscard]] constexpr bool is_shared(unsigned flags) noexcept {
    return (flags & kLockShared) != 0;
}

[[nodiscard]] constexpr bool has_waiters(unsigned flags) noexcept {
    return (flags & kLockWaiters) != 0;
}

[[nodiscard]] constexpr bool is_recursed(unsigned flags) noexcept {
    return (flags & kLockRecursed) != 0;
}

[[nodiscard]] constexpr unsigned sharers(std::uintptr_t word) noexcept {
    return static_cast<unsigned>((word & ~kLockFlagMask) >> kSharersShift);
}

[[nodiscard]] constexpr Status validate_sx_word(std::uintptr_t word) noexcept {
    if ((word & kLockFlagMask) == kLockFlagMask) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::sx
