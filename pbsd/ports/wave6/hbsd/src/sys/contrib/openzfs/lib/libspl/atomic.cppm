export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.atomic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/atomic.c
// void atomic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/atomic.c wave=wave6 loc=373
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::atomic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::atomic
