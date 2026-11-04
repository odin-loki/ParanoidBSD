module;
#include <cstdint>

export module pbsd.fs.union_vfsops;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/unionfs/union_vfsops.c — union mount ops.
export namespace pbsd::fs::union_vfsops {

enum class MountFlag : unsigned {
    ReadOnly = 0x0001,
    Below    = 0x0002,
    Hidden   = 0x0004,
};

struct MountCtx {
    unsigned layers{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_mount(const MountCtx& ctx) noexcept {
    if (ctx.layers == 0 || ctx.layers > 32) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_readonly(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(MountFlag::ReadOnly)) != 0;
}

} // namespace pbsd::fs::union_vfsops
