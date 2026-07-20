export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dsl_scan;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dsl_scan.c
// void dsl_scan_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dsl_scan.c wave=wave6 loc=5394
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dsl_scan {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dsl_scan
