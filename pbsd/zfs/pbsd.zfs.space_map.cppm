module;
#include <cstdint>

export module pbsd.zfs.space_map;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/space_map.h — space map sizing.
export namespace pbsd::zfs::space_map {

inline constexpr unsigned kHistogramSize = 32;
inline constexpr unsigned kSizeV0Fields = 3;

struct Phys {
    std::uint64_t length{};
    std::int64_t alloc{};
};

[[nodiscard]] inline Status validate_phys(const Phys& p) noexcept {
    if (p.length == 0) {
        return Status::Invalid;
    }
    if (p.alloc < 0 || static_cast<std::uint64_t>(p.alloc) > p.length) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t free_bytes(const Phys& p) noexcept {
    if (validate_phys(p) != Status::Ok) {
        return 0;
    }
    return p.length - static_cast<std::uint64_t>(p.alloc);
}

} // namespace pbsd::zfs::space_map
