export module pbsd.port.wave4.hbsd.src.sys.kern.subr_eventhandler;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_eventhandler.c
// void subr_eventhandler_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_eventhandler.c wave=wave4 loc=328
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_eventhandler {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_eventhandler
