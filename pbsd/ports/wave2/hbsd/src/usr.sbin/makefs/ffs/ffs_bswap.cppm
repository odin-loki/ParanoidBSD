export module pbsd.port.wave2.hbsd.src.usr_sbin.makefs.ffs.ffs_bswap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/makefs/ffs/ffs_bswap.c
// void ffs_bswap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/makefs/ffs/ffs_bswap.c wave=wave2 loc=258
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::makefs::ffs::ffs_bswap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::makefs::ffs::ffs_bswap
