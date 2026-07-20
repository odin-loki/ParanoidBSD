export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_resource;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_resource.c
// void acpi_resource_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_resource.c wave=wave5 loc=930
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_resource {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_resource
