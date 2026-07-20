export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.spl.spl_zlib;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-zlib.c
// void spl-zlib_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-zlib.c wave=wave6 loc=217
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_zlib {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_zlib
