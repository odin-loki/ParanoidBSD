export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libzfs.libzfs_iter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libzfs/libzfs_iter.c
// void libzfs_iter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libzfs/libzfs_iter.c wave=wave6 loc=655
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzfs::libzfs_iter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzfs::libzfs_iter
