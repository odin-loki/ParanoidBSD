export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_gpsvme;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_gpsvme.c
// void refclock_gpsvme_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_gpsvme.c wave=wave9 loc=253
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_gpsvme {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_gpsvme
