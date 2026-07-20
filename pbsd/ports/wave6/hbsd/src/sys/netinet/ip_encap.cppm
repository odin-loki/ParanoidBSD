export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_encap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/ip_encap.c
// void ip_encap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_encap.c wave=wave6 loc=413
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_encap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_encap
