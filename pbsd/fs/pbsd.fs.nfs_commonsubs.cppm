module;
#include <cstdint>

export module pbsd.fs.nfs_commonsubs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/nfs/nfs_commonsubs.c — NFS common helpers.
export namespace pbsd::fs::nfs_commonsubs {

inline constexpr unsigned kMaxHandle = 64;

struct Fh {
    unsigned len{};
    unsigned char data[kMaxHandle]{};
};

[[nodiscard]] inline Status validate_fh(const Fh& fh) noexcept {
    if (fh.len == 0 || fh.len > kMaxHandle) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool fh_equal(const Fh& a, const Fh& b) noexcept {
    if (a.len != b.len) {
        return false;
    }
    for (unsigned i = 0; i < a.len; ++i) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::fs::nfs_commonsubs
