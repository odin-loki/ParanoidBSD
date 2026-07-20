export module pbsd.port.wave4.hbsd.src.sys.kern.subr_capability;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_capability.c
// void subr_capability_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_capability.c wave=wave4 loc=387
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_capability {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_capability
