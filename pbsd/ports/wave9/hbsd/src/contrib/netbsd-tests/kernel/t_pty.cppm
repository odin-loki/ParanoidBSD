export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.kernel.t_pty;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/kernel/t_pty.c
// void t_pty_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/kernel/t_pty.c wave=wave9 loc=351
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::kernel::t_pty {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::kernel::t_pty
