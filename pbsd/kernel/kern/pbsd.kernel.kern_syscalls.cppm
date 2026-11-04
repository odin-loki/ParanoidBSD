module;
#include <cstdint>

export module pbsd.kernel.kern_syscalls;

export import pbsd.core;

/// Freestanding port of `kern/kern_syscalls.c` — syscalls helpers.
export namespace pbsd::kernel::kern_syscalls {

inline constexpr unsigned char kSyfCapenabled = 0x01;
inline constexpr unsigned kSyThrStatic = 0x01;
inline constexpr unsigned kSyThrDraining = 0x02;
inline constexpr unsigned kSyThrAbsent = 0x04;

[[nodiscard]] inline bool cap_enabled(unsigned char flags) noexcept {
    return (flags & kSyfCapenabled) != 0;
}

[[nodiscard]] inline bool is_draining(unsigned thr) noexcept {
    return (thr & kSyThrDraining) != 0;
}

} // namespace pbsd::kernel::kern_syscalls
