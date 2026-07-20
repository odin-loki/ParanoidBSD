export module pbsd.port.wave6.hbsd.src.sys.net.if_ethersubr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_ethersubr.c
// void if_ethersubr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_ethersubr.c wave=wave6 loc=1530
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_ethersubr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_ethersubr
