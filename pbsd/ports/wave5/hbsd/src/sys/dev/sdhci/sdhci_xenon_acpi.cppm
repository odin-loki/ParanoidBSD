export module pbsd.port.wave5.hbsd.src.sys.dev.sdhci.sdhci_xenon_acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sdhci/sdhci_xenon_acpi.c
// void sdhci_xenon_acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sdhci/sdhci_xenon_acpi.c wave=wave5 loc=129
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sdhci::sdhci_xenon_acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sdhci::sdhci_xenon_acpi
