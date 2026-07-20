export module pbsd.port.wave2.hbsd.src.lib.libc.net.linkaddr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/net/linkaddr.c
// void linkaddr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/net/linkaddr.c wave=wave2 loc=291
export namespace pbsd::port::wave2::hbsd::src::lib::libc::net::linkaddr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::net::linkaddr
