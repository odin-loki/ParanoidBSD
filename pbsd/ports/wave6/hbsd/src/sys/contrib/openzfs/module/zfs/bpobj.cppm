export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.bpobj;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/bpobj.c
// void bpobj_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/bpobj.c wave=wave6 loc=1033
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::bpobj {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::bpobj
