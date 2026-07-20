export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.zfs.zfs_dir;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/zfs_dir.c
// void zfs_dir_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/zfs_dir.c wave=wave6 loc=1292
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::zfs_dir {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::zfs_dir
