export module pbsd.port.wave2.hbsd.src.lib.libc.net.getaddrinfo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/net/getaddrinfo.c
// void getaddrinfo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/net/getaddrinfo.c wave=wave2 loc=3066
export namespace pbsd::port::wave2::hbsd::src::lib::libc::net::getaddrinfo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::net::getaddrinfo
