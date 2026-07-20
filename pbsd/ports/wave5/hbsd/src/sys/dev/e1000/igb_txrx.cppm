export module pbsd.port.wave5.hbsd.src.sys.dev.e1000.igb_txrx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/e1000/igb_txrx.c
// void igb_txrx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/e1000/igb_txrx.c wave=wave5 loc=580
export namespace pbsd::port::wave5::hbsd::src::sys::dev::e1000::igb_txrx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::e1000::igb_txrx
