module;
#include <cstdint>

export module pbsd.kernel.posix4_mib;

import pbsd.core;

/// Freestanding port of `kern/posix4_mib.c` — POSIX.4 MIB constants.
export namespace pbsd::kernel::posix4_mib {

inline constexpr unsigned kAioMax = 0;
inline constexpr unsigned kMqueueMax = 1;
inline constexpr unsigned kSemMax = 2;

[[nodiscard]] inline Status validate_oid(unsigned oid) noexcept {
    switch (oid) {
    case kAioMax:
    case kMqueueMax:
    case kSemMax:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::kernel::posix4_mib
