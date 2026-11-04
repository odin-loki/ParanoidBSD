module;
#include <cstdint>

export module pbsd.zfs.dsl_dir;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dsl_dir.h — dataset directory.
export namespace pbsd::zfs::dsl_dir {

inline constexpr unsigned kNameMax = 256;

struct Phys {
    std::uint64_t used_bytes{};
    std::uint64_t compressed_bytes{};
    std::uint64_t quota{};
    std::uint64_t reservation{};
};

[[nodiscard]] inline Status validate_phys(const Phys& p) noexcept {
    if (p.quota != 0 && p.used_bytes > p.quota) {
        return Status::Invalid;
    }
    if (p.reservation != 0 && p.used_bytes < p.reservation) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t logical_used(const Phys& p) noexcept {
    return p.used_bytes;
}

} // namespace pbsd::zfs::dsl_dir
