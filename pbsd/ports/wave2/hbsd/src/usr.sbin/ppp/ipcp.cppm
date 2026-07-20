export module pbsd.port.wave2.hbsd.src.usr_sbin.ppp.ipcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ppp/ipcp.c
// void ipcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ppp/ipcp.c wave=wave2 loc=1482
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::ipcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::ipcp
