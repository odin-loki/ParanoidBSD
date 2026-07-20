module;

export module pbsd.kernel.kern_exec;

export import pbsd.core;

/// Freestanding port of `kern/kern_exec.c` — exec helpers.
export namespace pbsd::kernel::kern_exec {

inline constexpr int kTraceExec = 0x0001;
inline constexpr int kTraceFork = 0x0002;
inline constexpr int kTraceVfork = 0x0004;

[[nodiscard]] inline Status validate_trace_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool traces_exec(int flags) noexcept {
    return (flags & kTraceExec) != 0;
}

} // namespace pbsd::kernel::kern_exec
