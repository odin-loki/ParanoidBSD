export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_pcivar;

module;
// Header bridge — replace #include of hbsd/src/sys/dev/acpica/acpi_pcivar.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_pcivar.h wave=wave5 loc=35
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pcivar {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pcivar
