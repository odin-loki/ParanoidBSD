export module pbsd.port.wave4.hbsd.src.sys.kern.subr_trap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_trap.c
// void subr_trap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_trap.c wave=wave4 loc=396
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_trap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_trap
