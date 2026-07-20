export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.freebsd.zfs.zfs_ioctl_os;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/freebsd/zfs/zfs_ioctl_os.c
// void zfs_ioctl_os_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/freebsd/zfs/zfs_ioctl_os.c wave=wave6 loc=172
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::zfs::zfs_ioctl_os {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::zfs::zfs_ioctl_os
