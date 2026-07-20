export module pbsd.port.wave5.hbsd.src.sys.dev.mmc.host.dwmmc_starfive;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mmc/host/dwmmc_starfive.c
// void dwmmc_starfive_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mmc/host/dwmmc_starfive.c wave=wave5 loc=114
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mmc::host::dwmmc_starfive {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mmc::host::dwmmc_starfive
