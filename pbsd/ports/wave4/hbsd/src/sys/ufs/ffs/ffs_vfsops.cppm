export module pbsd.port.wave4.hbsd.src.sys.ufs.ffs.ffs_vfsops;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ffs/ffs_vfsops.c
// void ffs_vfsops_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ffs/ffs_vfsops.c wave=wave4 loc=2466
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_vfsops {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_vfsops
