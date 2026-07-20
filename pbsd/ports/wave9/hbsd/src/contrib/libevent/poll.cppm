export module pbsd.port.wave9.hbsd.src.contrib.libevent.poll;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libevent/poll.c
// void poll_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libevent/poll.c wave=wave9 loc=358
export namespace pbsd::port::wave9::hbsd::src::contrib::libevent::poll {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libevent::poll
