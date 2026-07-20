export module pbsd.port.wave6.hbsd.src.sys.netinet6.udp6_usrreq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/udp6_usrreq.c
// void udp6_usrreq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/udp6_usrreq.c wave=wave6 loc=1311
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::udp6_usrreq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::udp6_usrreq
