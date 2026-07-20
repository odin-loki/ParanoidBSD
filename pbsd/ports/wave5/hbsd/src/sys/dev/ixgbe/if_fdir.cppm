export module pbsd.port.wave5.hbsd.src.sys.dev.ixgbe.if_fdir;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ixgbe/if_fdir.c
// void if_fdir_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ixgbe/if_fdir.c wave=wave5 loc=160
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_fdir {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ixgbe::if_fdir
