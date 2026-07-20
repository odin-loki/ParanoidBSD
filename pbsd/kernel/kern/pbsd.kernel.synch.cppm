module;

export module pbsd.kernel.synch;

export import pbsd.core;

/// Freestanding port of `kern/kern_synch.c` — sleep/wakeup flags.
export namespace pbsd::kernel::synch {

inline constexpr int kSleepPri = 0;
inline constexpr int kSleepIntr = 0x0100;
inline constexpr int kSleepCatch = 0x0200;
inline constexpr int kSleepFail = 0x0400;

[[nodiscard]] inline bool is_interruptible(int flags) noexcept {
    return (flags & kSleepIntr) != 0;
}

[[nodiscard]] inline Status validate_sleep_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::synch
