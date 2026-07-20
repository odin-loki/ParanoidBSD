export module pbsd.port.wave2.hbsd.src.usr_bin.systat.tcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/systat/tcp.c
// void tcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/systat/tcp.c wave=wave2 loc=318
export namespace pbsd::port::wave2::hbsd::src::usr_bin::systat::tcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::systat::tcp
