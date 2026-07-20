module;

export module pbsd.kernel.kern_lock;

export import pbsd.core;

/// Freestanding port of `kern/kern_lock.c` — lock helpers.
export namespace pbsd::kernel::kern_lock {

inline constexpr int kLockShared = 0x0001;
inline constexpr int kLockExclusive = 0x0002;
inline constexpr int kLockUpgrade = 0x0004;
inline constexpr int kLockDowngrade = 0x0008;

[[nodiscard]] inline Status validate_mode(int mode) noexcept {
    const int acc = mode & (kLockShared | kLockExclusive);
    if (acc != kLockShared && acc != kLockExclusive) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_lock
