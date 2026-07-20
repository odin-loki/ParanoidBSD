export module pbsd.port.wave4.hbsd.src.sys.kern.subr_pcpu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_pcpu.c
// void subr_pcpu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_pcpu.c wave=wave4 loc=425
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_pcpu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_pcpu
