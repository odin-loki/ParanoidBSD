export module pbsd.port.wave2.hbsd.src.usr_sbin.ppp.tcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ppp/tcp.c
// void tcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ppp/tcp.c wave=wave2 loc=212
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::tcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::tcp
