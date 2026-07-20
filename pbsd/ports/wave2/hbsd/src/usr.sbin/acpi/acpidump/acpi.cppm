export module pbsd.port.wave2.hbsd.src.usr_sbin.acpi.acpidump.acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/acpi/acpidump/acpi.c
// void acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/acpi/acpidump/acpi.c wave=wave2 loc=3055
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::acpi::acpidump::acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::acpi::acpidump::acpi
