export module pbsd.port.wave4.hbsd.src.sys.kern.subr_turnstile;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_turnstile.c
// void subr_turnstile_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_turnstile.c wave=wave4 loc=1328
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_turnstile {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_turnstile
