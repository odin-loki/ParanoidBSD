export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zstd.lib.compress.zstd_lazy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zstd/lib/compress/zstd_lazy.c
// void zstd_lazy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zstd/lib/compress/zstd_lazy.c wave=wave6 loc=1143
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::lib::compress::zstd_lazy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::lib::compress::zstd_lazy
