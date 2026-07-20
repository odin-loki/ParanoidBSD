export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.if_ixv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/if_ixv.c
// void if_ixv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/if_ixv.c wave=wave5 loc=1985
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_ixv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_ixv
