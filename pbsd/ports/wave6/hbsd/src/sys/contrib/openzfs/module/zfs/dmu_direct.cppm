export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dmu_direct;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dmu_direct.c
// void dmu_direct_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dmu_direct.c wave=wave6 loc=405
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dmu_direct {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dmu_direct
