export module pbsd.port.wave4.hbsd.src.sys.kern.kern_switch;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_switch.c
// void kern_switch_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_switch.c wave=wave4 loc=671
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_switch {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_switch
