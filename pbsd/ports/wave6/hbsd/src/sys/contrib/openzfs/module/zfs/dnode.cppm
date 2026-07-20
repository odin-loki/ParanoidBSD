export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dnode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dnode.c
// void dnode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dnode.c wave=wave6 loc=2778
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dnode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dnode
