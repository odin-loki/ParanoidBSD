export module pbsd.port.wave9.hbsd.src.tests.sys.capsicum.capsicum_test;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/capsicum/capsicum-test.cc
// void capsicum-test_init(void);
}

export import pbsd.userland.capsicum;
export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/capsicum/capsicum-test.cc wave=wave9 loc=119
export namespace pbsd::port::wave9::hbsd::src::tests::sys::capsicum::capsicum_test {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::capsicum::capsicum_test
