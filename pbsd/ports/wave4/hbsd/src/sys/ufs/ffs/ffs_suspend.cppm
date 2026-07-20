export module pbsd.port.wave4.hbsd.src.sys.ufs.ffs.ffs_suspend;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ffs/ffs_suspend.c
// void ffs_suspend_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ffs/ffs_suspend.c wave=wave4 loc=361
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_suspend {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_suspend
