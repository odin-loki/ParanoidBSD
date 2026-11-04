module;
#include <cstdint>

export module pbsd.kernel.rss;

import pbsd.core;

/// Freestanding port of `net/rss_config.c` — RSS bucket/key policy helpers.
export namespace pbsd::kernel::rss {

inline constexpr unsigned kMaxBuckets = 128u;
inline constexpr unsigned kDefaultBuckets = 4u;
inline constexpr unsigned kKeyLen = 40u;

enum class HashType : unsigned {
    Toeplitz = 0,
    Crc      = 1,
};

struct Config {
    unsigned    buckets{kDefaultBuckets};
    HashType    hash_type{HashType::Toeplitz};
    bool        enabled{};
};

[[nodiscard]] inline Status validate_buckets(unsigned buckets) noexcept {
    if (buckets == 0 || buckets > kMaxBuckets || (buckets & (buckets - 1)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init(Config& cfg, unsigned buckets) noexcept {
    if (validate_buckets(buckets) != Status::Ok) {
        return Status::Invalid;
    }
    cfg.buckets = buckets;
    cfg.hash_type = HashType::Toeplitz;
    cfg.enabled = false;
    return Status::Ok;
}

[[nodiscard]] inline unsigned bucket_index(unsigned hash, unsigned buckets) noexcept {
    return hash & (buckets - 1);
}

[[nodiscard]] inline Status enable(Config& cfg) noexcept {
    if (validate_buckets(cfg.buckets) != Status::Ok) {
        return Status::Invalid;
    }
    cfg.enabled = true;
    return Status::Ok;
}

} // namespace pbsd::kernel::rss
