export module pbsd.port.wave7.hbsd.src.sys.x86.isa.nmi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/isa/nmi.c
// void nmi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/isa/nmi.c wave=wave7 loc=94
export namespace pbsd::port::wave7::hbsd::src::sys::x86::isa::nmi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::isa::nmi
