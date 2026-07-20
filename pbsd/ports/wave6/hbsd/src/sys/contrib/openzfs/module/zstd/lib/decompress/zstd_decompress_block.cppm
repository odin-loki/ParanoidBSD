export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zstd.lib.decompress.zstd_decompress_block;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zstd/lib/decompress/zstd_decompress_block.c
// void zstd_decompress_block_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zstd/lib/decompress/zstd_decompress_block.c wave=wave6 loc=1439
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::lib::decompress::zstd_decompress_block {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::lib::decompress::zstd_decompress_block
