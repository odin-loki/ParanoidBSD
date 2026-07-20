export module pbsd.port.wave6.hbsd.src.sys.netinet.tcpip;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet/tcpip.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcpip.h wave=wave6 loc=58
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcpip {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcpip
