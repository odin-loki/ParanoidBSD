module;
#include <cstdint>

export module pbsd.kernel.kern_malloc;

export import pbsd.core;

/// Freestanding port of `kern/kern_malloc.c` — malloc type flags.
export namespace pbsd::kernel::kern_malloc {

inline constexpr unsigned kWaitOk = 0x0001;
inline constexpr unsigned kNowait = 0x0002;
inline constexpr unsigned kZero = 0x0100;
inline constexpr unsigned kNoFree = 0x0200;

[[nodiscard]] inline bool can_wait(unsigned flags) noexcept {
    return (flags & kWaitOk) != 0 && (flags & kNowait) == 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & kWaitOk) && (flags & kNowait)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_malloc
