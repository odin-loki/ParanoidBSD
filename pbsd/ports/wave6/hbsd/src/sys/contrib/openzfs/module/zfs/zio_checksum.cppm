export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.zio_checksum;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/zio_checksum.c
// void zio_checksum_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/zio_checksum.c wave=wave6 loc=609
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zio_checksum {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::zio_checksum
