module;
#include <cstdint>

export module pbsd.kernel.sys_getrandom;

import pbsd.core;

/// Freestanding port of `kern/sys_getrandom.c` GRND flag validation.
export namespace pbsd::kernel::sys_getrandom {

inline constexpr unsigned kGrndNonblock  = 0x0001;
inline constexpr unsigned kGrndRandom    = 0x0002;
inline constexpr unsigned kGrndInsecure  = 0x0004;
inline constexpr unsigned kGrndValidMask =
    kGrndNonblock | kGrndRandom | kGrndInsecure;

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & ~kGrndValidMask) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_buflen(unsigned buflen) noexcept {
    if (buflen == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_request(unsigned buflen,
                                             unsigned flags) noexcept {
    if (validate_buflen(buflen) != Status::Ok) {
        return Status::Invalid;
    }
    return validate_flags(flags);
}

} // namespace pbsd::kernel::sys_getrandom
