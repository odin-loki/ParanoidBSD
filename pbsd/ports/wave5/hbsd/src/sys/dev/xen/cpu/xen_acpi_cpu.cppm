export module pbsd.port.wave5.hbsd.src.sys.dev.xen.cpu.xen_acpi_cpu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/xen/cpu/xen_acpi_cpu.c
// void xen_acpi_cpu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/xen/cpu/xen_acpi_cpu.c wave=wave5 loc=631
export namespace pbsd::port::wave5::hbsd::src::sys::dev::xen::cpu::xen_acpi_cpu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::xen::cpu::xen_acpi_cpu
