export module pbsd.port.wave4.hbsd.src.sys.kern.kern_ntptime;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_ntptime.c
// void kern_ntptime_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_ntptime.c wave=wave4 loc=1049
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_ntptime {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_ntptime
