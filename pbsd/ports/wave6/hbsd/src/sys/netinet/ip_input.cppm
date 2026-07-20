export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_input;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/ip_input.c
// void ip_input_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_input.c wave=wave6 loc=1409
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_input {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_input
