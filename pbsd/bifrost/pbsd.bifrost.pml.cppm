module;
#include <cstdint>

export module pbsd.bifrost.pml;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/pml.c
export namespace pbsd::bifrost::pml {

inline constexpr unsigned kEntryCount = 512;
[[nodiscard]] inline Status validate_index(unsigned idx) noexcept {
    return idx < kEntryCount ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::pml
