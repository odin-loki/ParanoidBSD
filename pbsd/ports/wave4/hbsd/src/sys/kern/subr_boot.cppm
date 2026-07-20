export module pbsd.port.wave4.hbsd.src.sys.kern.subr_boot;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_boot.c
// void subr_boot_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_boot.c wave=wave4 loc=232
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_boot {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_boot
