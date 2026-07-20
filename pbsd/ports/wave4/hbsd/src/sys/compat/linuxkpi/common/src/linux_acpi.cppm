export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_acpi.c
// void linux_acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_acpi.c wave=wave4 loc=382
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_acpi
