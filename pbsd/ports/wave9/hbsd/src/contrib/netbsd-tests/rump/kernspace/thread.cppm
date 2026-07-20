export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.rump.kernspace.thread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/rump/kernspace/thread.c
// void thread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/rump/kernspace/thread.c wave=wave9 loc=108
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::rump::kernspace::thread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::rump::kernspace::thread
