export module pbsd.port.wave6.hbsd.src.sys.net.if_gre;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/if_gre.c
// void if_gre_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_gre.c wave=wave6 loc=832
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_gre {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_gre
