module;
#include <cstdint>

export module pbsd.zfs.txg;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/txg.h — transaction groups.
export namespace pbsd::zfs::txg {

inline constexpr unsigned kConcurrentStates = 3;
inline constexpr unsigned kSize = 4;
inline constexpr unsigned kMask = kSize - 1;
inline constexpr unsigned kInitial = kSize;
inline constexpr unsigned kDeferSize = 2;

[[nodiscard]] inline unsigned idx(std::uint64_t txg) noexcept {
    return static_cast<unsigned>(txg & kMask);
}

[[nodiscard]] inline Status validate_txg(std::uint64_t txg) noexcept {
    if (txg == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::txg
