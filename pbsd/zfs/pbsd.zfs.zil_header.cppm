module;
#include <cstdint>

export module pbsd.zfs.zil_header;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zil_impl.h — ZIL block header.
export namespace pbsd::zfs::zil_header {

inline constexpr std::uint64_t kMagic = 0x2badbc2bULL;

struct Block {
    std::uint64_t magic{kMagic};
    std::uint64_t seq{};
    std::uint64_t birth{};
};

[[nodiscard]] inline Status validate_magic(std::uint64_t magic) noexcept {
    return magic == kMagic ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_block(const Block& b) noexcept {
    if (validate_magic(b.magic) != Status::Ok) {
        return Status::Invalid;
    }
    if (b.seq == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::zil_header
