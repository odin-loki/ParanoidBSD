export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.aggsum;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/aggsum.c
// void aggsum_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/aggsum.c wave=wave6 loc=246
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::aggsum {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::aggsum
