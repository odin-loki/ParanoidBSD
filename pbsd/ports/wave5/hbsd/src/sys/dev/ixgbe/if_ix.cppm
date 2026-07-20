export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.if_ix;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/if_ix.c
// void if_ix_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/if_ix.c wave=wave5 loc=5944
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_ix {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_ix
