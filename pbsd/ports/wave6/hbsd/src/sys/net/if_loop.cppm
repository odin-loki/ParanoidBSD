export module pbsd.port.wave6.hbsd.src.sys.net.if_loop;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_loop.c
// void if_loop_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_loop.c wave=wave6 loc=446
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_loop {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_loop
