export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.lib.libpthread.t_sleep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/lib/libpthread/t_sleep.c
// void t_sleep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/lib/libpthread/t_sleep.c wave=wave9 loc=104
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libpthread::t_sleep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libpthread::t_sleep
