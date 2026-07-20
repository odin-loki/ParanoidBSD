export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.zfs.zpl_file_range;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/zpl_file_range.c
// void zpl_file_range_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/zpl_file_range.c wave=wave6 loc=214
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::zpl_file_range {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::zpl_file_range
