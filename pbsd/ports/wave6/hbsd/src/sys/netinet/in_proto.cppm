export module pbsd.port.wave6.hbsd.src.sys.netinet.in_proto;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/in_proto.c
// void in_proto_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/in_proto.c wave=wave6 loc=137
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::in_proto {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::in_proto
