export module pbsd.port.wave6.hbsd.src.sys.net.if_infiniband;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_infiniband.c
// void if_infiniband_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_infiniband.c wave=wave6 loc=731
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_infiniband {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_infiniband
