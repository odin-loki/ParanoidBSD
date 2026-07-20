export module pbsd.port.wave9.hbsd.src.tests.sys.kern.waitpid_nohang;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kern/waitpid_nohang.c
// void waitpid_nohang_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kern/waitpid_nohang.c wave=wave9 loc=68
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::waitpid_nohang {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::waitpid_nohang
