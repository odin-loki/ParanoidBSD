module;
#include <cstdint>

export module pbsd.fs.autofs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/autofs/autofs.h — autofs mount map types.
export namespace pbsd::fs::autofs {

inline constexpr unsigned kMaxPathLen = 1024;
inline constexpr unsigned kMaxKeyLen = 256;

enum class MapType : unsigned char {
    Indirect = 0,
    Direct   = 1,
    Master   = 2,
};

enum class Request : unsigned char {
    Mount   = 0,
    Unmount = 1,
};

[[nodiscard]] inline Status validate_map_type(MapType t) noexcept {
    switch (t) {
    case MapType::Indirect:
    case MapType::Direct:
    case MapType::Master:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::autofs
