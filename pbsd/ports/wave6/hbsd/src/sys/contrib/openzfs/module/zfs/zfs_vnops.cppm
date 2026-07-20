export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zfs_vnops;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zfs_vnops.c
// void zfs_vnops_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zfs_vnops.c wave=wave6 loc=2093
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_vnops {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_vnops
