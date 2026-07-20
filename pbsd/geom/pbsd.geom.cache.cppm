module;
#include <cstdint>

export module pbsd.geom.cache;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/cache/g_cache.h — GEOM CACHE metadata.
export namespace pbsd::geom::cache {

inline constexpr unsigned kVersion = 1;
inline constexpr unsigned kBuckets = 1u << 3;

enum class Type : unsigned char {
    Manual = 0,
    Automatic = 1,
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Manual:
    case Type::Automatic:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline constexpr unsigned bucket(unsigned bno) noexcept {
    return bno & (kBuckets - 1);
}

} // namespace pbsd::geom::cache
