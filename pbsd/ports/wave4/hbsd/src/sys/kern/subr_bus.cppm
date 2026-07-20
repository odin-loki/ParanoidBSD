export module pbsd.port.wave4.hbsd.src.sys.kern.subr_bus;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_bus.c
// void subr_bus_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_bus.c wave=wave4 loc=6372
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_bus {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_bus
