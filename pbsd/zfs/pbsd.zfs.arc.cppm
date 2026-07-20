module;
#include <cstdint>

export module pbsd.zfs.arc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/arc.h — ARC buf flags.
export namespace pbsd::zfs::arc {

inline constexpr unsigned long long kEvictAll = ~0ull;
inline constexpr unsigned long long kMinArcMax = 1ull << 17;

enum class Flag : unsigned int {
    Wait = 1u << 0,
    Nowait = 1u << 1,
    Prefetch = 1u << 2,
    Cached = 1u << 3,
    L2cache = 1u << 4,
    Uncached = 1u << 5,
    InHashTable = 1u << 7,
    IoInProgress = 1u << 8,
    IoError = 1u << 9,
    Indirect = 1u << 10,
    BufMetadata = 1u << 17,
    CompressedArc = 1u << 20,
    SharedData = 1u << 21,
    Encrypted = 1u << 22,
};

enum class Strategy : unsigned char {
    MetaOnly = 0,
    MetaBalanced = 1,
};

enum class BufContents : unsigned char {
    Data = 0,
    Metadata = 1,
};

enum class SpaceType : unsigned char {
    Data = 0,
    Meta = 1,
    Hdrs = 2,
    L2hdrs = 3,
    Dbuf = 4,
    Dnode = 5,
    Bonus = 6,
};

[[nodiscard]] inline bool is_encrypted(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(Flag::Encrypted)) != 0;
}

[[nodiscard]] inline bool is_metadata(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(Flag::BufMetadata)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::CompressedArc))
        && !is_encrypted(flags)) {
        return Status::Protocol;
    }
    if ((flags & static_cast<unsigned>(Flag::Wait))
        && (flags & static_cast<unsigned>(Flag::Nowait))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_evict_target(unsigned long long bytes,
                                                  unsigned long long arc_max) noexcept {
    if (bytes == 0 || (bytes != kEvictAll && bytes > arc_max)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::arc
