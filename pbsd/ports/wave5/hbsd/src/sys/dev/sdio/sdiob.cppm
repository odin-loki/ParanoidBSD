export module pbsd.port.wave5.hbsd.src.sys.dev.sdio.sdiob;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sdio/sdiob.c
// void sdiob_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sdio/sdiob.c wave=wave5 loc=1168
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sdio::sdiob {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sdio::sdiob
