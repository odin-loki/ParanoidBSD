module;
#include <cstdint>

export module pbsd.stand.cd9660;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/cd9660.c, sys/fs/cd9660/iso.h — loader ISO9660.
export namespace pbsd::stand::cd9660 {

inline constexpr unsigned kBlockShift = 11;
inline constexpr unsigned kBlockSize = 1u << kBlockShift;
inline constexpr unsigned char kVdPrimary = 1;
inline constexpr unsigned char kVdEnd = 255;
inline constexpr unsigned kRootIno = 1;

inline constexpr char kStandardId[] = "CD001";
inline constexpr char kSuspPresent[] = "SP";
inline constexpr char kSuspStop[] = "ST";
inline constexpr char kRripName[] = "NM";

enum class VdType : unsigned char {
    Primary = kVdPrimary,
    End = kVdEnd,
};

struct VolumeHint {
    VdType type{VdType::Primary};
    std::uint32_t logical_block_size{kBlockSize};
    std::uint32_t path_table_loc{0};
    bool rrip{false};
};

[[nodiscard]] inline Status validate_block_size(std::uint32_t bsize) noexcept {
    if (bsize != kBlockSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_volume(const VolumeHint& vol) noexcept {
    if (validate_block_size(vol.logical_block_size) != Status::Ok) {
        return Status::Invalid;
    }
    if (vol.type != VdType::Primary && vol.type != VdType::End) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t lba_to_byte_offset(std::uint64_t lba) noexcept {
    return lba << kBlockShift;
}

[[nodiscard]] inline std::uint64_t byte_offset_to_lba(std::uint64_t off) noexcept {
    if ((off & (kBlockSize - 1u)) != 0) {
        return 0;
    }
    return off >> kBlockShift;
}

} // namespace pbsd::stand::cd9660
