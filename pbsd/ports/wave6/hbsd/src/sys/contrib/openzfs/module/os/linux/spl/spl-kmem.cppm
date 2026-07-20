export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.spl.spl_kmem;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-kmem.c
// void spl-kmem_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-kmem.c wave=wave6 loc=630
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_kmem {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_kmem
