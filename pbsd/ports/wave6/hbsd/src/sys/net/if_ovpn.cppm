export module pbsd.port.wave6.hbsd.src.sys.net.if_ovpn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_ovpn.c
// void if_ovpn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_ovpn.c wave=wave6 loc=2920
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_ovpn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_ovpn
