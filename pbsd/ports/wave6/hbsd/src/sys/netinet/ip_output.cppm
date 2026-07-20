export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_output;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/ip_output.c
// void ip_output_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_output.c wave=wave6 loc=1591
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_output {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_output
