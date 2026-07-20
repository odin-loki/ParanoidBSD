export module pbsd.port.wave4.hbsd.src.sys.kern.kern_intr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_intr.c
// void kern_intr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_intr.c wave=wave4 loc=1685
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_intr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_intr
