export module pbsd.port.wave6.hbsd.src.sys.net.if_llc;

module;
// Header bridge — replace #include of hbsd/src/sys/net/if_llc.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_llc.h wave=wave6 loc=160
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_llc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_llc
