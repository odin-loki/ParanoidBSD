export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.lib.libc.ttyio.t_ptm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/lib/libc/ttyio/t_ptm.c
// void t_ptm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/lib/libc/ttyio/t_ptm.c wave=wave9 loc=174
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libc::ttyio::t_ptm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::lib::libc::ttyio::t_ptm
