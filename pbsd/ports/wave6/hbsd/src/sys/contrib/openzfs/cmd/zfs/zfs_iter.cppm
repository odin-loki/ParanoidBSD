export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.cmd.zfs.zfs_iter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/cmd/zfs/zfs_iter.c
// void zfs_iter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/cmd/zfs/zfs_iter.c wave=wave6 loc=555
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::cmd::zfs::zfs_iter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::cmd::zfs::zfs_iter
