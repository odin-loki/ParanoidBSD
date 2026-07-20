export module pbsd.port.wave5.hbsd.src.sys.dev.mmc.mmcspi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mmc/mmcspi.c
// void mmcspi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mmc/mmcspi.c wave=wave5 loc=2378
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mmc::mmcspi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mmc::mmcspi
