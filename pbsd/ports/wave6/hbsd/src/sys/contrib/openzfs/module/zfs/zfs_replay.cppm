export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zfs_replay;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zfs_replay.c
// void zfs_replay_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zfs_replay.c wave=wave6 loc=1265
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_replay {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zfs_replay
