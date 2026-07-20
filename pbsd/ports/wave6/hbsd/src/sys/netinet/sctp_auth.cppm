export module pbsd.port.wave6.hbsd.src.sys.netinet.sctp_auth;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/sctp_auth.c
// void sctp_auth_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/sctp_auth.c wave=wave6 loc=1986
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::sctp_auth {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::sctp_auth
