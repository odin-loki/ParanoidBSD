export module pbsd.port.wave7.hbsd.src.sys.arm64.arm64.gic_v3_acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/arm64/gic_v3_acpi.c
// void gic_v3_acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/arm64/gic_v3_acpi.c wave=wave7 loc=480
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::gic_v3_acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::gic_v3_acpi
