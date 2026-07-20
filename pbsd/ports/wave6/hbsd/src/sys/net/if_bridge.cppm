export module pbsd.port.wave6.hbsd.src.sys.net.if_bridge;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_bridge.c
// void if_bridge_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_bridge.c wave=wave6 loc=4476
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_bridge {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_bridge
