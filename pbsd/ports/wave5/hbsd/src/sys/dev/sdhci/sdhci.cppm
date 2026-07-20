export module pbsd.port.wave5.hbsd.src.sys.dev.sdhci.sdhci;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sdhci/sdhci.c
// void sdhci_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sdhci/sdhci.c wave=wave5 loc=2902
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sdhci::sdhci {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sdhci::sdhci
