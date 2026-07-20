export module pbsd.port.wave9.hbsd.src.tests.sys.kqueue.libkqueue.signal;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kqueue/libkqueue/signal.c
// void signal_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kqueue/libkqueue/signal.c wave=wave9 loc=196
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::signal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::signal
