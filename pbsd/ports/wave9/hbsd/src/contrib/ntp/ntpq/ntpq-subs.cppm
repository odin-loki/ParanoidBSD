export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpq.ntpq_subs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpq/ntpq-subs.c
// void ntpq-subs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpq/ntpq-subs.c wave=wave9 loc=4172
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpq::ntpq_subs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpq::ntpq_subs
