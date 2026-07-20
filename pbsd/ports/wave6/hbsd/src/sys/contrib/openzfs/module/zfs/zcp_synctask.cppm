export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zcp_synctask;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zcp_synctask.c
// void zcp_synctask_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zcp_synctask.c wave=wave6 loc=629
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zcp_synctask {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zcp_synctask
