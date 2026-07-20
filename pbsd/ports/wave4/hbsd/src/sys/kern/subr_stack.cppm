export module pbsd.port.wave4.hbsd.src.sys.kern.subr_stack;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_stack.c
// void subr_stack_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_stack.c wave=wave4 loc=285
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_stack {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_stack
