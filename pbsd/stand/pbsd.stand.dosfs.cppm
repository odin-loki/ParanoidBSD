module;
#include <cstddef>

export module pbsd.stand.dosfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/dosfs.c
export namespace pbsd::stand::dosfs {

inline constexpr unsigned kSectorSize = 512;
inline constexpr unsigned kClusterMax = 65528;

[[nodiscard]] inline Status validate_cluster(unsigned cluster) noexcept {
    if (cluster < 2 || cluster > kClusterMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::dosfs
