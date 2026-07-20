export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.zfs.zpl_export;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/zpl_export.c
// void zpl_export_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/zpl_export.c wave=wave6 loc=258
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::zpl_export {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::zpl_export
