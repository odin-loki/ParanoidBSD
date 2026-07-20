module;

export module pbsd.kernel.kern_kthread;

export import pbsd.core;

/// Freestanding port of `kern/kern_kthread.c` — kthread helpers.
export namespace pbsd::kernel::kern_kthread {

inline constexpr unsigned kKtRunning = 0x0001;
inline constexpr unsigned kKtStopped = 0x0002;

[[nodiscard]] inline bool is_running(unsigned flags) noexcept {
    return (flags & kKtRunning) != 0;
}

} // namespace pbsd::kernel::kern_kthread
