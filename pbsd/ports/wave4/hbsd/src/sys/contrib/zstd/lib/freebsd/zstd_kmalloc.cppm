export module pbsd.port.wave4.hbsd.src.sys.contrib.zstd.lib.freebsd.zstd_kmalloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/zstd/lib/freebsd/zstd_kmalloc.c
// void zstd_kmalloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/zstd/lib/freebsd/zstd_kmalloc.c wave=wave4 loc=39
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::zstd::lib::freebsd::zstd_kmalloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::zstd::lib::freebsd::zstd_kmalloc
