export module pbsd.port.wave7.hbsd.src.sys.x86.x86.busdma_machdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/x86/busdma_machdep.c
// void busdma_machdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/x86/busdma_machdep.c wave=wave7 loc=193
export namespace pbsd::port::wave7::hbsd::src::sys::x86::x86::busdma_machdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::x86::busdma_machdep
