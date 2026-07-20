export module pbsd.port.wave9.hbsd.src.contrib.libevent.evthread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libevent/evthread.c
// void evthread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libevent/evthread.c wave=wave9 loc=509
export namespace pbsd::port::wave9::hbsd::src::contrib::libevent::evthread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libevent::evthread
