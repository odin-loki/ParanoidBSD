export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zstd.zstd_sparc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zstd/zstd_sparc.c
// void zstd_sparc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zstd/zstd_sparc.c wave=wave6 loc=11
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::zstd_sparc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::zstd_sparc
