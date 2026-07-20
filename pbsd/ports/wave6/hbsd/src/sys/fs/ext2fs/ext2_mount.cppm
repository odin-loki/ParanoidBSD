export module pbsd.port.wave6.hbsd.src.sys.fs.ext2fs.ext2_mount;

module;
// Header bridge — replace #include of hbsd/src/sys/fs/ext2fs/ext2_mount.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/ext2fs/ext2_mount.h wave=wave6 loc=77
export namespace pbsd::port::wave6::hbsd::src::sys::fs::ext2fs::ext2_mount {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::ext2fs::ext2_mount
