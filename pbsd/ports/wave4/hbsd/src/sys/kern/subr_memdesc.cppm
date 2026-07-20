export module pbsd.port.wave4.hbsd.src.sys.kern.subr_memdesc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_memdesc.c
// void subr_memdesc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_memdesc.c wave=wave4 loc=799
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_memdesc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_memdesc
