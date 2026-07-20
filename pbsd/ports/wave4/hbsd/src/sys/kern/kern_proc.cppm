export module pbsd.port.wave4.hbsd.src.sys.kern.kern_proc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_proc.c
// void kern_proc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_proc.c wave=wave4 loc=3586
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_proc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_proc
