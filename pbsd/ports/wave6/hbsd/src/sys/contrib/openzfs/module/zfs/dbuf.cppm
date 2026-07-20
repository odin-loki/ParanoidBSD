export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.dbuf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/dbuf.c
// void dbuf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/dbuf.c wave=wave6 loc=5562
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dbuf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::dbuf
