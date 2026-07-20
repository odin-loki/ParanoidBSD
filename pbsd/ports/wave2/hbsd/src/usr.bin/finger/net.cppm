export module pbsd.port.wave2.hbsd.src.usr_bin.finger.net;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/finger/net.c
// void net_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/finger/net.c wave=wave2 loc=230
export namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::net {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::net
