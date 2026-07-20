export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.spa_history;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/spa_history.c
// void spa_history_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/spa_history.c wave=wave6 loc=644
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::spa_history {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::spa_history
