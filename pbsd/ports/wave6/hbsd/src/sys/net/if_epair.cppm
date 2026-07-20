export module pbsd.port.wave6.hbsd.src.sys.net.if_epair;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_epair.c
// void if_epair_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_epair.c wave=wave6 loc=1018
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_epair {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_epair
