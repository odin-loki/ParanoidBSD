export module pbsd.port.wave4.hbsd.src.sys.kern.stack_protector;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/stack_protector.c
// void stack_protector_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/stack_protector.c wave=wave4 loc=28
export namespace pbsd::port::wave4::hbsd::src::sys::kern::stack_protector {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::stack_protector
