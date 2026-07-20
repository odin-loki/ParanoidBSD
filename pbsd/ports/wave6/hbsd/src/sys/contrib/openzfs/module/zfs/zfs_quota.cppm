export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zfs_quota;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zfs_quota.c
// void zfs_quota_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zfs_quota.c wave=wave6 loc=524
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_quota {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_quota
