export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.include.t_errno;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/include/t_errno.c
// void t_errno_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/include/t_errno.c wave=wave9 loc=765
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::include::t_errno {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::include::t_errno
