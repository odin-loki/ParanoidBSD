export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zfs_ioctl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zfs_ioctl.c
// void zfs_ioctl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zfs_ioctl.c wave=wave6 loc=8343
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_ioctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_ioctl
