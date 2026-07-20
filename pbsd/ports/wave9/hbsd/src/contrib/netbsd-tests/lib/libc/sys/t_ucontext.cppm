export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.lib.libc.sys.t_ucontext;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/lib/libc/sys/t_ucontext.c
// void t_ucontext_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/lib/libc/sys/t_ucontext.c wave=wave9 loc=76
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libc::sys::t_ucontext {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libc::sys::t_ucontext
