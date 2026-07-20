export module pbsd.port.wave2.hbsd.src.usr_bin.netstat.ipsec;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/netstat/ipsec.c
// void ipsec_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/netstat/ipsec.c wave=wave2 loc=433
export namespace pbsd::port::wave2::hbsd::src::usr_bin::netstat::ipsec {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::netstat::ipsec
