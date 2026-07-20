export module pbsd.port.wave9.hbsd.src.contrib.libevent.devpoll;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libevent/devpoll.c
// void devpoll_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libevent/devpoll.c wave=wave9 loc=311
export namespace pbsd::port::wave9::hbsd::src::contrib::libevent::devpoll {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libevent::devpoll
