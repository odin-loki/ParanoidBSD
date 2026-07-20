export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dnode_sync;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dnode_sync.c
// void dnode_sync_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dnode_sync.c wave=wave6 loc=899
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dnode_sync {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dnode_sync
