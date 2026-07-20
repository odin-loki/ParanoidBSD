export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zap.c
// void zap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zap.c wave=wave6 loc=1717
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zap
