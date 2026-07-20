export module pbsd.port.wave4.hbsd.src.sys.kern.subr_epoch;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_epoch.c
// void subr_epoch_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_epoch.c wave=wave4 loc=1018
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_epoch {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_epoch
