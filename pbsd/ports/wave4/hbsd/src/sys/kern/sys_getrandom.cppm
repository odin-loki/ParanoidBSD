export module pbsd.port.wave4.hbsd.src.sys.kern.sys_getrandom;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/sys_getrandom.c
// void sys_getrandom_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/sys_getrandom.c wave=wave4 loc=127
export namespace pbsd::port::wave4::hbsd::src::sys::kern::sys_getrandom {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::sys_getrandom
