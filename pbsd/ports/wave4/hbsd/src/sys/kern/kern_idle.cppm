export module pbsd.port.wave4.hbsd.src.sys.kern.kern_idle;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_idle.c
// void kern_idle_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_idle.c wave=wave4 loc=88
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_idle {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_idle
