export module pbsd.port.wave6.hbsd.src.sys.net.if_strings;

module;
// Header bridge — replace #include of hbsd/src/sys/net/if_strings.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/if_strings.h wave=wave6 loc=114
export namespace pbsd::port::wave6::hbsd::src::sys::net::if_strings {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::if_strings
