export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.lib.libpthread.t_once;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/lib/libpthread/t_once.c
// void t_once_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/lib/libpthread/t_once.c wave=wave9 loc=199
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libpthread::t_once {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libpthread::t_once
