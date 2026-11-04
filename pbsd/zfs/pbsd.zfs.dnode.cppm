module;
#include <cstdint>

export module pbsd.zfs.dnode;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dnode.h — dnode types.
export namespace pbsd::zfs::dnode {

enum class Type : unsigned char {
    None     = 0,
    Meta     = 1,
    Directory = 2,
    File     = 3,
    Zap      = 4,
    Dnode    = 5,
};

enum class Flag : unsigned int {
    DirtyLink   = 0x0001,
    DirtyData   = 0x0002,
    DirtyMeta   = 0x0004,
    Evicted     = 0x0008,
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    if (static_cast<unsigned char>(t) > static_cast<unsigned char>(Type::Dnode)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dnode
