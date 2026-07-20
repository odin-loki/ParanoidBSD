module;
#include <cstdint>

export module pbsd.fs.ufs_dir;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ufs/dir.h — UFS directory entry layout.
export namespace pbsd::fs::ufs_dir {

inline constexpr unsigned kDirectEntries = 12;
inline constexpr unsigned kNameMax = 255;
inline constexpr unsigned kRecLenMin = 8;

struct Entry {
    unsigned ino{};
    unsigned short reclen{};
    unsigned char type{};
    unsigned char namelen{};
};

[[nodiscard]] inline Status validate_entry(const Entry& e) noexcept {
    if (e.ino == 0 || e.reclen < kRecLenMin) {
        return Status::Invalid;
    }
    if (e.namelen == 0 || e.namelen > kNameMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::ufs_dir
