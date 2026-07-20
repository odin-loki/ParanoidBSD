export module pbsd.port.wave2.hbsd.src.lib.libc.net.getprotoname;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/net/getprotoname.c
// void getprotoname_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/net/getprotoname.c wave=wave2 loc=147
export namespace pbsd::port::wave2::hbsd::src::lib::libc::net::getprotoname {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::net::getprotoname
