module;
#include <cstdint>

export module pbsd.pkg.lockfile;

import pbsd.core;
import pbsd.pkg.index;

/// Burst 13 — lockfile slot metadata (frozen index snapshot).
export namespace pbsd::pkg::lockfile {

inline constexpr unsigned kMagic = 0x50425344u; // 'PBSD'
inline constexpr unsigned kVersion = 1;

struct Header {
    unsigned magic{kMagic};
    unsigned version{kVersion};
    unsigned entry_count{0};
    std::uint64_t created_unix{0};
};

struct Lockfile {
    Header hdr{};
    index::PackageIndex index{};
};

[[nodiscard]] inline Status validate(Header const& h) noexcept {
    if (h.magic != kMagic || h.version != kVersion) {
        return Status::Protocol;
    }
    if (h.entry_count > index::kMaxEntries) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status freeze(Lockfile& lf, index::PackageIndex const& idx,
                                   std::uint64_t ts) noexcept {
    lf.hdr.created_unix = ts;
    lf.index = idx;
    lf.hdr.entry_count = idx.count;
    return validate(lf.hdr);
}

} // namespace pbsd::pkg::lockfile
