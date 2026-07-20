export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dmu_recv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dmu_recv.c
// void dmu_recv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dmu_recv.c wave=wave6 loc=3904
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dmu_recv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dmu_recv
