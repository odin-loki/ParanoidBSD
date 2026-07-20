export module pbsd.port.wave9.hbsd.src.contrib.ntp.libntp.adjtime;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/libntp/adjtime.c
// void adjtime_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/libntp/adjtime.c wave=wave9 loc=386
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::libntp::adjtime {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::libntp::adjtime
