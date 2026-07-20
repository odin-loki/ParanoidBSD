export module pbsd.port.wave4.hbsd.src.sys.ufs.ffs.ffs_vnops;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ffs/ffs_vnops.c
// void ffs_vnops_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ffs/ffs_vnops.c wave=wave4 loc=2105
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_vnops {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_vnops
