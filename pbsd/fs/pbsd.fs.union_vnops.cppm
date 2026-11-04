module;
#include <cstdint>

export module pbsd.fs.union_vnops;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/unionfs/union_vnops.c — union vnode op flags.
export namespace pbsd::fs::union_vnops {

enum class OpFlag : unsigned {
    UpperOnly  = 0x0001,
    CopyUp     = 0x0002,
    Whiteout   = 0x0004,
    LookupOnly = 0x0008,
};

[[nodiscard]] inline Status validate_op_flags(unsigned flags) noexcept {
    constexpr unsigned kAll = static_cast<unsigned>(OpFlag::UpperOnly)
                            | static_cast<unsigned>(OpFlag::CopyUp)
                            | static_cast<unsigned>(OpFlag::Whiteout)
                            | static_cast<unsigned>(OpFlag::LookupOnly);
    if ((flags & ~kAll) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool needs_copyup(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(OpFlag::CopyUp)) != 0;
}

} // namespace pbsd::fs::union_vnops
