export module pbsd.port.wave6.hbsd.src.sys.net.flowtable;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/flowtable.c
// void flowtable_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/flowtable.c wave=wave6 loc=820
export namespace pbsd::port::wave6::hbsd::src::sys::net::flowtable {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::flowtable
