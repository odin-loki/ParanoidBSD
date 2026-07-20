module;
#include <cstdint>

export module pbsd.zfs.metaslab;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/metaslab.h — alloc flags.
export namespace pbsd::zfs::metaslab {

inline constexpr unsigned kFlagZil = 0x1;
inline constexpr unsigned kFlagGangHeader = 0x2;
inline constexpr unsigned kFlagGangChild = 0x4;
inline constexpr unsigned kFlagAsyncAlloc = 0x8;

enum class AllocFlag : unsigned {
    None = 0,
    Zil = kFlagZil,
    GangHeader = kFlagGangHeader,
    GangChild = kFlagGangChild,
    AsyncAlloc = kFlagAsyncAlloc,
};

[[nodiscard]] constexpr AllocFlag operator|(AllocFlag a, AllocFlag b) noexcept {
    return static_cast<AllocFlag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool has_flag(unsigned flags, AllocFlag f) noexcept {
    return (flags & static_cast<unsigned>(f)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    constexpr unsigned kMask = kFlagZil | kFlagGangHeader | kFlagGangChild | kFlagAsyncAlloc;
    if ((flags & ~kMask) != 0) {
        return Status::Invalid;
    }
    if (has_flag(flags, AllocFlag::GangHeader) && has_flag(flags, AllocFlag::GangChild)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

struct GroupHint {
    std::uint64_t metaslab_id{0};
    std::uint64_t weight{0};
    std::uint64_t allocated{0};
    bool loaded{false};
};

[[nodiscard]] inline Status validate_group(const GroupHint& g) noexcept {
    if (g.loaded && g.metaslab_id == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::metaslab
