module;
#include <cstdint>

export module pbsd.stand.mbr;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/part/gpt_mbr.c
export namespace pbsd::stand::mbr {

inline constexpr unsigned kSignature = 0xAA55;
inline constexpr unsigned kPartitionCount = 4;

struct Partition {
    unsigned char boot_indicator{};
    unsigned char type{};
    std::uint32_t lba_start{};
    std::uint32_t sector_count{};
};

[[nodiscard]] inline Status validate_signature(unsigned sig) noexcept {
    return sig == kSignature ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::mbr
