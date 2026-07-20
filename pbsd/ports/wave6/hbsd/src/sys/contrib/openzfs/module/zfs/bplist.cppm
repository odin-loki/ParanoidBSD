export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.bplist;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/bplist.c
// void bplist_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/bplist.c wave=wave6 loc=88
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::bplist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::bplist
