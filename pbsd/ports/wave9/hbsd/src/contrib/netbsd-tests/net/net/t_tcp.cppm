export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.net.net.t_tcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/net/net/t_tcp.c
// void t_tcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/net/net/t_tcp.c wave=wave9 loc=227
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::net::net::t_tcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::net::net::t_tcp
