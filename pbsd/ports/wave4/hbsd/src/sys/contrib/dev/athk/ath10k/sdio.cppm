export module pbsd.port.wave4.hbsd.src.sys.contrib.dev.athk.ath10k.sdio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/dev/athk/ath10k/sdio.c
// void sdio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/dev/athk/ath10k/sdio.c wave=wave4 loc=2684
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::athk::ath10k::sdio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::athk::ath10k::sdio
