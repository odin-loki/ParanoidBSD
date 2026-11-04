module;
#include <cstdint>

export module pbsd.stand.bcache;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/bcache.c — hashed block cache sizing.
export namespace pbsd::stand::bcache {

inline constexpr unsigned kDefaultBlks = 64;
inline constexpr unsigned kMaxBlks = 4096;
inline constexpr unsigned kMaxDevices = 31;
inline constexpr unsigned kDefaultReadahead = 4;

struct Config {
    unsigned nblks{kDefaultBlks};
    unsigned blksize{512};
    unsigned readahead{kDefaultReadahead};
};

[[nodiscard]] inline Status validate_config(const Config& cfg) noexcept {
    if (cfg.nblks == 0 || cfg.nblks > kMaxBlks) {
        return Status::Invalid;
    }
    if (cfg.blksize < 512 || cfg.blksize > 4096) {
        return Status::Invalid;
    }
    if (cfg.readahead == 0 || cfg.readahead > cfg.nblks) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::bcache
