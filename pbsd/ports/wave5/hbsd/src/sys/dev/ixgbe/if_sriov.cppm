export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.if_sriov;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/if_sriov.c
// void if_sriov_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/if_sriov.c wave=wave5 loc=912
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_sriov {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_sriov
