export module pbsd.port.wave6.hbsd.src.sys.netinet.sctp_input;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/sctp_input.c
// void sctp_input_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/sctp_input.c wave=wave6 loc=5858
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::sctp_input {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::sctp_input
