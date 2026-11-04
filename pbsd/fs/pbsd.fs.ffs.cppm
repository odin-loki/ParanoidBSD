module;
#include <cstddef>
#include <cstdint>

export module pbsd.fs.ffs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ffs/fs.h — FFS superblock search and magic.
export namespace pbsd::fs::ffs {

inline constexpr int kSblockFloppy = 0;
inline constexpr int kSblockUfs1 = 8192;
inline constexpr int kSblockUfs2 = 65536;
inline constexpr int kSblockPiggy = 262144;
inline constexpr int kSblockSize = 8192;
inline constexpr int kStdSb = -1;

inline constexpr unsigned kMagicUfs1 = 0x011954;
inline constexpr unsigned kMagicUfs2 = 0x19540119;

enum class SbFlag : unsigned {
    NoHashFail = 0x0001,
    NoWarnFail = 0x0003,
    NoMsg      = 0x0004,
    NoCsum     = 0x0008,
    FsReadOnly = 0x0010,
};

[[nodiscard]] inline Status validate_magic(unsigned magic) noexcept {
    if (magic != kMagicUfs1 && magic != kMagicUfs2) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_ufs2(unsigned magic) noexcept {
    return magic == kMagicUfs2;
}

[[nodiscard]] inline int next_search_offset(int index) noexcept {
    static constexpr int kSearch[] = {kSblockUfs2, kSblockUfs1, kSblockFloppy, kSblockPiggy, -1};
    if (index < 0) {
        return kSearch[0];
    }
    for (unsigned i = 0; i < sizeof(kSearch) / sizeof(kSearch[0]); ++i) {
        if (kSearch[i] == index && i + 1 < sizeof(kSearch) / sizeof(kSearch[0])) {
            return kSearch[i + 1];
        }
    }
    return -1;
}

[[nodiscard]] inline Status validate_sb_flags(unsigned flags) noexcept {
    if ((flags & ~static_cast<unsigned>(SbFlag::FsReadOnly)) != 0
        && (flags & static_cast<unsigned>(SbFlag::NoWarnFail)) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::ffs
