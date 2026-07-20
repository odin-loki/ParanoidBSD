export module pbsd.port.wave5.hbsd.src.sys.dev.sdio.sdio_subr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sdio/sdio_subr.c
// void sdio_subr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sdio/sdio_subr.c wave=wave5 loc=227
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sdio::sdio_subr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sdio::sdio_subr
