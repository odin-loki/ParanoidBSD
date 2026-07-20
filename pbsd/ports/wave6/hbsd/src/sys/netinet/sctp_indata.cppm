export module pbsd.port.wave6.hbsd.src.sys.netinet.sctp_indata;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/sctp_indata.c
// void sctp_indata_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/sctp_indata.c wave=wave6 loc=5731
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::sctp_indata {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::sctp_indata
