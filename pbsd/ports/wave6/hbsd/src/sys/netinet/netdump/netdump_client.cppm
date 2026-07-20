export module pbsd.port.wave6.hbsd.src.sys.netinet.netdump.netdump_client;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/netdump/netdump_client.c
// void netdump_client_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/netdump/netdump_client.c wave=wave6 loc=757
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::netdump::netdump_client {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::netdump::netdump_client
