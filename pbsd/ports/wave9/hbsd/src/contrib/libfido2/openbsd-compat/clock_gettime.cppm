export module pbsd.port.wave9.hbsd.src.contrib.libfido2.openbsd_compat.clock_gettime;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/openbsd-compat/clock_gettime.c
// void clock_gettime_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/openbsd-compat/clock_gettime.c wave=wave9 loc=33
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::clock_gettime {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::clock_gettime
