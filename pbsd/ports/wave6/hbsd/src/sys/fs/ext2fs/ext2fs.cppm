export module pbsd.port.wave6.hbsd.src.sys.fs.ext2fs.ext2fs;

module;
// Header bridge — replace #include of hbsd/src/sys/fs/ext2fs/ext2fs.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/ext2fs/ext2fs.h wave=wave6 loc=438
export namespace pbsd::port::wave6::hbsd::src::sys::fs::ext2fs::ext2fs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::ext2fs::ext2fs
