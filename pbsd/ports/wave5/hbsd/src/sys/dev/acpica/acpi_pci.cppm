export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_pci;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_pci.c
// void acpi_pci_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_pci.c wave=wave5 loc=520
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pci {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pci
