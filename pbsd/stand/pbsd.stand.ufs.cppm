module;
#include <cstdint>

export module pbsd.stand.ufs;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/ufs.c — loader UFS superblock/inode constants.
export namespace pbsd::stand::ufs {

inline constexpr unsigned kMagicUfs1 = 0x00011954u;
inline constexpr unsigned kMagicUfs2 = 0x19540119u;
inline constexpr unsigned kSuperblockOffset = 65536;
inline constexpr unsigned kMaxNameLen = 255;
inline constexpr unsigned kRootIno = 2;

enum class InoType : unsigned char {
    Unknown = 0,
    File = 1,
    Dir = 2,
    Symlink = 3,
};

struct SuperblockHint {
    std::uint32_t magic{0};
    std::uint32_t bsize{0};
    std::uint32_t fsize{0};
    std::uint64_t ncg{0};
};

[[nodiscard]] inline bool is_ufs2(std::uint32_t magic) noexcept {
    return magic == kMagicUfs2;
}

[[nodiscard]] inline bool is_ufs1(std::uint32_t magic) noexcept {
    return magic == kMagicUfs1;
}

[[nodiscard]] inline Status validate_superblock(const SuperblockHint& sb) noexcept {
    if (!is_ufs1(sb.magic) && !is_ufs2(sb.magic)) {
        return Status::Invalid;
    }
    if (sb.bsize == 0 || sb.fsize == 0 || sb.bsize < sb.fsize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_ino(std::uint32_t ino) noexcept {
    return ino >= kRootIno ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::ufs
