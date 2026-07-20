export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.freebsd.spl.spl_uio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/freebsd/spl/spl_uio.c
// void spl_uio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/freebsd/spl/spl_uio.c wave=wave6 loc=319
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::spl::spl_uio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::spl::spl_uio
