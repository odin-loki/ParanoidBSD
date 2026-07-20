export module pbsd.port.wave2.hbsd.src.usr_bin.netstat.pfkey;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/netstat/pfkey.c
// void pfkey_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/netstat/pfkey.c wave=wave2 loc=200
export namespace pbsd::port::wave2::hbsd::src::usr_bin::netstat::pfkey {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::netstat::pfkey
