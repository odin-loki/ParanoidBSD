export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libzfs.os.freebsd.libzfs_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libzfs/os/freebsd/libzfs_compat.c
// void libzfs_compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libzfs/os/freebsd/libzfs_compat.c wave=wave6 loc=379
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzfs::os::freebsd::libzfs_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzfs::os::freebsd::libzfs_compat
