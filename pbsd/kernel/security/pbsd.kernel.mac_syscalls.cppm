module;
#include <cstdint>

export module pbsd.kernel.mac_syscalls;

import pbsd.core;

/// Freestanding port of `security/mac/mac_syscalls.c` — MAC syscall helpers.
export namespace pbsd::kernel::mac_syscalls {

inline constexpr unsigned kMacGetPid = 0;
inline constexpr unsigned kMacSetPid = 1;
inline constexpr unsigned kMacGetFile = 2;
inline constexpr unsigned kMacSetFile = 3;

[[nodiscard]] inline Status validate_op(unsigned op) noexcept {
    switch (op) {
    case kMacGetPid:
    case kMacSetPid:
    case kMacGetFile:
    case kMacSetFile:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::kernel::mac_syscalls
