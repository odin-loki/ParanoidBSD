module;
#include <cstdint>

export module pbsd.kernel.mac_framework;

import pbsd.core;

/// Freestanding port of `security/mac/mac_framework.c` — MAC framework constants.
export namespace pbsd::kernel::mac_framework {

inline constexpr unsigned kMaxPolicies = 32;
inline constexpr unsigned kSlotInvalid = 0xFFFFFFFFu;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kMaxPolicies ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status register_policy(unsigned slot, bool occupied) noexcept {
    if (validate_slot(slot) != Status::Ok) {
        return Status::Invalid;
    }
    return occupied ? Status::Busy : Status::Ok;
}

} // namespace pbsd::kernel::mac_framework
