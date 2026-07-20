export module pbsd.port.wave6.hbsd.src.sys.net.if_vlan;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_vlan.c
// void if_vlan_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_vlan.c wave=wave6 loc=2600
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_vlan {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_vlan
