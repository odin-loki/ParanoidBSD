export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zstd.lib.compress.zstd_compress;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zstd/lib/compress/zstd_compress.c
// void zstd_compress_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zstd/lib/compress/zstd_compress.c wave=wave6 loc=4279
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::lib::compress::zstd_compress {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::lib::compress::zstd_compress
