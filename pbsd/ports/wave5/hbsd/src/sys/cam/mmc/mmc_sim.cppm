export module pbsd.port.wave5.hbsd.src.sys.cam.mmc.mmc_sim;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/cam/mmc/mmc_sim.c
// void mmc_sim_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/cam/mmc/mmc_sim.c wave=wave5 loc=250
export namespace pbsd::port::wave5::hbsd::src::sys::cam::mmc::mmc_sim {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::cam::mmc::mmc_sim
