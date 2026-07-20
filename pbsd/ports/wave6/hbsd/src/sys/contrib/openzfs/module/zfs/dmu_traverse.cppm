export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dmu_traverse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dmu_traverse.c
// void dmu_traverse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dmu_traverse.c wave=wave6 loc=826
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dmu_traverse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dmu_traverse
