export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.zfs.zfs_uio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/zfs_uio.c
// void zfs_uio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/zfs_uio.c wave=wave6 loc=682
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::zfs_uio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::zfs_uio
