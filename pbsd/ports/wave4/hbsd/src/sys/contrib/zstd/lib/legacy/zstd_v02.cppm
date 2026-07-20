export module pbsd.port.wave4.hbsd.src.sys.contrib.zstd.lib.legacy.zstd_v02;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/zstd/lib/legacy/zstd_v02.c
// void zstd_v02_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/zstd/lib/legacy/zstd_v02.c wave=wave4 loc=3465
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::zstd::lib::legacy::zstd_v02 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::zstd::lib::legacy::zstd_v02
