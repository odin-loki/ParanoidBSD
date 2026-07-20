export module pbsd.port.wave6.hbsd.src.sys.net.if_vxlan;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_vxlan.c
// void if_vxlan_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_vxlan.c wave=wave6 loc=3696
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_vxlan {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_vxlan
