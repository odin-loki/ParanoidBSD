export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_tpro;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_tpro.c
// void refclock_tpro_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_tpro.c wave=wave9 loc=208
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_tpro {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_tpro
