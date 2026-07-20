export module pbsd.port.wave4.hbsd.src.sys.kern.kern_kcov;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_kcov.c
// void kern_kcov_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_kcov.c wave=wave4 loc=577
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_kcov {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_kcov
