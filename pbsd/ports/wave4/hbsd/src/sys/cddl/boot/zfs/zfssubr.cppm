export module pbsd.port.wave4.hbsd.src.sys.cddl.boot.zfs.zfssubr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/cddl/boot/zfs/zfssubr.c
// void zfssubr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/cddl/boot/zfs/zfssubr.c wave=wave4 loc=1829
export namespace pbsd::port::wave4::hbsd::src::sys::cddl::boot::zfs::zfssubr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::cddl::boot::zfs::zfssubr
