export module pbsd.port.wave9.hbsd.src.tests.sys.kqueue.libkqueue.user;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kqueue/libkqueue/user.c
// void user_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kqueue/libkqueue/user.c wave=wave9 loc=186
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::user {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::user
