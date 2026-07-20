export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zstd.zstd_in;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zstd/zstd-in.c
// void zstd-in_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zstd/zstd-in.c wave=wave6 loc=69
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::zstd_in {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zstd::zstd_in
