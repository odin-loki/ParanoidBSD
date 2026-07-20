export module pbsd.port.wave9.hbsd.src.contrib.libevent.epoll_sub;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libevent/epoll_sub.c
// void epoll_sub_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libevent/epoll_sub.c wave=wave9 loc=66
export namespace pbsd::port::wave9::hbsd::src::contrib::libevent::epoll_sub {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libevent::epoll_sub
