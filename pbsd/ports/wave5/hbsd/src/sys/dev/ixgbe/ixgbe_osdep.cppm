export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.ixgbe_osdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/ixgbe_osdep.c
// void ixgbe_osdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/ixgbe_osdep.c wave=wave5 loc=178
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::ixgbe_osdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::ixgbe_osdep
