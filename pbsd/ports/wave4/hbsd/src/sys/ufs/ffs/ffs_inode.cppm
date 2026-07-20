export module pbsd.port.wave4.hbsd.src.sys.ufs.ffs.ffs_inode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ffs/ffs_inode.c
// void ffs_inode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ffs/ffs_inode.c wave=wave4 loc=822
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_inode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_inode
