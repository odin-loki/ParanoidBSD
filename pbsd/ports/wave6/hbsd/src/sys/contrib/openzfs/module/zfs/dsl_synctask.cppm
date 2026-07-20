export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dsl_synctask;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dsl_synctask.c
// void dsl_synctask_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dsl_synctask.c wave=wave6 loc=265
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dsl_synctask {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dsl_synctask
