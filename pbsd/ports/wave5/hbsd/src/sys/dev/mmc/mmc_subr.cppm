export module pbsd.port.wave5.hbsd.src.sys.dev.mmc.mmc_subr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mmc/mmc_subr.c
// void mmc_subr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mmc/mmc_subr.c wave=wave5 loc=275
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mmc::mmc_subr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mmc::mmc_subr
