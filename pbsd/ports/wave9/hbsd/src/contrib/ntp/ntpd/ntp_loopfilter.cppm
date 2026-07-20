export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.ntp_loopfilter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/ntp_loopfilter.c
// void ntp_loopfilter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/ntp_loopfilter.c wave=wave9 loc=1418
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_loopfilter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_loopfilter
