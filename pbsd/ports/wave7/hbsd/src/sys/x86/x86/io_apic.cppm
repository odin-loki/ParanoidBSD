export module pbsd.port.wave7.hbsd.src.sys.x86.x86.io_apic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/x86/io_apic.c
// void io_apic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/x86/io_apic.c wave=wave7 loc=1269
export namespace pbsd::port::wave7::hbsd::src::sys::x86::x86::io_apic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::x86::io_apic
