export module pbsd.port.wave9.hbsd.src.tests.sys.netinet.udp_dontroute;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/netinet/udp_dontroute.c
// void udp_dontroute_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/netinet/udp_dontroute.c wave=wave9 loc=136
export namespace pbsd::port::wave9::hbsd::src::tests::sys::netinet::udp_dontroute {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::netinet::udp_dontroute
