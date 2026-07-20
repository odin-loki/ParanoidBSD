export module pbsd.port.wave7.hbsd.src.sys.x86.x86.delay;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/x86/delay.c
// void delay_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/x86/delay.c wave=wave7 loc=139
export namespace pbsd::port::wave7::hbsd::src::sys::x86::x86::delay {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::x86::delay
