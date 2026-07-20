export module pbsd.port.wave4.hbsd.src.sys.kern.kern_cpu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_cpu.c
// void kern_cpu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_cpu.c wave=wave4 loc=1155
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_cpu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_cpu
