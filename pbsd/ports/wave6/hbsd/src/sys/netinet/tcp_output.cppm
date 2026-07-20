export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_output;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_output.c
// void tcp_output_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_output.c wave=wave6 loc=2179
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_output {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_output
