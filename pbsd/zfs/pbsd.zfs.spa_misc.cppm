module;
#include <cstdint>

export module pbsd.zfs.spa_misc;

import pbsd.core;

/// PROVENANCE: openzfs spa_misc — pool misc helpers scaffold.
export namespace pbsd::zfs::spa_misc {

struct SpaLite {
    std::uint64_t guid{};
    std::uint64_t load_txg{};
    bool imported{false};
};

[[nodiscard]] inline Status import_spa(SpaLite& s, std::uint64_t guid) noexcept {
    if (guid == 0) {
        return Status::Invalid;
    }
    s.guid = guid;
    s.load_txg = 1;
    s.imported = true;
    return Status::Ok;
}

[[nodiscard]] inline Status export_spa(SpaLite& s) noexcept {
    if (!s.imported) {
        return Status::Invalid;
    }
    s = SpaLite{};
    return Status::Ok;
}

} // namespace pbsd::zfs::spa_misc
