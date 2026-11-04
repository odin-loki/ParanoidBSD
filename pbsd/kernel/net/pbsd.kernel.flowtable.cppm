module;
#include <cstdint>

export module pbsd.kernel.flowtable;

import pbsd.core;

/// Freestanding port of `net/flowtable.c` — flow hash table sizing helpers.
export namespace pbsd::kernel::flowtable {

inline constexpr unsigned kMinBuckets = 256u;
inline constexpr unsigned kMaxBuckets = 65536u;
inline constexpr unsigned kDefaultBuckets = 4096u;

struct TableParams {
    unsigned hash_size{kDefaultBuckets};
    unsigned entries{};
    unsigned collisions{};
};

[[nodiscard]] inline Status validate_hash_size(unsigned hash_size) noexcept {
    if (hash_size < kMinBuckets || hash_size > kMaxBuckets) {
        return Status::Invalid;
    }
    if ((hash_size & (hash_size - 1)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init(TableParams& tp, unsigned hash_size) noexcept {
    if (validate_hash_size(hash_size) != Status::Ok) {
        return Status::Invalid;
    }
    tp.hash_size = hash_size;
    tp.entries = 0;
    tp.collisions = 0;
    return Status::Ok;
}

[[nodiscard]] inline unsigned hash_mask(unsigned hash_size) noexcept {
    return hash_size - 1;
}

[[nodiscard]] inline unsigned bucket_for(unsigned hash, unsigned hash_size) noexcept {
    return hash & hash_mask(hash_size);
}

[[nodiscard]] inline Status record_collision(TableParams& tp) noexcept {
    ++tp.collisions;
    if (tp.collisions > tp.hash_size * 8) {
        return Status::Busy;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::flowtable
