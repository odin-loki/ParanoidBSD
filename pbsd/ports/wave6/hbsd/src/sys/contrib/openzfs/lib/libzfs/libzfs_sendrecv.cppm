export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libzfs.libzfs_sendrecv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libzfs/libzfs_sendrecv.c
// void libzfs_sendrecv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libzfs/libzfs_sendrecv.c wave=wave6 loc=5648
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzfs::libzfs_sendrecv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzfs::libzfs_sendrecv
