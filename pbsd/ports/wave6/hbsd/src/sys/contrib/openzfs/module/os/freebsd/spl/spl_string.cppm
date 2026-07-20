export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.freebsd.spl.spl_string;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/freebsd/spl/spl_string.c
// void spl_string_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/freebsd/spl/spl_string.c wave=wave6 loc=138
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::spl::spl_string {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::spl::spl_string
