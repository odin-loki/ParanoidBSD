export module pbsd.port.wave6.hbsd.src.sys.netinet.netdump.netdump;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet/netdump/netdump.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/netdump/netdump.h wave=wave6 loc=60
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::netdump::netdump {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::netdump::netdump
