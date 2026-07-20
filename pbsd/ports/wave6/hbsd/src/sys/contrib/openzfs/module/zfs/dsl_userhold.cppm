export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dsl_userhold;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dsl_userhold.c
// void dsl_userhold_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dsl_userhold.c wave=wave6 loc=692
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dsl_userhold {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dsl_userhold
