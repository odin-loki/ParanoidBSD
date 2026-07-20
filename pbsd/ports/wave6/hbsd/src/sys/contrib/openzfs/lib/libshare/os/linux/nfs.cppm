export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libshare.os.linux.nfs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libshare/os/linux/nfs.c
// void nfs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libshare/os/linux/nfs.c wave=wave6 loc=587
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libshare::os::linux::nfs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libshare::os::linux::nfs
