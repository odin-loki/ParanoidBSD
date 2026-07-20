export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zfs_chksum;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zfs_chksum.c
// void zfs_chksum_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zfs_chksum.c wave=wave6 loc=393
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_chksum {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_chksum
