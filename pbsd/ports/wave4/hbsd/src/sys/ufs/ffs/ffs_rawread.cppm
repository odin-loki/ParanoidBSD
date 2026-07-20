export module pbsd.port.wave4.hbsd.src.sys.ufs.ffs.ffs_rawread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ffs/ffs_rawread.c
// void ffs_rawread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ffs/ffs_rawread.c wave=wave4 loc=452
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_rawread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ffs::ffs_rawread
