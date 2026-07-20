module;
#include <cstdint>

export module pbsd.bifrost.memslot;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmcs.c
export namespace pbsd::bifrost::memslot {

inline constexpr unsigned kMaxSlots = 32;
inline constexpr unsigned kGuestPhysMask = 0x000FFFFFFFFFF000ull;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kMaxSlots ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::memslot
