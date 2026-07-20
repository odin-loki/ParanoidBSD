export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_true;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_true.c
// void refclock_true_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_true.c wave=wave9 loc=981
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_true {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_true
