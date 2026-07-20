export module pbsd.port.wave4.hbsd.src.sys.ufs.ufs.ufs_vnops;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ufs/ufs_vnops.c
// void ufs_vnops_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ufs/ufs_vnops.c wave=wave4 loc=3087
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ufs::ufs_vnops {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ufs::ufs_vnops
