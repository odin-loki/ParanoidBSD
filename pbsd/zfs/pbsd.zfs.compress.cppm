module;
#include <cstdint>

export module pbsd.zfs.compress;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zio.h — ZIO compression algorithms.
export namespace pbsd::zfs::compress {

enum class Algorithm : unsigned char {
    Inherit = 0,
    On,
    Off,
    Lzjb,
    Gzip,
    Zle,
    Lz4,
    Functions,
};

inline constexpr Algorithm kDefault = Algorithm::On;
inline constexpr Algorithm kLegacyOnValue = Algorithm::Lzjb;
inline constexpr Algorithm kLz4OnValue = Algorithm::Lz4;

[[nodiscard]] inline Status validate_algo(Algorithm a) noexcept {
    if (a >= Algorithm::Functions) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::compress
