export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.ixgbe_vf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/ixgbe_vf.c
// void ixgbe_vf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/ixgbe_vf.c wave=wave5 loc=817
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::ixgbe_vf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::ixgbe_vf
