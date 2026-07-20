export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.spl.spl_generic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-generic.c
// void spl-generic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-generic.c wave=wave6 loc=622
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_generic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_generic
