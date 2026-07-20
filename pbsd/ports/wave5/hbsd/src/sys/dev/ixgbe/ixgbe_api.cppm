export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.ixgbe_api;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/ixgbe_api.c
// void ixgbe_api_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/ixgbe_api.c wave=wave5 loc=1812
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::ixgbe_api {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::ixgbe_api
