module;
#include <cstdint>

export module pbsd.geom.vfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_vfs.h — GEOM VFS mount hooks.
export namespace pbsd::geom::vfs {

enum class Hook : unsigned char {
    Mount = 0,
    Unmount = 1,
    Sync = 2,
};

[[nodiscard]] inline Status validate_hook(Hook h) noexcept {
    switch (h) {
    case Hook::Mount:
    case Hook::Unmount:
    case Hook::Sync:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::vfs
