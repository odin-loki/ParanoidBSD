export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.if_bypass;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/if_bypass.c
// void if_bypass_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/if_bypass.c wave=wave5 loc=797
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_bypass {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_bypass
