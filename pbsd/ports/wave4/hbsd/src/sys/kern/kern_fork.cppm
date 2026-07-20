export module pbsd.port.wave4.hbsd.src.sys.kern.kern_fork;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_fork.c
// void kern_fork_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_fork.c wave=wave4 loc=1296
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_fork {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_fork
