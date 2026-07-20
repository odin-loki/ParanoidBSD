export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.ixgbe_mbx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/ixgbe_mbx.c
// void ixgbe_mbx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/ixgbe_mbx.c wave=wave5 loc=1210
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::ixgbe_mbx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::ixgbe_mbx
