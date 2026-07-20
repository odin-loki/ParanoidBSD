export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.spl.spl_taskq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-taskq.c
// void spl-taskq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-taskq.c wave=wave6 loc=1860
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_taskq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_taskq
