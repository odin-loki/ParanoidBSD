export module pbsd.port.wave7.hbsd.src.sys.x86.acpica.madt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/acpica/madt.c
// void madt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/acpica/madt.c wave=wave7 loc=778
export namespace pbsd::port::wave7::hbsd::src::sys::x86::acpica::madt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::acpica::madt
