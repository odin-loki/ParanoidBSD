export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.ddt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/ddt.c
// void ddt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/ddt.c wave=wave6 loc=2995
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::ddt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::ddt
