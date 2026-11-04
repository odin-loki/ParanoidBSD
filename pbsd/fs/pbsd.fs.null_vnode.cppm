module;
#include <cstdint>

export module pbsd.fs.null_vnode;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/nullfs/null.h — nullfs bypass flags.
export namespace pbsd::fs::null_vnode {

enum class BypassFlag : unsigned int {
    Read  = 0x0001,
    Write = 0x0002,
    Lookup = 0x0004,
};

struct Target {
    unsigned lower_mount_id{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_target(const Target& t) noexcept {
    if (t.lower_mount_id == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool bypass_read(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(BypassFlag::Read)) != 0;
}

} // namespace pbsd::fs::null_vnode
