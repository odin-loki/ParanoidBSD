export module pbsd.port.wave2.hbsd.src.usr_sbin.ppp.throughput;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ppp/throughput.c
// void throughput_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ppp/throughput.c wave=wave2 loc=302
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::throughput {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::throughput
