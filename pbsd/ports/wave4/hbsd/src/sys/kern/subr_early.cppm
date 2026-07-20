export module pbsd.port.wave4.hbsd.src.sys.kern.subr_early;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_early.c
// void subr_early_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_early.c wave=wave4 loc=69
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_early {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_early
