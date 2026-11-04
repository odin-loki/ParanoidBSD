module;

export module pbsd.kernel.kern_thread;

export import pbsd.core;

/// Freestanding port of `kern/kern_thread.c` — thread helpers.
export namespace pbsd::kernel::kern_thread {

inline constexpr int kTdRunning = 0x0001;
inline constexpr int kTdSleeping = 0x0002;
inline constexpr int kTdLocksleep = 0x0004;
inline constexpr int kTdIntr = 0x0008;

[[nodiscard]] inline bool is_sleeping(int flags) noexcept {
    return (flags & (kTdSleeping | kTdLocksleep)) != 0;
}

[[nodiscard]] inline Status validate_state(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_thread
