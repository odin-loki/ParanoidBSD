export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_hopfpci;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_hopfpci.c
// void refclock_hopfpci_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_hopfpci.c wave=wave9 loc=258
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_hopfpci {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_hopfpci
