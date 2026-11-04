module;
#include <cstdint>

export module pbsd.stand.fat;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/dosfs.c
export namespace pbsd::stand::fat {

inline constexpr unsigned kSectorSize = 512;
inline constexpr unsigned kMaxCluster = 0x0FFFFFF8u;

enum class FatType : unsigned char {
    Fat12 = 12,
    Fat16 = 16,
    Fat32 = 32,
};

[[nodiscard]] inline Status validate_cluster(unsigned cluster, FatType type) noexcept {
    if (cluster < 2) {
        return Status::Invalid;
    }
    if (type == FatType::Fat32 && cluster >= kMaxCluster) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::fat
