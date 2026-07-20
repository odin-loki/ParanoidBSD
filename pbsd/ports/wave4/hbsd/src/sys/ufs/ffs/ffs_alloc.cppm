export module pbsd.port.wave4.hbsd.src.sys.ufs.ffs.ffs_alloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ffs/ffs_alloc.c
// void ffs_alloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ffs/ffs_alloc.c wave=wave4 loc=3620
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_alloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_alloc
