module;

export module pbsd.kernel.kern_proc;

export import pbsd.core;

/// Freestanding port of `kern/kern_proc.c` — proc sysctl flags.
export namespace pbsd::kernel::kern_proc {

inline constexpr int kProcAll = 0;
inline constexpr int kProcByPid = 1;
inline constexpr int kProcByPgrp = 2;
inline constexpr int kProcBySession = 3;
inline constexpr int kProcByTgid = 4;

[[nodiscard]] inline Status validate_what(int what) noexcept {
    if (what < kProcAll || what > kProcByTgid) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_proc
