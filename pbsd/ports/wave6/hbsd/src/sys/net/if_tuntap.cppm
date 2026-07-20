export module pbsd.port.wave6.hbsd.src.sys.net.if_tuntap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_tuntap.c
// void if_tuntap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_tuntap.c wave=wave6 loc=2192
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_tuntap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_tuntap
