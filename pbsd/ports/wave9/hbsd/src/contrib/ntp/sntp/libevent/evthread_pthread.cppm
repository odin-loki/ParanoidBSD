export module pbsd.port.wave9.hbsd.src.contrib.ntp.sntp.libevent.evthread_pthread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/sntp/libevent/evthread_pthread.c
// void evthread_pthread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/sntp/libevent/evthread_pthread.c wave=wave9 loc=191
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::libevent::evthread_pthread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::libevent::evthread_pthread
