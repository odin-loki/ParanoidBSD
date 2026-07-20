export module pbsd.port.wave4.hbsd.src.sys.cddl.boot.zfs.gzip;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/cddl/boot/zfs/gzip.c
// void gzip_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/cddl/boot/zfs/gzip.c wave=wave4 loc=96
export namespace pbsd::port::wave4::hbsd::src::sys::cddl::boot::zfs::gzip {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::cddl::boot::zfs::gzip
