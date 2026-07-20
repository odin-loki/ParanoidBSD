module;
#include <cstdint>

export module pbsd.fs.ufs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ufs/inode.h — UFS file flags (subset).
export namespace pbsd::fs::ufs {

enum class InodeFlag : unsigned int {
    Nsnapshot = 0x00000001,
    Immutable = 0x00000002,
    Append    = 0x00000004,
    Nounlink  = 0x00000008,
    Archive   = 0x00000010,
    Opaque    = 0x00000020,
    Nodump    = 0x00000040,
};

enum class FileType : unsigned char {
    Vfifo  = 1,
    Vchr   = 2,
    Vdir   = 4,
    Vblk   = 6,
    Vreg   = 8,
    Vlnk   = 10,
    Vsock  = 12,
};

[[nodiscard]] inline bool is_dir(FileType t) noexcept {
    return t == FileType::Vdir;
}

[[nodiscard]] inline Status validate_inode_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(InodeFlag::Immutable))
        && (flags & static_cast<unsigned>(InodeFlag::Append))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::ufs
