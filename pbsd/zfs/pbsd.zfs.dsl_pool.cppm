module;
#include <cstdint>

export module pbsd.zfs.dsl_pool;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs — dsl_pool thin scaffold.
export namespace pbsd::zfs::dsl_pool {

struct Pool {
    std::uint64_t txg{};
    std::uint64_t guid{};
    bool open{false};
};

[[nodiscard]] inline Status open_pool(Pool& p, std::uint64_t guid) noexcept {
    if (guid == 0) {
        return Status::Invalid;
    }
    p.guid = guid;
    p.txg = 1;
    p.open = true;
    return Status::Ok;
}

[[nodiscard]] inline Status sync_txg(Pool& p) noexcept {
    if (!p.open) {
        return Status::Invalid;
    }
    ++p.txg;
    return Status::Ok;
}

[[nodiscard]] inline Status close_pool(Pool& p) noexcept {
    p = Pool{};
    return Status::Ok;
}

} // namespace pbsd::zfs::dsl_pool
