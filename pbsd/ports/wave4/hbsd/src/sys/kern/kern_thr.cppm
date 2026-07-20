export module pbsd.port.wave4.hbsd.src.sys.kern.kern_thr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_thr.c
// void kern_thr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_thr.c wave=wave4 loc=660
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_thr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_thr
