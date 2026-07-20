module;
#include <cstdint>

export module pbsd.zfs.fuid;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/uts/common/fs/zfs/zfs_fuid.c — file unique ID helpers.
export namespace pbsd::zfs::fuid {

inline constexpr unsigned kFuidSize = 16;

struct Id {
    unsigned char bytes[kFuidSize]{};
};

[[nodiscard]] inline bool is_zero(const Id& id) noexcept {
    for (unsigned i = 0; i < kFuidSize; ++i) {
        if (id.bytes[i] != 0) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline Status validate(const Id& id) noexcept {
    if (is_zero(id)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool equal(const Id& a, const Id& b) noexcept {
    for (unsigned i = 0; i < kFuidSize; ++i) {
        if (a.bytes[i] != b.bytes[i]) {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::zfs::fuid
