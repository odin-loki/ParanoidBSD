export module pbsd.port.wave9.hbsd.src.tests.sys.kern.socket_splice;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kern/socket_splice.c
// void socket_splice_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kern/socket_splice.c wave=wave9 loc=979
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::socket_splice {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::socket_splice
