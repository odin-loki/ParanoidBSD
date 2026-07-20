export module pbsd.port.wave9.hbsd.src.tests.sys.kqueue.libkqueue.proc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kqueue/libkqueue/proc.c
// void proc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kqueue/libkqueue/proc.c wave=wave9 loc=423
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::proc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::proc
