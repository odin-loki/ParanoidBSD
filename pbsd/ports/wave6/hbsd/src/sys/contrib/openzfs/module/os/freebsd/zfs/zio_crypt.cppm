export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.freebsd.zfs.zio_crypt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/freebsd/zfs/zio_crypt.c
// void zio_crypt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/freebsd/zfs/zio_crypt.c wave=wave6 loc=1813
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::zfs::zio_crypt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::zfs::zio_crypt
