export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/acpi.c
// void acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/acpi.c wave=wave2 loc=904
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::acpi
