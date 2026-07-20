export module pbsd.port.wave4.hbsd.src.sys.kern.subr_intr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_intr.c
// void subr_intr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_intr.c wave=wave4 loc=1958
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_intr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_intr
