export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zap_leaf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zap_leaf.c
// void zap_leaf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zap_leaf.c wave=wave6 loc=858
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zap_leaf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zap_leaf
