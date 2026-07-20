export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libzfs.libzfs_diff;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libzfs/libzfs_diff.c
// void libzfs_diff_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libzfs/libzfs_diff.c wave=wave6 loc=830
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzfs::libzfs_diff {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzfs::libzfs_diff
