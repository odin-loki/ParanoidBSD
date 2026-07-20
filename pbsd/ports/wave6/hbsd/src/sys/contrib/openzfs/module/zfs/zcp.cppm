export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zcp.c
// void zcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zcp.c wave=wave6 loc=1454
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zcp
