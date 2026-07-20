export module pbsd.port.wave4.hbsd.src.sys.contrib.zstd.lib.compress.zstd_double_fast;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/zstd/lib/compress/zstd_double_fast.c
// void zstd_double_fast_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/zstd/lib/compress/zstd_double_fast.c wave=wave4 loc=778
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::zstd::lib::compress::zstd_double_fast {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::zstd::lib::compress::zstd_double_fast
