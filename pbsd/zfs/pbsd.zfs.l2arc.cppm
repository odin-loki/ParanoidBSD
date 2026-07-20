module;
#include <cstdint>

export module pbsd.zfs.l2arc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/arc_impl.h — L2ARC header constants.
export namespace pbsd::zfs::l2arc {

inline constexpr unsigned kHeaderSize = 4096;
inline constexpr unsigned kDevBlockSize = 4096;

enum class Flag : unsigned int {
    Enabled = 1u << 0,
    Feed    = 1u << 1,
    Write   = 1u << 2,
};

[[nodiscard]] inline Status validate_dev_size(unsigned long long bytes) noexcept {
    if (bytes < kDevBlockSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool flag_has(Flag f, Flag bit) noexcept {
    return (static_cast<unsigned>(f) & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::zfs::l2arc
