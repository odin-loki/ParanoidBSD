export module pbsd.port.wave9.hbsd.src.contrib.ntp.sntp.libevent.poll;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/sntp/libevent/poll.c
// void poll_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/sntp/libevent/poll.c wave=wave9 loc=358
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::libevent::poll {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::libevent::poll
