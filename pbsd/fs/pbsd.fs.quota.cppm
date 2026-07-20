module;
#include <cstdint>

export module pbsd.fs.quota;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ufs/quota.h — dqblk quota types.
export namespace pbsd::fs::quota {

enum class Type : unsigned char {
    User    = 0,
    Group   = 1,
    Default = 2,
};

enum class Flag : unsigned int {
    Inode  = 0x01,
    Block  = 0x02,
    Limits = 0x04,
};

struct Dqblk {
    long long bhardlimit{};
    long long bsoftlimit{};
    long long curblocks{};
    long long ihardlimit{};
    long long isoftlimit{};
    long long curinodes{};
    long long btime{};
    long long itime{};
};

[[nodiscard]] inline Status validate_dqblk(Dqblk const& dq) noexcept {
    if (dq.bsoftlimit > dq.bhardlimit && dq.bhardlimit >= 0) {
        return Status::Invalid;
    }
    if (dq.isoftlimit > dq.ihardlimit && dq.ihardlimit >= 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::quota
