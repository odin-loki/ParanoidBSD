export module pbsd.port.wave9.hbsd.src.contrib.ntp.sntp.libevent.evthread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/sntp/libevent/evthread.c
// void evthread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/sntp/libevent/evthread.c wave=wave9 loc=509
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::libevent::evthread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::libevent::evthread
