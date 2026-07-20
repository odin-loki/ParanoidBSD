export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_hopfser;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_hopfser.c
// void refclock_hopfser_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_hopfser.c wave=wave9 loc=366
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_hopfser {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_hopfser
