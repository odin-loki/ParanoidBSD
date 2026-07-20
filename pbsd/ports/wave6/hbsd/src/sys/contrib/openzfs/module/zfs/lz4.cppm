export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.lz4;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/lz4.c
// void lz4_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/lz4.c wave=wave6 loc=988
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::lz4 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::lz4
