export module pbsd.port.wave5.hbsd.src.sys.dev.ofw.ofw_cpu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ofw/ofw_cpu.c
// void ofw_cpu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ofw/ofw_cpu.c wave=wave5 loc=455
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ofw::ofw_cpu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ofw::ofw_cpu
