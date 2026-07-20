export module pbsd.port.wave4.hbsd.src.sys.contrib.dev.acpica.components.hardware.hwtimer;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/dev/acpica/components/hardware/hwtimer.c
// void hwtimer_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/dev/acpica/components/hardware/hwtimer.c wave=wave4 loc=348
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::components::hardware::hwtimer {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::components::hardware::hwtimer
