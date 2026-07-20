export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.os.freebsd.getexecname;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/os/freebsd/getexecname.c
// void getexecname_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/os/freebsd/getexecname.c wave=wave6 loc=41
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::os::freebsd::getexecname {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::os::freebsd::getexecname
