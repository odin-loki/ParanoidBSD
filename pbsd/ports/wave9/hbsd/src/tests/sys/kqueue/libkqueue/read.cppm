export module pbsd.port.wave9.hbsd.src.tests.sys.kqueue.libkqueue.read;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kqueue/libkqueue/read.c
// void read_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kqueue/libkqueue/read.c wave=wave9 loc=323
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::read {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::read
