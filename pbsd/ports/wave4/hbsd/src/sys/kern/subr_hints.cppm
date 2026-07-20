export module pbsd.port.wave4.hbsd.src.sys.kern.subr_hints;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_hints.c
// void subr_hints_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_hints.c wave=wave4 loc=518
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_hints {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_hints
