export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpq.libntpq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpq/libntpq.c
// void libntpq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpq/libntpq.c wave=wave9 loc=772
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpq::libntpq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpq::libntpq
