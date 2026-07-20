export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.os.linux.getexecname;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/os/linux/getexecname.c
// void getexecname_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/os/linux/getexecname.c wave=wave6 loc=33
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::os::linux::getexecname {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::os::linux::getexecname
