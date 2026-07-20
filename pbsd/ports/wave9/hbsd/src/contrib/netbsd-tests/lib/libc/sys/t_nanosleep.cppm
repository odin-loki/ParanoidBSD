export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.lib.libc.sys.t_nanosleep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/lib/libc/sys/t_nanosleep.c
// void t_nanosleep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/lib/libc/sys/t_nanosleep.c wave=wave9 loc=268
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libc::sys::t_nanosleep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libc::sys::t_nanosleep
