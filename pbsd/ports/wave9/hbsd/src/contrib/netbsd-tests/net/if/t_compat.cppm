export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.net.if.t_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/net/if/t_compat.c
// void t_compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/net/if/t_compat.c wave=wave9 loc=85
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::net::if::t_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::net::if::t_compat
