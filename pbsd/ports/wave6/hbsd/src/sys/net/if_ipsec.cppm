export module pbsd.port.wave6.hbsd.src.sys.net.if_ipsec;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_ipsec.c
// void if_ipsec_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_ipsec.c wave=wave6 loc=1095
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_ipsec {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_ipsec
