export module pbsd.port.wave4.hbsd.src.sys.kern.kern_context;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_context.c
// void kern_context_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_context.c wave=wave4 loc=127
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_context {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_context
