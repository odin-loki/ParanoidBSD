export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.spl.spl_kmem_cache;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-kmem-cache.c
// void spl-kmem-cache_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/spl/spl-kmem-cache.c wave=wave6 loc=1444
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_kmem_cache {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::spl::spl_kmem_cache
