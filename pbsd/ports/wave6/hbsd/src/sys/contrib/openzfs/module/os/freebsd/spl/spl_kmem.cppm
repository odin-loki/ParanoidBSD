export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.freebsd.spl.spl_kmem;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/freebsd/spl/spl_kmem.c
// void spl_kmem_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/freebsd/spl/spl_kmem.c wave=wave6 loc=342
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::spl::spl_kmem {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::freebsd::spl::spl_kmem
