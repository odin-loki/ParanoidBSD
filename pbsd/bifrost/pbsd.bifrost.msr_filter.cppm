module;
#include <cstdint>

export module pbsd.bifrost.msr_filter;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/msr_bitmap.c
export namespace pbsd::bifrost::msr_filter {

inline constexpr unsigned kBitmapBytes = 4096;

[[nodiscard]] inline Status validate_offset(unsigned offset) noexcept {
    return offset < kBitmapBytes ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::msr_filter
