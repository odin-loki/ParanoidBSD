export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_seq;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet/tcp_seq.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_seq.h wave=wave6 loc=100
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_seq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_seq
