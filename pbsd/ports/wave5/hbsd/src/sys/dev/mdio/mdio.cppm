export module pbsd.port.wave5.hbsd.src.sys.dev.mdio.mdio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mdio/mdio.c
// void mdio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mdio/mdio.c wave=wave5 loc=126
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mdio::mdio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mdio::mdio
