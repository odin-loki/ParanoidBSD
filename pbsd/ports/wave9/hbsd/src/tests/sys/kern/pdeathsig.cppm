export module pbsd.port.wave9.hbsd.src.tests.sys.kern.pdeathsig;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kern/pdeathsig.c
// void pdeathsig_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kern/pdeathsig.c wave=wave9 loc=345
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::pdeathsig {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::pdeathsig
