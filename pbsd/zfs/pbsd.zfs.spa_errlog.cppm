module;
#include <cstdint>

export module pbsd.zfs.spa_errlog;

import pbsd.core;

/// PROVENANCE: openzfs spa_errlog — pool error log scaffold.
export namespace pbsd::zfs::spa_errlog {

struct Entry {
    std::uint64_t bookmark{};
    std::uint64_t birth{};
    unsigned err{};
};

struct Log {
    unsigned count{};
    unsigned capacity{64};
};

[[nodiscard]] inline Status append(Log& log, const Entry& e) noexcept {
    if (e.err == 0) {
        return Status::Invalid;
    }
    if (log.count >= log.capacity) {
        return Status::Busy;
    }
    ++log.count;
    (void)e;
    return Status::Ok;
}

[[nodiscard]] inline Status clear(Log& log) noexcept {
    log.count = 0;
    return Status::Ok;
}

} // namespace pbsd::zfs::spa_errlog
