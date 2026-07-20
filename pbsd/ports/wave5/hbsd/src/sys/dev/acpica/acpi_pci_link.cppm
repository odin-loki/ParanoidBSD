export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_pci_link;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_pci_link.c
// void acpi_pci_link_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_pci_link.c wave=wave5 loc=1131
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pci_link {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pci_link
