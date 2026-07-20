module;
#include <cstdint>

export module pbsd.kernel.rwlock;

export import pbsd.core;

/// Wave 5 — rwlock flags from sys/rwlock.h.
export namespace pbsd::kernel::rwlock {

inline constexpr unsigned kLockRead            = 0x01;
inline constexpr unsigned kLockReadWaiters     = 0x02;
inline constexpr unsigned kLockWriteWaiters    = 0x04;
inline constexpr unsigned kLockWriteSpinner    = 0x08;
inline constexpr unsigned kLockWriterRecursed  = 0x10;
inline constexpr unsigned kLockFlagMask =
    kLockRead | kLockReadWaiters | kLockWriteWaiters | kLockWriteSpinner
    | kLockWriterRecursed;
inline constexpr unsigned kLockWaiters = kLockReadWaiters | kLockWriteWaiters;

inline constexpr unsigned kReadersShift = 5;
inline constexpr unsigned kOneReader    = 1u << kReadersShift;
inline constexpr unsigned kUnlocked     = 0u << kReadersShift | kLockRead;
inline constexpr unsigned kDestroyed    = kLockReadWaiters | kLockWriteWaiters;

struct RwlockStub {
    std::uintptr_t lock_word{kUnlocked};
};

[[nodiscard]] constexpr bool is_read_lock(unsigned flags) noexcept {
    return (flags & kLockRead) != 0;
}

[[nodiscard]] constexpr unsigned readers(std::uintptr_t word) noexcept {
    return static_cast<unsigned>((word & ~kLockFlagMask) >> kReadersShift);
}

[[nodiscard]] constexpr bool has_waiters(unsigned flags) noexcept {
    return (flags & kLockWaiters) != 0;
}

[[nodiscard]] constexpr Status validate_rwlock_word(std::uintptr_t word) noexcept {
    if (word == kDestroyed) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::rwlock
