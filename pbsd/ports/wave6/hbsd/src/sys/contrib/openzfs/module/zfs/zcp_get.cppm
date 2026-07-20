export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zcp_get;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zcp_get.c
// void zcp_get_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zcp_get.c wave=wave6 loc=838
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zcp_get {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zcp_get
