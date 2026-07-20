export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.os.freebsd.mnttab;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/os/freebsd/mnttab.c
// void mnttab_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/os/freebsd/mnttab.c wave=wave6 loc=235
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::os::freebsd::mnttab {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::os::freebsd::mnttab
