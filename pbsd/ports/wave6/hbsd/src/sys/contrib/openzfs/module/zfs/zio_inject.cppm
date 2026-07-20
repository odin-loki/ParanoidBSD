export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zio_inject;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zio_inject.c
// void zio_inject_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zio_inject.c wave=wave6 loc=1199
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zio_inject {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zio_inject
