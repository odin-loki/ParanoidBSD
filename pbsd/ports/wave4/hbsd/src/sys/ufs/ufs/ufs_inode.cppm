export module pbsd.port.wave4.hbsd.src.sys.ufs.ufs.ufs_inode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ufs/ufs_inode.c
// void ufs_inode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ufs/ufs_inode.c wave=wave4 loc=256
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ufs::ufs_inode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ufs::ufs_inode
