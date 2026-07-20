export module pbsd.port.wave6.hbsd.src.sys.fs.ext2fs.ext2_inode_cnv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/ext2fs/ext2_inode_cnv.c
// void ext2_inode_cnv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/ext2fs/ext2_inode_cnv.c wave=wave6 loc=336
export namespace pbsd::port::wave6::hbsd::src::sys::fs::ext2fs::ext2_inode_cnv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::ext2fs::ext2_inode_cnv
