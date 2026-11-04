module;
#include <cstdint>

export module pbsd.zfs.zil;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zil.h — ZIL block types.
export namespace pbsd::zfs::zil {

inline constexpr unsigned kHeaderSize = 128;
inline constexpr unsigned kMaxBlockSize = 131072;

enum class BlockType : unsigned char {
    Header = 1,
    Write  = 2,
    Free   = 3,
    Intent = 4,
};

enum class State : unsigned char {
    Initial = 0,
    Active  = 1,
    Closed  = 2,
};

[[nodiscard]] inline Status validate_block_type(BlockType t) noexcept {
    switch (t) {
    case BlockType::Header:
    case BlockType::Write:
    case BlockType::Free:
    case BlockType::Intent:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::zfs::zil
