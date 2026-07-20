export module pbsd.port.wave7.hbsd.src.sys.arm64.acpica.acpi_iort;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/acpica/acpi_iort.c
// void acpi_iort_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/acpica/acpi_iort.c wave=wave7 loc=692
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::acpica::acpi_iort {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::acpica::acpi_iort
