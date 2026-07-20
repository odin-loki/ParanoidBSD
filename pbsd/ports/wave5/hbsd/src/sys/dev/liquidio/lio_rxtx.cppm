export module pbsd.port.wave5.hbsd.src.sys.dev.liquidio.lio_rxtx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/liquidio/lio_rxtx.c
// void lio_rxtx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/liquidio/lio_rxtx.c wave=wave5 loc=324
export namespace pbsd::port::wave5::hbsd::src::sys::dev::liquidio::lio_rxtx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::liquidio::lio_rxtx
